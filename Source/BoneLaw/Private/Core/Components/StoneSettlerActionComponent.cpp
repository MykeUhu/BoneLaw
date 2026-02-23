#include "Core/Components/StoneSettlerActionComponent.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

#include "Runtime/StoneRunSubsystem.h"
#include "Data/StoneActionDefinitionData.h"
#include "Core/StoneGameplayTags.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "Runtime/StoneActionTypes.h"

static constexpr float kActionTickInterval = 0.25f;

UStoneSettlerActionComponent::UStoneSettlerActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStoneSettlerActionComponent::BeginPlay()
{
	Super::BeginPlay();
	RNG.Initialize(static_cast<int32>(FPlatformTime::Cycles64() + reinterpret_cast<PTRINT>(this)));
}

void UStoneSettlerActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopCurrentAction(false);
	Super::EndPlay(EndPlayReason);
}

UAbilitySystemComponent* UStoneSettlerActionComponent::GetASC() const
{
	if (AActor* Owner = GetOwner())
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
		{
			return ASI->GetAbilitySystemComponent();
		}
	}
	return nullptr;
}

UStoneRunSubsystem* UStoneSettlerActionComponent::GetRun() const
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

float UStoneSettlerActionComponent::ResolveActionSpeedMult() const
{
	if (!CurrentDef)
	{
		return 1.f;
	}

	UAbilitySystemComponent* ASC = GetASC();
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

void UStoneSettlerActionComponent::ApplyRunSideEffects()
{
	UStoneRunSubsystem* Run = GetRun();
	if (!Run || !CurrentDef)
	{
		return;
	}

	// MP READY: Authority check
	if (UWorld* World = GetWorld())
	{
		if (World->GetNetMode() == NM_Client)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] ApplyRunSideEffects on client - ignoring"));
			return;
		}
	}

	// Activate event packs temporarily
	ActivatedPackIds.Reset();
	for (const FName& PackId : CurrentDef->PackIdsToActivate)
	{
		if (!PackId.IsNone())
		{
			Run->ActivatePackTemporary(PackId);
			ActivatedPackIds.AddUnique(PackId);
		}
	}

	// Apply state tags (event requirements rely on these!)
	AppliedStateTags = CurrentDef->GrantedStateTags;
	if (!AppliedStateTags.IsEmpty())
	{
		Run->AddStateTags(AppliedStateTags);
	}
}

void UStoneSettlerActionComponent::RemoveRunSideEffects()
{
	UStoneRunSubsystem* Run = GetRun();
	if (!Run)
	{
		AppliedStateTags.Reset();
		ActivatedPackIds.Reset();
		return;
	}

	// MP READY: Authority check
	if (UWorld* World = GetWorld())
	{
		if (World->GetNetMode() == NM_Client)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] RemoveRunSideEffects on client - ignoring"));
			AppliedStateTags.Reset();
			ActivatedPackIds.Reset();
			return;
		}
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

bool UStoneSettlerActionComponent::StartAction(UStoneActionDefinitionData* ActionDef)
{
	if (!ActionDef)
	{
		return false;
	}

	StopCurrentAction(false);

	CurrentDef = ActionDef;
	bActionRunning = true;
	bReturnHomeQueued = false;

	BaseDurationSeconds = FMath::Max(1.f, ActionDef->BaseDurationSeconds);

	OutboundSeconds = BaseDurationSeconds * ActionDef->OutboundShare01;
	ReturnSeconds   = BaseDurationSeconds * ActionDef->ReturnShare01;

	// ✅ NEW: actual work time between outbound and return
	ArrivalSeconds  = FMath::Max(0.f, BaseDurationSeconds - OutboundSeconds - ReturnSeconds);

	Phase = EStoneActionPhase::Outbound;
	PhaseElapsedBaseSeconds = 0.f;
	TotalElapsedBaseSeconds = 0.f;

	// Precompute random event times
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

	// Apply packs + tags so arrival event requirements can pass
	ApplyRunSideEffects();

	// Start tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ActionTickHandle,
			this,
			&UStoneSettlerActionComponent::TickAction,
			kActionTickInterval,
			true
		);
	}

	OnActionStateChanged.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Started action: %s  Base=%.2fs Out=%.2fs Work=%.2fs Ret=%.2fs"),
		*ActionDef->GetName(), BaseDurationSeconds, OutboundSeconds, ArrivalSeconds, ReturnSeconds);

	return true;
}

