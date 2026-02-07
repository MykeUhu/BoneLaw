#include "Runtime/StoneActionSubsystem.h"

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

bool UStoneActionSubsystem::StartAction(UStoneActionDefinitionData* ActionDef)
{
	if (!ActionDef)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneActionSubsystem] StartAction: ActionDef is null"));
		return false;
	}

	if (bActionRunning)
	{
		StopCurrentAction(false);
	}

	UStoneRunSubsystem* Run = GetRun();
	if (!Run)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneActionSubsystem] StartAction: RunSubsystem missing"));
		return false;
	}

	// Demo rule: do not stack real-time actions across systems.
	if (Run->IsOnExpedition())
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneActionSubsystem] StartAction: stopping active expedition (no stacking actions)."));
		Run->StopExpedition(/*bForceReturnEvent*/ false);
	}
	if (Run->IsTravelActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneActionSubsystem] StartAction: stopping active travel action (no stacking actions)."));
		Run->StopTravelAction(/*bForceReturnHomeEvent*/ false);
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

	// -------------------------
	// Cache timing (SSOT)
	// -------------------------
	BaseDurationSeconds = FMath::Max(1.f, ActionDef->BaseDurationSeconds);

	// SSOT from the data asset: only enforce TECHNICAL validity (no gameplay-magic clamps).
	const float TickMin = kActionTickInterval; // minimum meaningful interval for countdown/tick
	RandomMinGapSeconds = FMath::Max(TickMin, ActionDef->RandomMinGapSeconds);
	RandomMaxGapSeconds = FMath::Max(RandomMinGapSeconds, ActionDef->RandomMaxGapSeconds);

	RandomChance01 = FMath::Clamp(ActionDef->RandomChance01, 0.f, 1.f);
	bAllowImmediateRandom = ActionDef->bAllowImmediateRandom;

	// Travel split
	const float OutShare = FMath::Clamp(ActionDef->OutboundShare01, 0.f, 1.f);
	const float RetShare = FMath::Clamp(ActionDef->ReturnShare01, 0.f, 1.f);
	const float Sum = FMath::Max(KINDA_SMALL_NUMBER, OutShare + RetShare);

	OutboundSeconds = BaseDurationSeconds * (OutShare / Sum);
	ReturnSeconds   = BaseDurationSeconds * (RetShare / Sum);

	PhaseElapsedSeconds = 0.f;
	TotalElapsedSeconds = 0.f;

	// Activate packs only while action runs
	ApplyActionPacks();

	EnterPhase(EStoneActionPhase::Outbound);

	// First tick after phase set should not roll random
	bSkipRandomThisTick = true;
	bReturnHomeQueued = false;

	NextRandomCountdownSeconds = bAllowImmediateRandom ? 0.f : RNG.FRandRange(RandomMinGapSeconds, RandomMaxGapSeconds);

	// Start tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ActionTickHandle, this, &UStoneActionSubsystem::TickAction, kActionTickInterval, true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneActionSubsystem] StartAction: World is null; cannot start ticking."));
		StopCurrentAction(false);
		return false;
	}

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

	UE_LOG(LogTemp, Warning,
		TEXT("[StoneActionSubsystem] StopCurrentAction: Phase=%d TotalElapsed=%.2f Base=%.2f ForceReturn=%s"),
		(int32)Phase, TotalElapsedSeconds, BaseDurationSeconds, bForceReturnHomeEvent ? TEXT("true") : TEXT("false"));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActionTickHandle);
	}

	UStoneRunSubsystem* Run = GetRun();

	// Optional final event request (if you want return home event guaranteed)
	if (bForceReturnHomeEvent && Run)
	{
		if (Run->HasOpenEvent())
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneActionSubsystem] StopCurrentAction: return-home event requested but an event is already open; not queuing."));
		}
		else
		{
			const int32 PendingBefore = Run->GetPendingEventCount();
			const FStoneGameplayTags& T = FStoneGameplayTags::Get();
			Run->QueueEventByTag(T.Event_Travel_ReturnHome, /*bAutoPresent*/ true);
			const int32 PendingAfter = Run->GetPendingEventCount();
			UE_LOG(LogTemp, Warning,
				TEXT("[StoneActionSubsystem] StopCurrentAction: return-home queue attempt done. PendingEvents %d -> %d OpenEvent=%s"),
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

	bActionRunning = false;
	Phase = EStoneActionPhase::None;

	CurrentDef = nullptr;
	BaseDurationSeconds = 0.f;
	OutboundSeconds = 0.f;
	ReturnSeconds = 0.f;
	PhaseElapsedSeconds = 0.f;
	TotalElapsedSeconds = 0.f;
	NextRandomCountdownSeconds = 0.f;

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
	int32 ActivatedNow = 0;

	for (const FName& PackId : CurrentDef->PackIdsToActivate)
	{
		if (PackId.IsNone())
		{
			continue;
		}
		++Attempts;
		const bool bAlreadyActive = Run->GetActivePackIds().Contains(PackId);
		Run->ActivatePackTemporary(PackId);
		const bool bIsActiveAfter = Run->GetActivePackIds().Contains(PackId);
		if (bIsActiveAfter)
		{
			ActivatedNow += bAlreadyActive ? 0 : 1;
			ActivatedPackIds.AddUnique(PackId);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneActionSubsystem] ApplyActionPacks: pack '%s' did not become active (see earlier errors)."), *PackId.ToString());
		}
	}

	const int32 ActiveAfter = Run->GetActivePackIds().Num();
	UE_LOG(LogTemp, Warning,
		TEXT("[StoneActionSubsystem] ApplyActionPacks: Attempts=%d AddedNow=%d ActivePacks %d -> %d"),
		Attempts, ActivatedNow, ActiveBefore, ActiveAfter);
}

