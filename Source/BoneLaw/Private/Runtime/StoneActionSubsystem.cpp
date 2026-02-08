#include "Runtime/StoneActionSubsystem.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "AbilitySystem/StoneAttributeSet.h"

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
	return FMath::Clamp(Score / 100.f, 0.10f, 10.0f);
}

void UStoneActionSubsystem::ApplyRunSideEffects()
{
	UStoneRunSubsystem* Run = GetRun();
	if (!Run || !CurrentDef)
	{
		return;
	}

	// Activate packs temporarily (SSOT via RunSubsystem)
	ActivatedPackIds.Reset();
	for (const FName& PackId : CurrentDef->PackIdsToActivate)
	{
		if (!PackId.IsNone())
		{
			Run->ActivatePackTemporary(PackId);
			ActivatedPackIds.AddUnique(PackId);
		}
	}

	// Apply state tags while action runs (this is what your Event Requirements rely on!)
	AppliedStateTags = CurrentDef->GrantedStateTags;
	if (!AppliedStateTags.IsEmpty())
	{
		Run->AddStateTags(AppliedStateTags);
	}
}

void UStoneActionSubsystem::RemoveRunSideEffects()
{
	UStoneRunSubsystem* Run = GetRun();
	if (!Run)
	{
		AppliedStateTags.Reset();
		ActivatedPackIds.Reset();
		return;
	}

	if (!AppliedStateTags.IsEmpty())
	{
		Run->RemoveStateTags(AppliedStateTags);
		AppliedStateTags.Reset();
	}

	if (ActivatedPackIds.Num() > 0)
	{
		Run->DeactivateTemporaryPacksByIds(ActivatedPackIds);
		ActivatedPackIds.Reset();
	}
}

bool UStoneActionSubsystem::StartAction(UStoneActionDefinitionData* ActionDef)
{
	if (!ActionDef) return false;

	StopCurrentAction(false);

	CurrentDef = ActionDef;
	bActionRunning = true;
	bReturnHomeQueued = false;

	BaseDurationSeconds = FMath::Max(1.f, ActionDef->BaseDurationSeconds);
	OutboundSeconds = BaseDurationSeconds * ActionDef->OutboundShare01;
	ReturnSeconds   = BaseDurationSeconds * ActionDef->ReturnShare01;

	Phase = EStoneActionPhase::Outbound;
	PhaseElapsedBaseSeconds = 0.f;
	TotalElapsedBaseSeconds = 0.f;

	// Precompute random times
	OutboundRandomTimes.Reset();
	ReturnRandomTimes.Reset();

	for (int32 i = 0; i < ActionDef->OutboundRandomCountMax; ++i)
	{
		if (RNG.FRand() <= ActionDef->OutboundRandomChance01)
		{
			OutboundRandomTimes.Add(
				RNG.FRandRange(ActionDef->OutboundRandomAtMin01, ActionDef->OutboundRandomAtMax01) * OutboundSeconds
			);
		}
	}
	for (int32 i = 0; i < ActionDef->ReturnRandomCountMax; ++i)
	{
		if (RNG.FRand() <= ActionDef->ReturnRandomChance01)
		{
			ReturnRandomTimes.Add(
				RNG.FRandRange(ActionDef->ReturnRandomAtMin01, ActionDef->ReturnRandomAtMax01) * ReturnSeconds
			);
		}
	}

	OutboundRandomTimes.Sort();
	ReturnRandomTimes.Sort();
	OutboundIndex = 0;
	ReturnIndex = 0;

	// **CRITICAL**: apply packs + tags so your arrival event requirements can pass
	ApplyRunSideEffects();

	// Timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ActionTickHandle, this,
			&UStoneActionSubsystem::TickAction,
			kActionTickInterval, true
		);
	}

	OnActionStateChanged.Broadcast();
	OnActionProgressChanged.Broadcast(0.f);

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
		if (!(Run->HasOpenEvent() || Run->GetPendingEventCount() > 0))
		{
			Run->QueueEventByTag(FStoneGameplayTags::Get().Event_Travel_ReturnHome, true);
		}
	}

	// revert applied packs/tags
	RemoveRunSideEffects();

	bActionRunning = false;
	Phase = EStoneActionPhase::None;

	CurrentDef = nullptr;
	BaseDurationSeconds = 0.f;
	OutboundSeconds = 0.f;
	ReturnSeconds = 0.f;
	PhaseElapsedBaseSeconds = 0.f;
	TotalElapsedBaseSeconds = 0.f;

	bReturnHomeQueued = false;

	OnActionStateChanged.Broadcast();
	OnActionProgressChanged.Broadcast(0.f);
}

float UStoneActionSubsystem::GetActionProgress01() const
{
	if (!bActionRunning || BaseDurationSeconds <= 0.f)
	{
		return 0.f;
	}
	return FMath::Clamp(TotalElapsedBaseSeconds / BaseDurationSeconds, 0.f, 1.f);
}

float UStoneActionSubsystem::GetPhaseProgress01() const
{
	if (!bActionRunning)
	{
		return 0.f;
	}

	const float PhaseDur =
		(Phase == EStoneActionPhase::Outbound) ? OutboundSeconds :
		(Phase == EStoneActionPhase::Return) ? ReturnSeconds : 0.f;

	if (PhaseDur <= 0.f)
	{
		return 0.f;
	}
	return FMath::Clamp(PhaseElapsedBaseSeconds / PhaseDur, 0.f, 1.f);
}

FText UStoneActionSubsystem::GetActionTitleText() const
{
	if (!CurrentDef) return FText::GetEmpty();
	return CurrentDef->DisplayName.IsEmpty()
		? FText::FromString(CurrentDef->GetName())
		: CurrentDef->DisplayName;
}