void UStoneSettlerActionComponent::StopCurrentAction(bool bForceReturnHomeEvent)
{
	if (!bActionRunning)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActionTickHandle);
	}

	RemoveRunSideEffects();

	// Trigger return home event if requested
	UStoneRunSubsystem* Run = GetRun();
	if (bForceReturnHomeEvent && Run)
	{
		if (!(Run->HasOpenEvent() || Run->GetPendingEventCount() > 0))
		{
			Run->QueueEventByTag(FStoneGameplayTags::Get().Event_Travel_ReturnHome, true);
		}
	}

	bActionRunning = false;
	Phase = EStoneActionPhase::None;
	CurrentDef = nullptr;

	// ✅ reset cached timings
	BaseDurationSeconds = 0.f;
	OutboundSeconds = 0.f;
	ArrivalSeconds = 0.f;
	ReturnSeconds = 0.f;
	PhaseElapsedBaseSeconds = 0.f;
	TotalElapsedBaseSeconds = 0.f;

	OnActionStateChanged.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Stopped action"));
}

void UStoneSettlerActionComponent::TickAction()
{
	if (!bActionRunning || !CurrentDef)
	{
		return;
	}

	UStoneRunSubsystem* Run = GetRun();
	if (!Run)
	{
		StopCurrentAction(false);
		return;
	}

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

	const float SpeedMult = ResolveActionSpeedMult();
	const float AdvanceBaseSeconds = kActionTickInterval * SimSpeed * SpeedMult;
	if (AdvanceBaseSeconds <= 0.f)
	{
		return;
	}

	AdvancePhaseTimeline(AdvanceBaseSeconds);
	OnActionProgressChanged.Broadcast(GetActionProgress01());
}

void UStoneSettlerActionComponent::AdvancePhaseTimeline(float AdvanceBaseSeconds)
{
	if (!bActionRunning || !CurrentDef)
	{
		return;
	}

	PhaseElapsedBaseSeconds += AdvanceBaseSeconds;
	TotalElapsedBaseSeconds += AdvanceBaseSeconds;

	// Fire random events during outbound/return legs
	UStoneRunSubsystem* Run = GetRun();
	if (Run)
	{
		if (Phase == EStoneActionPhase::Outbound)
		{
			while (OutboundIndex < OutboundRandomTimes.Num() &&
				PhaseElapsedBaseSeconds >= OutboundRandomTimes[OutboundIndex])
			{
				FGameplayTag EventTag = GetLegRandomEventTag(EStoneActionPhase::Outbound);
				if (EventTag.IsValid())
				{
					Run->QueueEventByTag(EventTag, true);

					if (Run->HasOpenEvent() || Run->GetPendingEventCount() > 0)
					{
						return; // ✅ pause timeline until player resolves event
					}
				}
				++OutboundIndex;
			}
		}
		else if (Phase == EStoneActionPhase::Return)
		{
			while (ReturnIndex < ReturnRandomTimes.Num() &&
				PhaseElapsedBaseSeconds >= ReturnRandomTimes[ReturnIndex])
			{
				FGameplayTag EventTag = GetLegRandomEventTag(EStoneActionPhase::Return);
				if (EventTag.IsValid())
				{
					Run->QueueEventByTag(EventTag, true);

					if (Run->HasOpenEvent() || Run->GetPendingEventCount() > 0)
					{
						return; // ✅ pause timeline until player resolves event
					}
				}
				++ReturnIndex;
			}
		}
	}

	HandlePhaseAdvance();
}

void UStoneSettlerActionComponent::HandlePhaseAdvance()
{
	if (!bActionRunning)
	{
		return;
	}

	// Outbound -> Arrival
	if (Phase == EStoneActionPhase::Outbound)
	{
		if (PhaseElapsedBaseSeconds >= OutboundSeconds)
		{
			EnterPhase(EStoneActionPhase::Arrival);
		}
		return;
	}

	// ✅ Arrival (Work) -> Return
	if (Phase == EStoneActionPhase::Arrival)
	{
		// If there is no work time configured, immediately continue.
		if (ArrivalSeconds <= 0.f || PhaseElapsedBaseSeconds >= ArrivalSeconds)
		{
			EnterPhase(EStoneActionPhase::Return);
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
					const FStoneGameplayTags& T = FStoneGameplayTags::Get();
					FGameplayTag ReturnHomeTag = T.Event_Travel_ReturnHome;

					if (CurrentDef)
					{
						switch (CurrentDef->ActionType)
						{
						case EStoneActionType::Travel:  ReturnHomeTag = T.Action_Travel_ReturnHome; break;
						case EStoneActionType::Gather:  ReturnHomeTag = T.Action_Gather_ReturnHome; break;
						case EStoneActionType::Explore: ReturnHomeTag = T.Action_Explore_ReturnHome; break;
						case EStoneActionType::Custom:
						default:                        ReturnHomeTag = T.Event_Travel_ReturnHome; break;
						}
					}

					Run->QueueEventByTag(ReturnHomeTag, true);
				}
			}

			OnActionStateChanged.Broadcast();
		}
		return;
	}
}

