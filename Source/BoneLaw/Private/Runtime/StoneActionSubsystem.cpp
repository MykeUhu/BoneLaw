#include "Runtime/StoneActionSubsystem.h"

#include "Engine/World.h"
#include "TimerManager.h"

#include "Runtime/StoneRunSubsystem.h"
#include "Data/StoneActionDefinitionData.h"
#include "Core/StoneGameplayTags.h"

static constexpr float kActionTickInterval = 0.25f;
static constexpr float kMinRandomGapFloorSeconds = 5.0f;

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
	// If RunSubsystem currently runs an expedition/travel, we stop it deterministically before starting a new action.
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

	// Cache timing
	BaseDurationSeconds = FMath::Max(1.f, ActionDef->BaseDurationSeconds);
	RandomMinGapSeconds = FMath::Max(kMinRandomGapFloorSeconds, ActionDef->RandomMinGapSeconds);
	RandomMaxGapSeconds = FMath::Max(RandomMinGapSeconds, ActionDef->RandomMaxGapSeconds);
	RandomChance01 = FMath::Clamp(ActionDef->RandomChance01, 0.f, 1.f);
	bAllowImmediateRandom = ActionDef->bAllowImmediateRandom;

	// Travel split
	const float OutShare = FMath::Clamp(ActionDef->OutboundShare01, 0.f, 1.f);
	const float RetShare = FMath::Clamp(ActionDef->ReturnShare01, 0.f, 1.f);
	const float Sum = FMath::Max(KINDA_SMALL_NUMBER, OutShare + RetShare);

	OutboundSeconds = BaseDurationSeconds * (OutShare / Sum);
	ReturnSeconds = BaseDurationSeconds * (RetShare / Sum);

	PhaseElapsedSeconds = 0.f;
	TotalElapsedSeconds = 0.f;

	// Activate packs only while action runs
	ApplyActionPacks();

	// Grant runtime tags while action runs (optional)
	Run->AddStateTags(ActionDef->GrantedStateTags);

	EnterPhase(EStoneActionPhase::Outbound);

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

	// Remove granted tags
	if (Run && CurrentDef)
	{
		Run->RemoveStateTags(CurrentDef->GrantedStateTags);
	}

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
			// ActivatePackTemporary logs detailed errors when a pack cannot be loaded.
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
	// This prevents action subsystem from clearing temporary packs used by other systems.
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

	// Arrival must trigger an event immediately
	if (Phase == EStoneActionPhase::Arrival)
	{
		if (UStoneRunSubsystem* Run = GetRun())
		{
			const FStoneGameplayTags& T = FStoneGameplayTags::Get();
			Run->QueueEventByTag(T.Event_Travel_Arrival, /*bAutoPresent*/ true);
		}
	}
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
		StopCurrentAction(false);
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

	// Pause: SimulationSpeed==0 OR event open -> no time advances
	if (Run->GetSimulationSpeed() <= 0.f || Run->HasOpenEvent())
	{
		return;
	}

	const float Dt = kActionTickInterval * Run->GetSimulationSpeed();
	if (Dt <= 0.f)
	{
		return;
	}

	TotalElapsedSeconds += Dt;
	PhaseElapsedSeconds += Dt;

	NextRandomCountdownSeconds -= Dt;
	if (NextRandomCountdownSeconds <= 0.f)
	{
		RollRandomTravelEvent();
	}

	// Phase timing
	if (Phase == EStoneActionPhase::Outbound && PhaseElapsedSeconds >= OutboundSeconds)
	{
		HandlePhaseAdvance();
	}
	else if (Phase == EStoneActionPhase::Return && PhaseElapsedSeconds >= ReturnSeconds)
	{
		HandlePhaseAdvance();
	}

	OnActionProgressChanged.Broadcast(GetActionProgress01());
}
