#include "Runtime/StoneActionSubsystem.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"

#include "Engine/World.h"
#include "TimerManager.h"

#include "Runtime/StoneRunSubsystem.h"
#include "Data/StoneActionDefinitionData.h"
#include "Core/StoneGameplayTags.h"

static constexpr float kActionTickInterval = 0.25f;

void UStoneActionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RNG.Initialize(static_cast<int32>(FPlatformTime::Cycles64()));
}

void UStoneActionSubsystem::Deinitialize()
{
	StopCurrentAction(false);
	Super::Deinitialize();
}

UStoneRunSubsystem* UStoneActionSubsystem::GetRun() const
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UStoneRunSubsystem>();
		}
	}
	return nullptr;
}

float UStoneActionSubsystem::ResolveActionSpeedMult() const
{
	// SSOT = Ability System (currently player only, later can be routed to an active unit).
	const UStoneRunSubsystem* Run = GetRun();
	if (!Run || !CurrentDef)
	{
		return 1.f;
	}

	UAbilitySystemComponent* ASC = Run->GetASC();
	if (!ASC)
	{
		return 1.f;
	}

	float Score = 100.f;

	switch (CurrentDef->ActionType)
	{
	case EStoneActionType::Travel:
	case EStoneActionType::Explore:
		Score = ASC->GetNumericAttribute(UStoneAttributeSet::GetTravelSpeedAttribute());
		break;

	case EStoneActionType::Gather:
		Score = ASC->GetNumericAttribute(UStoneAttributeSet::GetGatherEfficiencyAttribute());
		break;

	case EStoneActionType::Custom:
	default:
		Score = 100.f;
		break;
	}

	Score = FMath::Max(0.f, Score);
	// Technical sanity clamp only. Design clamps belong in MMCs / score ranges.
	return FMath::Clamp(Score / 100.f, 0.10f, 10.0f);
}

float UStoneActionSubsystem::GetCurrentActionSpeedMult() const
{
	return ResolveActionSpeedMult();
}

FGameplayTag UStoneActionSubsystem::GetLegRandomEventTag(EStoneActionPhase InPhase) const
{
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	if (!CurrentDef || (InPhase != EStoneActionPhase::Outbound && InPhase != EStoneActionPhase::Return))
	{
		return FGameplayTag();
	}

	switch (CurrentDef->ActionType)
	{
	case EStoneActionType::Explore:
		return (InPhase == EStoneActionPhase::Outbound) ? T.Event_Explore : T.Event_ExploreReturn;

	case EStoneActionType::Travel:
		return (InPhase == EStoneActionPhase::Outbound) ? T.Event_Travel_Outbound : T.Event_Travel_Return;

	case EStoneActionType::Gather:
	case EStoneActionType::Custom:
	default:
		// Custom/Gather can opt into an explicit tag; otherwise travel tags are a safe fallback.
		if (CurrentDef->ActionTag.IsValid())
		{
			return CurrentDef->ActionTag;
		}
		return (InPhase == EStoneActionPhase::Outbound) ? T.Event_Travel_Outbound : T.Event_Travel_Return;
	}
}