void UStoneSettlerActionComponent::EnterPhase(EStoneActionPhase NewPhase)
{
	Phase = NewPhase;
	PhaseElapsedBaseSeconds = 0.f;

	UStoneRunSubsystem* Run = GetRun();
	if (Run && Phase == EStoneActionPhase::Arrival)
	{
		const FStoneGameplayTags& T = FStoneGameplayTags::Get();
		FGameplayTag ArrivalTag = T.Event_Travel_Arrival;

		if (CurrentDef)
		{
			switch (CurrentDef->ActionType)
			{
			case EStoneActionType::Travel:   ArrivalTag = T.Action_Travel_Arrival; break;
			case EStoneActionType::Gather:   ArrivalTag = T.Action_Gather_Arrival; break;
			case EStoneActionType::Explore:  ArrivalTag = T.Action_Explore_Arrival; break;
			case EStoneActionType::Custom:
			default:                         ArrivalTag = T.Event_Travel_Arrival; break;
			}
		}

		Run->QueueEventByTag(ArrivalTag, true);
	}

	OnActionStateChanged.Broadcast();
}

FGameplayTag UStoneSettlerActionComponent::GetLegRandomEventTag(EStoneActionPhase InPhase) const
{
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();
	if (!CurrentDef) return FGameplayTag();

	if (InPhase != EStoneActionPhase::Outbound && InPhase != EStoneActionPhase::Return)
	{
		return FGameplayTag();
	}

	switch (CurrentDef->ActionType)
	{
	case EStoneActionType::Travel:
		return (InPhase == EStoneActionPhase::Outbound) ? T.Action_Travel_Outbound : T.Action_Travel_Return;

	case EStoneActionType::Gather:
		return (InPhase == EStoneActionPhase::Outbound) ? T.Action_Gather_Outbound : T.Action_Gather_Return;

	case EStoneActionType::Explore:
		return (InPhase == EStoneActionPhase::Outbound) ? T.Action_Explore_Outbound : T.Action_Explore_Return;

	case EStoneActionType::Custom:
	default:
		return (InPhase == EStoneActionPhase::Outbound) ? T.Event_Travel_Outbound : T.Event_Travel_Return;
	}
}

float UStoneSettlerActionComponent::GetActionProgress01() const
{
	if (!bActionRunning || BaseDurationSeconds <= 0.f)
	{
		return 0.f;
	}
	return FMath::Clamp(TotalElapsedBaseSeconds / BaseDurationSeconds, 0.f, 1.f);
}

float UStoneSettlerActionComponent::GetPhaseProgress01() const
{
	if (!bActionRunning || !CurrentDef)
	{
		return 0.f;
	}

	float PhaseDuration = 0.f;
	switch (Phase)
	{
	case EStoneActionPhase::Outbound:
		PhaseDuration = OutboundSeconds;
		break;

	case EStoneActionPhase::Arrival:
		// ✅ Use cached value so we don't recompute inconsistently
		PhaseDuration = ArrivalSeconds;
		break;

	case EStoneActionPhase::Return:
		PhaseDuration = ReturnSeconds;
		break;

	default:
		return 0.f;
	}

	if (PhaseDuration <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(PhaseElapsedBaseSeconds / PhaseDuration, 0.f, 1.f);
}

FText UStoneSettlerActionComponent::GetActionTitleText() const
{
	if (CurrentDef)
	{
		return CurrentDef->DisplayName.IsEmpty()
	? FText::FromString(CurrentDef->GetName())
	: CurrentDef->DisplayName;
	}
	return FText::GetEmpty();
}

FText UStoneSettlerActionComponent::GetActionDescriptionText() const
{
	if (CurrentDef)
	{
		return CurrentDef ? CurrentDef->Description : FText::GetEmpty();
	}
	return FText::GetEmpty();
}

FText UStoneSettlerActionComponent::GetPhaseText() const
{
	switch (Phase)
	{
	case EStoneActionPhase::Outbound:
		return FText::FromString(TEXT("Traveling..."));
	case EStoneActionPhase::Arrival:
		return FText::FromString(TEXT("Working..."));
	case EStoneActionPhase::Return:
		return FText::FromString(TEXT("Returning..."));
	default:
		return FText::GetEmpty();
	}
}

float UStoneSettlerActionComponent::GetRemainingSeconds() const
{
	if (!bActionRunning || BaseDurationSeconds <= 0.f)
	{
		return 0.f;
	}

	const float SpeedMult = ResolveActionSpeedMult();
	if (SpeedMult <= 0.f)
	{
		return 999999.f;
	}

	const float RemainingBase = BaseDurationSeconds - TotalElapsedBaseSeconds;
	return FMath::Max(0.f, RemainingBase / SpeedMult);
}