FText UStoneActionSubsystem::GetActionDescriptionText() const
{
	return CurrentDef ? CurrentDef->Description : FText::GetEmpty();
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

	return RemainingBase / Denom;
}

FGameplayTag UStoneActionSubsystem::GetLegRandomEventTag(EStoneActionPhase InPhase) const
{
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();
	if (!CurrentDef) return FGameplayTag();

	if (InPhase != EStoneActionPhase::Outbound && InPhase != EStoneActionPhase::Return)
	{
		return FGameplayTag();
	}

	return (InPhase == EStoneActionPhase::Outbound) ? T.Event_Travel_Outbound : T.Event_Travel_Return;
}

void UStoneActionSubsystem::EnterPhase(EStoneActionPhase NewPhase)
{
	Phase = NewPhase;
	PhaseElapsedBaseSeconds = 0.f;

	UStoneRunSubsystem* Run = GetRun();
	if (Run && Phase == EStoneActionPhase::Arrival)
	{
		// Arrival is a gate event: open immediately if possible
		Run->QueueEventByTag(FStoneGameplayTags::Get().Event_Travel_Arrival, true);
	}

	OnActionStateChanged.Broadcast();
}

void UStoneActionSubsystem::HandlePhaseAdvance()
{
	if (!bActionRunning) return;

	// Outbound -> Arrival
	if (Phase == EStoneActionPhase::Outbound)
	{
		if (PhaseElapsedBaseSeconds >= OutboundSeconds)
		{
			EnterPhase(EStoneActionPhase::Arrival); // queues arrival gate event
		}
		return;
	}

	// Return -> Completed (queue return-home once)
	if (Phase == EStoneActionPhase::Return)
	{
		if (PhaseElapsedBaseSeconds >= ReturnSeconds || TotalElapsedBaseSeconds >= BaseDurationSeconds)
		{
			Phase = EStoneActionPhase::Completed;

			if (!bReturnHomeQueued)
			{
				bReturnHomeQueued = true;
				if (UStoneRunSubsystem* Run = GetRun())
				{
					Run->QueueEventByTag(FStoneGameplayTags::Get().Event_Travel_ReturnHome, true);
				}
			}

			OnActionStateChanged.Broadcast();
		}
		return;
	}
}

void UStoneActionSubsystem::AdvancePhaseTimeline(float AdvanceBaseSeconds)
{
	if (!bActionRunning || AdvanceBaseSeconds <= 0.f) return;

	UStoneRunSubsystem* Run = GetRun();
	if (!Run) return;

	PhaseElapsedBaseSeconds += AdvanceBaseSeconds;
	TotalElapsedBaseSeconds += AdvanceBaseSeconds;

	// Clamp total only; phase clamp happens naturally via phase duration checks.
	TotalElapsedBaseSeconds = FMath::Min(TotalElapsedBaseSeconds, BaseDurationSeconds);

	// ----------------------------
	// RANDOM LEG EVENTS (RESTORED)
	// ----------------------------
	// Only for Outbound/Return legs.
	if (Phase == EStoneActionPhase::Outbound)
	{
		while (OutboundIndex < OutboundRandomTimes.Num() &&
		       PhaseElapsedBaseSeconds >= OutboundRandomTimes[OutboundIndex])
		{
			++OutboundIndex;

			const FGameplayTag Tag = GetLegRandomEventTag(Phase); // Event.Travel.Outbound
			if (Tag.IsValid())
			{
				Run->QueueEventByTag(Tag, true);

				// If an event opened or got queued -> pause timeline until player resolves it.
				if (Run->HasOpenEvent() || Run->GetPendingEventCount() > 0)
				{
					return;
				}
			}
		}
	}
	else if (Phase == EStoneActionPhase::Return)
	{
		while (ReturnIndex < ReturnRandomTimes.Num() &&
		       PhaseElapsedBaseSeconds >= ReturnRandomTimes[ReturnIndex])
		{
			++ReturnIndex;

			const FGameplayTag Tag = GetLegRandomEventTag(Phase); // Event.Travel.Return
			if (Tag.IsValid())
			{
				Run->QueueEventByTag(Tag, true);

				if (Run->HasOpenEvent() || Run->GetPendingEventCount() > 0)
				{
					return;
				}
			}
		}
	}

	// After random leg events, advance phase if we reached end of the leg.
	HandlePhaseAdvance();
}

void UStoneActionSubsystem::TickAction()
{
	if (!bActionRunning || !CurrentDef) return;

	UStoneRunSubsystem* Run = GetRun();
	if (!Run) { StopCurrentAction(false); return; }

	const float SimSpeed = Run->GetSimulationSpeed();
	if (SimSpeed <= 0.f)
	{
		return;
	}

	// Pause gate: do not advance while any event is open or pending.
	const bool bEventBlocking = Run->HasOpenEvent() || Run->GetPendingEventCount() > 0;
	if (bEventBlocking)
	{
		return;
	}

	// Completed -> shutdown
	if (Phase == EStoneActionPhase::Completed)
	{
		StopCurrentAction(false);
		return;
	}

	// Arrival gate: once arrival event resolved (we are here => not blocking), continue to Return.
	if (Phase == EStoneActionPhase::Arrival)
	{
		EnterPhase(EStoneActionPhase::Return);
		OnActionProgressChanged.Broadcast(GetActionProgress01());
		return;
	}

	const float SpeedMult = ResolveActionSpeedMult();
	const float AdvanceBaseSeconds = kActionTickInterval * SimSpeed * SpeedMult;
	if (AdvanceBaseSeconds <= 0.f)
	{
		return;
	}

	// ✅ advance timeline (includes random leg events + phase changes)
	AdvancePhaseTimeline(AdvanceBaseSeconds);

	OnActionProgressChanged.Broadcast(GetActionProgress01());
}