bool UStoneActionSubsystem::StartAction(UStoneActionDefinitionData* ActionDef)
{
	if (!ActionDef)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneActionSubsystem] StartAction: ActionDef is null"));
		return false;
	}

	if (bActionRunning)
	{
		// Explicitly stop; do not stack actions.
		StopCurrentAction(false);
	}

	UStoneRunSubsystem* Run = GetRun();
	if (!Run)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneActionSubsystem] StartAction: RunSubsystem missing"));
		return false;
	}

	// Requirements check (optional)
	if (!ActionDef->RequiredTags.IsEmpty())
	{
		const FGameplayTagContainer CurrentTags = Run->GetCurrentStateTags();
		if (!CurrentTags.HasAll(ActionDef->RequiredTags))
		{
			FString Missing;
			for (auto It = ActionDef->RequiredTags.CreateConstIterator(); It; ++It)
			{
				const FGameplayTag& Required = *It;
				if (!CurrentTags.HasTagExact(Required))
				{
					if (!Missing.IsEmpty()) Missing += TEXT(", ");
					Missing += Required.ToString();
				}
			}

			UE_LOG(LogTemp, Warning,
				TEXT("[StoneActionSubsystem] StartAction rejected: requirements not met for %s. MissingTags=[%s]"),
				*GetNameSafe(ActionDef), *Missing);
			return false;
		}
	}

	CurrentDef = ActionDef;
	bActionRunning = true;

	// Apply run-state tags while this action is active.
	ActiveStateTags.Reset();
	ActiveStateTags.AppendTags(ActionDef->GrantedStateTags);

	{
		const FStoneGameplayTags& Tags = FStoneGameplayTags::Get();
		ActiveStateTags.AddTag(Tags.State_OnAction);

		if (ActionDef->ActionType == EStoneActionType::Travel || ActionDef->ActionType == EStoneActionType::Explore)
		{
			ActiveStateTags.AddTag(Tags.State_OnTravel);
		}
	}

	Run->AddStateTags(ActiveStateTags);

	// Cache timing (BASE seconds at speed=1)
	BaseDurationSeconds = FMath::Max(1.f, ActionDef->BaseDurationSeconds);

	// Random pacing configuration (interpreted as BASE seconds)
	RandomMinGapSeconds = FMath::Max(kActionTickInterval, ActionDef->RandomMinGapSeconds);
	RandomMaxGapSeconds = FMath::Max(RandomMinGapSeconds, ActionDef->RandomMaxGapSeconds);
	RandomChance01 = FMath::Clamp(ActionDef->RandomChance01, 0.f, 1.f);
	bAllowImmediateRandom = ActionDef->bAllowImmediateRandom;

	// Phase split
	const float OutShare = FMath::Clamp(ActionDef->OutboundShare01, 0.f, 1.f);
	const float RetShare = FMath::Clamp(ActionDef->ReturnShare01, 0.f, 1.f);
	const float Sum = FMath::Max(KINDA_SMALL_NUMBER, OutShare + RetShare);

	OutboundSeconds = BaseDurationSeconds * (OutShare / Sum);
	ReturnSeconds   = BaseDurationSeconds * (RetShare / Sum);

	PhaseElapsedBaseSeconds = 0.f;
	TotalElapsedBaseSeconds = 0.f;

	// Activate packs only while action runs
	ApplyActionPacks();

	EnterPhase(EStoneActionPhase::Outbound);

	bSkipRandomThisTick = true;
	bReturnHomeQueued = false;

	NextRandomCountdownBaseSeconds =
		bAllowImmediateRandom ? 0.f : RNG.FRandRange(RandomMinGapSeconds, RandomMaxGapSeconds);

	// Start tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ActionTickHandle, this, &UStoneActionSubsystem::TickAction, kActionTickInterval, true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneActionSubsystem] StartAction: World is null; cannot tick."));
		StopCurrentAction(false);
		return false;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[StoneActionSubsystem] StartAction: %s Type=%d Base=%.1fs Out=%.1fs Ret=%.1fs RandomGap=[%.1f..%.1f] Chance=%.2f Packs=%d"),
		*GetNameSafe(ActionDef),
		(int32)ActionDef->ActionType,
		BaseDurationSeconds, OutboundSeconds, ReturnSeconds,
		RandomMinGapSeconds, RandomMaxGapSeconds, RandomChance01,
		ActionDef->PackIdsToActivate.Num());

	OnActionStateChanged.Broadcast();
	OnActionProgressChanged.Broadcast(GetActionProgress01());
	return true;
}