void UStoneActionSubsystem::ClearActionPacks()
{
	UStoneRunSubsystem* Run = GetRun();
	if (!Run)
	{
		ActivatedPackIds.Reset();
		return;
	}

	// Deactivate ONLY the packs that this action activated.
	Run->DeactivateTemporaryPacksByIds(ActivatedPackIds);
	ActivatedPackIds.Reset();
}

float UStoneActionSubsystem::GetActionProgress01() const
{
	if (!bActionRunning || BaseDurationSeconds <= 0.f)
	{
		return 0.f;
	}
	return FMath::Clamp(TotalElapsedSeconds / BaseDurationSeconds, 0.f, 1.f);
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
	return FMath::Clamp(PhaseElapsedSeconds / LegDur, 0.f, 1.f);
}

void UStoneActionSubsystem::EnterPhase(EStoneActionPhase NewPhase)
{
	Phase = NewPhase;
	PhaseElapsedSeconds = 0.f;

	// Phase transitions can cause boundary artifacts (random roll + phase switch in same tick).
	// We explicitly skip random rolls for this tick.
	bSkipRandomThisTick = true;

	// Gate events must be queued BEFORE broadcasting UI state,
	// otherwise the UI briefly shows "Arrived" without the event panel.
	if (UStoneRunSubsystem* Run = GetRun())
	{
		const FStoneGameplayTags& T = FStoneGameplayTags::Get();

		if (Phase == EStoneActionPhase::Arrival)
		{
			Run->QueueEventByTag(T.Event_Travel_Arrival, /*bAutoPresent*/ true);
		}

		// IMPORTANT:
		// Do NOT queue Event_Travel_Return at the start of Return.
		// Return leg events are random and should fire DURING the leg via RollRandomTravelEvent().
	}

	// Now broadcast (UI may switch panels, update phase text, etc.)
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
		// We are done traveling back. Queue a deterministic "return home" gate event if desired/content exists.
		// We do NOT stop the action immediately if we just queued an event; the pause gate will freeze time.
		Phase = EStoneActionPhase::Completed;

		if (!bReturnHomeQueued)
		{
			bReturnHomeQueued = true;

			if (UStoneRunSubsystem* Run = GetRun())
			{
				const FStoneGameplayTags& T = FStoneGameplayTags::Get();
				Run->QueueEventByTag(T.Event_Travel_ReturnHome, /*bAutoPresent*/ true);
			}
		}

		// Let TickAction() decide when to finalize StopCurrentAction()
		// (it will stop once Completed AND not event-blocked).
		OnActionStateChanged.Broadcast();
		return;
	}
}

void UStoneActionSubsystem::RollRandomTravelEvent()
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
	if (!Run || Run->HasOpenEvent())
	{
		return;
	}

	const float Roll = RNG.FRand();
	if (Roll <= RandomChance01)
	{
		const FStoneGameplayTags& T = FStoneGameplayTags::Get();
		const FGameplayTag Tag = (Phase == EStoneActionPhase::Outbound)
			? T.Event_Travel_Outbound
			: T.Event_Travel_Return;

		Run->QueueEventByTag(Tag, /*bAutoPresent*/ true);
	}

	NextRandomCountdownSeconds = RNG.FRandRange(RandomMinGapSeconds, RandomMaxGapSeconds);
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
	if (Run->GetSimulationSpeed() <= 0.f || bEventBlocking)
	{
		return;
	}

	// If we reached Completed earlier and no longer have events blocking, we can finalize shutdown.
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

	const float Dt = kActionTickInterval * Run->GetSimulationSpeed();
	if (Dt <= 0.f)
	{
		return;
	}

	// ----- PHASE BOUNDARY GUARD -----
	// If this tick would complete the current leg, we advance phase BEFORE any random rolls.
	// This prevents "on-the-way" random events from firing at the exact moment we arrive/return.
	const bool bWillFinishOutbound = (Phase == EStoneActionPhase::Outbound) && ((PhaseElapsedSeconds + Dt) >= OutboundSeconds);
	const bool bWillFinishReturn   = (Phase == EStoneActionPhase::Return)   && ((PhaseElapsedSeconds + Dt) >= ReturnSeconds);

	if (bWillFinishOutbound || bWillFinishReturn)
	{
		TotalElapsedSeconds += Dt;
		PhaseElapsedSeconds += Dt;

		HandlePhaseAdvance();
		OnActionProgressChanged.Broadcast(GetActionProgress01());
		return;
	}

	// Normal time advance.
	TotalElapsedSeconds += Dt;
	PhaseElapsedSeconds += Dt;

	// ----- RANDOM TRAVEL EVENT -----
	// Random events only during Outbound/Return legs and never on the same tick as a phase change.
	if (!bSkipRandomThisTick && (Phase == EStoneActionPhase::Outbound || Phase == EStoneActionPhase::Return))
	{
		NextRandomCountdownSeconds -= Dt;
		if (NextRandomCountdownSeconds <= 0.f)
		{
			RollRandomTravelEvent();

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
	case EStoneActionPhase::Arrival:    return FText::FromString(TEXT("Arrived"));
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

	return FMath::Max(0.f, BaseDurationSeconds - TotalElapsedSeconds);
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