void UStoneActionSubsystem::StopCurrentAction(bool bForceReturnHomeEvent)
{
	if (!bActionRunning)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActionTickHandle);
	}

	UStoneRunSubsystem* Run = GetRun();

	// Optional final event request
	if (bForceReturnHomeEvent && Run)
	{
		if (Run->HasOpenEvent() || Run->GetPendingEventCount() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneActionSubsystem] StopCurrentAction: return-home requested but events are already pending/open; not queuing."));
		}
		else
		{
			const int32 PendingBefore = Run->GetPendingEventCount();
			Run->QueueEventByTag(FStoneGameplayTags::Get().Event_Travel_ReturnHome, true);
			const int32 PendingAfter = Run->GetPendingEventCount();
			UE_LOG(LogTemp, Warning,
				TEXT("[StoneActionSubsystem] StopCurrentAction: return-home queued. Pending %d -> %d Open=%s"),
				PendingBefore, PendingAfter, Run->HasOpenEvent() ? TEXT("true") : TEXT("false"));
		}
	}

	// Remove action-applied state tags
	if (Run && !ActiveStateTags.IsEmpty())
	{
		Run->RemoveStateTags(ActiveStateTags);
	}
	ActiveStateTags.Reset();

	ClearActionPacks();

	UE_LOG(LogTemp, Warning,
		TEXT("[StoneActionSubsystem] StopCurrentAction: Done. Phase=%d ElapsedBase=%.2f/%.2f"),
		(int32)Phase, TotalElapsedBaseSeconds, BaseDurationSeconds);

	bActionRunning = false;
	Phase = EStoneActionPhase::None;

	CurrentDef = nullptr;
	BaseDurationSeconds = 0.f;
	OutboundSeconds = 0.f;
	ReturnSeconds = 0.f;
	PhaseElapsedBaseSeconds = 0.f;
	TotalElapsedBaseSeconds = 0.f;
	NextRandomCountdownBaseSeconds = 0.f;

	OnActionStateChanged.Broadcast();
	OnActionProgressChanged.Broadcast(0.f);
}

void UStoneActionSubsystem::ApplyActionPacks()
{
	ActivatedPackIds.Reset();

	UStoneRunSubsystem* Run = GetRun();
	if (!Run || !CurrentDef)
	{
		return;
	}

	const int32 ActiveBefore = Run->GetActivePackIds().Num();
	int32 Attempts = 0;
	int32 AddedNow = 0;

	for (const FName& PackId : CurrentDef->PackIdsToActivate)
	{
		if (PackId.IsNone())
		{
			continue;
		}
		++Attempts;

		const bool bAlready = Run->GetActivePackIds().Contains(PackId);
		Run->ActivatePackTemporary(PackId);

		if (Run->GetActivePackIds().Contains(PackId))
		{
			ActivatedPackIds.AddUnique(PackId);
			AddedNow += bAlready ? 0 : 1;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[StoneActionSubsystem] ApplyActionPacks: Failed to activate temp pack '%s'."), *PackId.ToString());
		}
	}

	const int32 ActiveAfter = Run->GetActivePackIds().Num();
	UE_LOG(LogTemp, Warning,
		TEXT("[StoneActionSubsystem] ApplyActionPacks: Attempts=%d AddedNow=%d ActivePacks %d -> %d"),
		Attempts, AddedNow, ActiveBefore, ActiveAfter);
}

void UStoneActionSubsystem::ClearActionPacks()
{
	UStoneRunSubsystem* Run = GetRun();
	if (!Run)
	{
		ActivatedPackIds.Reset();
		return;
	}

	Run->DeactivateTemporaryPacksByIds(ActivatedPackIds);
	ActivatedPackIds.Reset();
}

float UStoneActionSubsystem::GetActionProgress01() const
{
	if (!bActionRunning || BaseDurationSeconds <= 0.f)
	{
		return 0.f;
	}
	return FMath::Clamp(TotalElapsedBaseSeconds / BaseDurationSeconds, 0.f, 1.f);
}

float UStoneActionSubsystem::GetLegProgress01() const
{
	if (!bActionRunning)
	{
		return 0.f;
	}

	const float LegDur =
		(Phase == EStoneActionPhase::Outbound) ? OutboundSeconds :
		(Phase == EStoneActionPhase::Return) ? ReturnSeconds : 0.f;

	if (LegDur <= 0.f)
	{
		return 0.f;
	}
	return FMath::Clamp(PhaseElapsedBaseSeconds / LegDur, 0.f, 1.f);
}

void UStoneActionSubsystem::EnterPhase(EStoneActionPhase NewPhase)
{
	Phase = NewPhase;
	PhaseElapsedBaseSeconds = 0.f;
	bSkipRandomThisTick = true;

	UStoneRunSubsystem* Run = GetRun();
	if (Run)
	{
		if (Phase == EStoneActionPhase::Arrival)
		{
			const int32 PendingBefore = Run->GetPendingEventCount();
			Run->QueueEventByTag(FStoneGameplayTags::Get().Event_Travel_Arrival, true);
			const int32 PendingAfter = Run->GetPendingEventCount();
			UE_LOG(LogTemp, Warning,
				TEXT("[StoneActionSubsystem] Phase=Arrival: queued arrival gate event. Pending %d -> %d Open=%s"),
				PendingBefore, PendingAfter, Run->HasOpenEvent() ? TEXT("true") : TEXT("false"));
		}
	}

	OnActionStateChanged.Broadcast();
}

void UStoneActionSubsystem::HandlePhaseAdvance()
{
	if (!bActionRunning)
	{
		return;
	}

	if (Phase == EStoneActionPhase::Outbound)
	{
		EnterPhase(EStoneActionPhase::Arrival);
		return;
	}

	if (Phase == EStoneActionPhase::Arrival)
	{
		EnterPhase(EStoneActionPhase::Return);
		return;
	}

	if (Phase == EStoneActionPhase::Return)
	{
		Phase = EStoneActionPhase::Completed;

		if (!bReturnHomeQueued)
		{
			bReturnHomeQueued = true;

			if (UStoneRunSubsystem* Run = GetRun())
			{
				const int32 PendingBefore = Run->GetPendingEventCount();
				Run->QueueEventByTag(FStoneGameplayTags::Get().Event_Travel_ReturnHome, true);
				const int32 PendingAfter = Run->GetPendingEventCount();

				UE_LOG(LogTemp, Warning,
					TEXT("[StoneActionSubsystem] Phase=Completed: queued return-home gate event. Pending %d -> %d Open=%s"),
					PendingBefore, PendingAfter, Run->HasOpenEvent() ? TEXT("true") : TEXT("false"));
			}
		}

		OnActionStateChanged.Broadcast();
		return;
	}
}

void UStoneActionSubsystem::RollRandomLegEvent()
{
	if (!bActionRunning)
	{
		return;
	}

	if (Phase != EStoneActionPhase::Outbound && Phase != EStoneActionPhase::Return)
	{
		return;
	}

	UStoneRunSubsystem* Run = GetRun();
	if (!Run || Run->HasOpenEvent() || Run->GetPendingEventCount() > 0)
	{
		return;
	}

	const float Roll = RNG.FRand();
	if (Roll <= RandomChance01)
	{
		const FGameplayTag Tag = GetLegRandomEventTag(Phase);
		if (Tag.IsValid())
		{
			const int32 PendingBefore = Run->GetPendingEventCount();
			Run->QueueEventByTag(Tag, true);
			const int32 PendingAfter = Run->GetPendingEventCount();

			UE_LOG(LogTemp, Warning,
				TEXT("[StoneActionSubsystem] RandomEvent: Phase=%d Tag=%s Roll=%.3f<=%.3f Pending %d->%d Open=%s"),
				(int32)Phase,
				*Tag.ToString(),
				Roll, RandomChance01,
				PendingBefore, PendingAfter,
				Run->HasOpenEvent() ? TEXT("true") : TEXT("false"));
		}
	}

	NextRandomCountdownBaseSeconds = RNG.FRandRange(RandomMinGapSeconds, RandomMaxGapSeconds);
}

void UStoneActionSubsystem::TickAction()
{
	if (!bActionRunning || !CurrentDef)
	{
		return;
	}

	UStoneRunSubsystem* Run = GetRun();
	if (!Run)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneActionSubsystem] TickAction: RunSubsystem missing while action is running. Stopping action."));
		StopCurrentAction(false);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneActionSubsystem] TickAction: World is null while action is running. Stopping action."));
		StopCurrentAction(false);
		return;
	}

	// ----- PAUSE GATE -----
	// No time advances while an event is open OR there are pending events waiting to be shown.
	const bool bEventBlocking = Run->HasOpenEvent() || Run->GetPendingEventCount() > 0;
	const float SimSpeed = Run->GetSimulationSpeed();
	if (SimSpeed <= 0.f || bEventBlocking)
	{
		return;
	}

	// If we reached Completed earlier and no longer have events blocking, finalize shutdown.
	if (Phase == EStoneActionPhase::Completed)
	{
		StopCurrentAction(false);
		return;
	}

	// ----- ARRIVAL GATE -----
	// Arrival phase is a pure gate: after the arrival event is resolved, we advance to Return.
	if (Phase == EStoneActionPhase::Arrival)
	{
		HandlePhaseAdvance();
		OnActionProgressChanged.Broadcast(GetActionProgress01());
		return;
	}

	const float SpeedMult = ResolveActionSpeedMult();
	const float AdvanceBaseSeconds = kActionTickInterval * SimSpeed * SpeedMult;
	if (AdvanceBaseSeconds <= 0.f)
	{
		return;
	}

	// ----- PHASE BOUNDARY GUARD -----
	// If this tick would complete the current leg, we advance phase BEFORE any random rolls.
	const bool bWillFinishOutbound = (Phase == EStoneActionPhase::Outbound) && ((PhaseElapsedBaseSeconds + AdvanceBaseSeconds) >= OutboundSeconds);
	const bool bWillFinishReturn   = (Phase == EStoneActionPhase::Return)   && ((PhaseElapsedBaseSeconds + AdvanceBaseSeconds) >= ReturnSeconds);

	if (bWillFinishOutbound || bWillFinishReturn)
	{
		TotalElapsedBaseSeconds += AdvanceBaseSeconds;
		PhaseElapsedBaseSeconds += AdvanceBaseSeconds;

		HandlePhaseAdvance();
		OnActionProgressChanged.Broadcast(GetActionProgress01());
		return;
	}

	// Normal time advance (BASE seconds)
	TotalElapsedBaseSeconds += AdvanceBaseSeconds;
	PhaseElapsedBaseSeconds += AdvanceBaseSeconds;

	// ----- RANDOM LEG EVENT -----
	// Random pacing is BASE seconds so speed does not delete content.
	if (!bSkipRandomThisTick && (Phase == EStoneActionPhase::Outbound || Phase == EStoneActionPhase::Return))
	{
		NextRandomCountdownBaseSeconds -= AdvanceBaseSeconds;

		// If multipliers are high, we might skip multiple gaps in one tick.
		// Safety cap prevents machine-gun rolls.
		int32 SafetyRolls = 0;
		while (NextRandomCountdownBaseSeconds <= 0.f && SafetyRolls < 3)
		{
			++SafetyRolls;

			RollRandomLegEvent();

			// If an event was queued/presented, stop this tick immediately.
			if (Run->HasOpenEvent() || Run->GetPendingEventCount() > 0)
			{
				OnActionProgressChanged.Broadcast(GetActionProgress01());
				return;
			}
		}
	}

	// We only skip random for one tick after a phase transition.
	bSkipRandomThisTick = false;

	OnActionProgressChanged.Broadcast(GetActionProgress01());
}

FText UStoneActionSubsystem::GetActionTitleText() const
{
	if (!CurrentDef)
	{
		return FText::GetEmpty();
	}

	// Prefer DisplayName if set; fallback to asset name (debug-friendly).
	return CurrentDef->DisplayName.IsEmpty()
		? FText::FromString(CurrentDef->GetName())
		: CurrentDef->DisplayName;
}

FText UStoneActionSubsystem::GetActionDescriptionText() const
{
	if (!CurrentDef)
	{
		return FText::GetEmpty();
	}

	return CurrentDef->Description;
}

FText UStoneActionSubsystem::GetPhaseText() const
{
	switch (Phase)
	{
	case EStoneActionPhase::Outbound:   return FText::FromString(TEXT("On the way"));
	case EStoneActionPhase::Arrival:    return FText::FromString(TEXT("At destination"));
	case EStoneActionPhase::Return:     return FText::FromString(TEXT("Returning"));
	case EStoneActionPhase::Completed:  return FText::FromString(TEXT("Completed"));
	default:                            return FText::GetEmpty();
	}
}

float UStoneActionSubsystem::GetRemainingSeconds() const
{
	if (!bActionRunning || BaseDurationSeconds <= 0.f)
	{
		return 0.f;
	}

	const UStoneRunSubsystem* Run = GetRun();
	const float SimSpeed = Run ? Run->GetSimulationSpeed() : 1.f;
	const float SpeedMult = ResolveActionSpeedMult();

	const float RemainingBase = FMath::Max(0.f, BaseDurationSeconds - TotalElapsedBaseSeconds);
	const float Denom = FMath::Max(0.001f, SimSpeed * SpeedMult);

	// Remaining REAL seconds for UI.
	return RemainingBase / Denom;
}

bool UStoneActionSubsystem::IsPausedByGameState() const
{
	if (!bActionRunning)
	{
		return false;
	}

	if (const UStoneRunSubsystem* Run = GetRun())
	{
		return (Run->GetSimulationSpeed() <= 0.f)
			|| Run->HasOpenEvent()
			|| (Run->GetPendingEventCount() > 0);
	}

	return false;
}
