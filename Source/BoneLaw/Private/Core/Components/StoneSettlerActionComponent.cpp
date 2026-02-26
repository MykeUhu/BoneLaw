// Copyright by MykeUhu

#include "Core/Components/StoneSettlerActionComponent.h"

// Project
#include "Core/StoneGameplayTags.h"
#include "Data/StoneEventData.h"
#include "Data/StoneActionDefinitionData.h"

// Engine
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/AssetManager.h"
#include "Core/Character/StoneSettlerChar.h"

static constexpr float kActionTickInterval = 0.25f;

UStoneSettlerActionComponent::UStoneSettlerActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	RNG.Initialize(FPlatformTime::Cycles());
}

void UStoneSettlerActionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStoneSettlerActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopInternal(false, false);
	Super::EndPlay(EndPlayReason);
}

UAbilitySystemComponent* UStoneSettlerActionComponent::GetASC() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] GetASC failed: Owner is null."));
		return nullptr;
	}

	// Best practice: prefer IAbilitySystemInterface
	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OwnerActor))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			return ASC;
		}
	}

	// Fallback: component search (keeps compatibility)
	if (UAbilitySystemComponent* ASC = OwnerActor->FindComponentByClass<UAbilitySystemComponent>())
	{
		return ASC;
	}

	UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] GetASC failed: Owner '%s' has no ASC."), *OwnerActor->GetName());
	return nullptr;
}

bool UStoneSettlerActionComponent::StartAction(UStoneActionDefinitionData* ActionDef)
{
	// If same action already running -> do nothing (prevents BT spam restart)
	if (bActionRunning && CurrentDef == ActionDef)
	{
		return true;
	}

	// If ANY action running and BT re-enters, decide: ignore OR fail.
	// For your BT setup: ignore is safest.
	if (bActionRunning && CurrentDef != nullptr)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[SettlerAction] StartAction ignored: action already running. Owner=%s Current=%s New=%s"),
			*GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef), *GetNameSafe(ActionDef));
		return false; // or true if you want “already running” treated as success
	}
	if (!ActionDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] StartAction failed: ActionDef is null. Owner=%s"), *GetNameSafe(GetOwner()));
		return false;
	}

	StopCurrentAction(false);

	CurrentDef = ActionDef;
	bActionRunning = true;
	bReturnHomeQueued = false;
	
	// Optional: apply action-scoped tags to the Settler ASC for the lifetime of this action.
	// WARNING: Do not put BT phase/state tags here. Those are managed by EnterPhase() SSOT.
	AppliedStateTags = CurrentDef->GrantedStateTags;

	if (AppliedStateTags.Num() > 0)
	{
		if (UAbilitySystemComponent* ASC = GetASC())
		{
			ASC->AddLooseGameplayTags(AppliedStateTags);

			UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Applied GrantedStateTags x%d to ASC. Owner=%s Def=%s"),
				AppliedStateTags.Num(), *GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] GrantedStateTags present but ASC is null. Owner=%s Def=%s"),
				*GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef));
		}
	}

	BaseDurationSeconds = FMath::Max(1.f, ActionDef->BaseDurationSeconds);

	OutboundSeconds = BaseDurationSeconds * ActionDef->OutboundShare01;
	ReturnSeconds   = BaseDurationSeconds * ActionDef->ReturnShare01;

	ArrivalSeconds  = FMath::Max(0.f, BaseDurationSeconds - OutboundSeconds - ReturnSeconds);

	Phase = EStoneActionPhase::Outbound;
	PhaseElapsedBaseSeconds = 0.f;
	TotalElapsedBaseSeconds = 0.f;

	OutboundEncounterSlots.Reset();
	ReturnEncounterSlots.Reset();

	bEncounterOpen = false;
	CurrentEncounterTag = FGameplayTag();
	bLastEncounterAborted = false;

	// Pre-roll encounter slots using progress01 thresholds (NOT wall-clock seconds).
	for (int32 i = 0; i < ActionDef->OutboundRandomCountMax; ++i)
	{
		if (RNG.FRand() <= ActionDef->OutboundRandomChance01)
		{
			FStonePlannedEncounter Slot;
			Slot.EventTag = GetLegRandomEventTag(EStoneActionPhase::Outbound);
			Slot.TriggerAtProgress01 = RNG.FRandRange(ActionDef->OutboundRandomAtMin01, ActionDef->OutboundRandomAtMax01);
			Slot.bTriggered = false;
			OutboundEncounterSlots.Add(Slot);
		}
	}

	for (int32 i = 0; i < ActionDef->ReturnRandomCountMax; ++i)
	{
		if (RNG.FRand() <= ActionDef->ReturnRandomChance01)
		{
			FStonePlannedEncounter Slot;
			Slot.EventTag = GetLegRandomEventTag(EStoneActionPhase::Return);
			Slot.TriggerAtProgress01 = RNG.FRandRange(ActionDef->ReturnRandomAtMin01, ActionDef->ReturnRandomAtMax01);
			Slot.bTriggered = false;
			ReturnEncounterSlots.Add(Slot);
		}
	}

	OutboundEncounterSlots.Sort([](const FStonePlannedEncounter& A, const FStonePlannedEncounter& B)
	{
		return A.TriggerAtProgress01 < B.TriggerAtProgress01;
	});
	ReturnEncounterSlots.Sort([](const FStonePlannedEncounter& A, const FStonePlannedEncounter& B)
	{
		return A.TriggerAtProgress01 < B.TriggerAtProgress01;
	});

	// IMPORTANT:
	// No RunSubsystem. All state is per-settler; state tags/effects are applied on the Settler ASC.
	// Called in Behaivor Tree
	//EnterPhase(EStoneActionPhase::Outbound);

	// Start tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ActionTickHandle, this, &UStoneSettlerActionComponent::TickAction, kActionTickInterval, true);
	}

	OnActionStateChanged.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Started action: %s Owner=%s Base=%.2fs Out=%.2fs Work=%.2fs Ret=%.2fs"),
		*ActionDef->GetName(), *GetNameSafe(GetOwner()), BaseDurationSeconds, OutboundSeconds, ArrivalSeconds, ReturnSeconds);

	return true;
}

void UStoneSettlerActionComponent::StopAction(bool bSuccess)
{
	StopInternal(bSuccess, /*bForceReturnHomeEvent*/ false);
}

void UStoneSettlerActionComponent::StopCurrentAction(bool bForceReturnHomeEvent)
{
	StopInternal(/*bSuccess*/ true, bForceReturnHomeEvent);
}

void UStoneSettlerActionComponent::StopInternal(bool bSuccess, bool bForceReturnHomeEvent)
{
	if (!bActionRunning)
	{
		return;
	}

	const TObjectPtr<UStoneActionDefinitionData> FinishedDef = CurrentDef;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActionTickHandle);
	}
	
	// Optional: remove action-scoped tags we applied to the Settler ASC.
	if (AppliedStateTags.Num() > 0)
	{
		if (UAbilitySystemComponent* ASC = GetASC())
		{
			ASC->RemoveLooseGameplayTags(AppliedStateTags);

			UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Removed GrantedStateTags x%d from ASC. Owner=%s"),
				AppliedStateTags.Num(), *GetNameSafe(GetOwner()));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] Could not remove GrantedStateTags: ASC is null. Owner=%s"),
				*GetNameSafe(GetOwner()));
		}

		AppliedStateTags.Reset();
	}

	// Close encounter UI cleanly
	NotifyEncounterClosed(bLastEncounterAborted);

	bActionRunning = false;
	Phase = EStoneActionPhase::None;
	CurrentDef = nullptr;

	BaseDurationSeconds = 0.f;
	OutboundSeconds = 0.f;
	ArrivalSeconds = 0.f;
	ReturnSeconds = 0.f;
	PhaseElapsedBaseSeconds = 0.f;
	TotalElapsedBaseSeconds = 0.f;

	OutboundEncounterSlots.Reset();
	ReturnEncounterSlots.Reset();

	OnActionStateChanged.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Finished action: %s Owner=%s Success=%s"),
		*GetNameSafe(FinishedDef), *GetNameSafe(GetOwner()), bSuccess ? TEXT("true") : TEXT("false"));

	OnActionFinishedNative.Broadcast(FinishedDef.Get(), bSuccess);
	OnActionFinished.Broadcast(FinishedDef.Get(), bSuccess);
}

float UStoneSettlerActionComponent::ResolveActionSpeedMult() const
{
	// Currently no per-action speed multiplier in ActionDefinitionData (by design).
	// Keep hook for future expansion.
	return 1.f;
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
	case EStoneActionPhase::Outbound: PhaseDuration = OutboundSeconds; break;
	case EStoneActionPhase::Arrival:  PhaseDuration = ArrivalSeconds; break;
	case EStoneActionPhase::Return:   PhaseDuration = ReturnSeconds; break;
	default: return 0.f;
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
		return CurrentDef->Description;
	}
	return FText::GetEmpty();
}

FText UStoneSettlerActionComponent::GetPhaseText() const
{
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	switch (Phase)
	{
	case EStoneActionPhase::Outbound: return FText::FromString(TEXT("Outbound"));
	case EStoneActionPhase::Arrival:  return FText::FromString(TEXT("Arrival"));
	case EStoneActionPhase::Return:   return FText::FromString(TEXT("Return"));
	case EStoneActionPhase::Completed:return FText::FromString(TEXT("Completed"));
	default:                          return FText::FromString(TEXT("None"));
	}
}

float UStoneSettlerActionComponent::GetRemainingSeconds() const
{
	if (!bActionRunning)
	{
		return 0.f;
	}
	return FMath::Max(0.f, BaseDurationSeconds - TotalElapsedBaseSeconds);
}

void UStoneSettlerActionComponent::TickAction()
{
	if (!bActionRunning || !CurrentDef)
	{
		return;
	}

	// Local gate: while this settler has an open encounter, action time does not advance.
	if (bEncounterOpen)
	{
		return;
	}

	// Abort/ReturnImmediately outcomes (GAS tags) consumed here
	const EStoneActionAbortResult AbortResult = CheckAndConsumeAbortTags();
	if (AbortResult == EStoneActionAbortResult::Abort)
	{
		UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Action.Abort detected -> stopping action as failure. Owner=%s Def=%s"),
			*GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef));

		bLastEncounterAborted = true;
		NotifyEncounterClosed(true);
		StopAction(false);
		return;
	}
	if (AbortResult == EStoneActionAbortResult::ReturnImmediately)
	{
		UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Action.ReturnImmediately detected -> jumping to Return phase. Owner=%s Def=%s"),
			*GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef));

		if (Phase != EStoneActionPhase::Return && Phase != EStoneActionPhase::Completed)
		{
			EnterPhase(EStoneActionPhase::Return);
		}
	}

	if (Phase == EStoneActionPhase::Completed)
	{
		StopInternal(true, false);
		return;
	}

	// Advance base-time scaled by any future speed multiplier hook.
	const float AdvanceBaseSeconds = kActionTickInterval * ResolveActionSpeedMult();
	AdvancePhaseTimeline(AdvanceBaseSeconds);

	OnActionProgressChanged.Broadcast(GetActionProgress01());
}

void UStoneSettlerActionComponent::AdvancePhaseTimeline(float AdvanceBaseSeconds)
{
	if (!bActionRunning || Phase == EStoneActionPhase::None || Phase == EStoneActionPhase::Completed)
	{
		return;
	}

	PhaseElapsedBaseSeconds += AdvanceBaseSeconds;
	TotalElapsedBaseSeconds += AdvanceBaseSeconds;

	const float PhaseProgress01 = GetPhaseProgress01();

	if (Phase == EStoneActionPhase::Outbound)
	{
		for (FStonePlannedEncounter& E : OutboundEncounterSlots)
		{
			if (!E.bTriggered && PhaseProgress01 >= E.TriggerAtProgress01)
			{
				E.bTriggered = true;
				OpenEncounterByTag(E.EventTag);
				return;
			}
		}
	}
	else if (Phase == EStoneActionPhase::Return)
	{
		for (FStonePlannedEncounter& E : ReturnEncounterSlots)
		{
			if (!E.bTriggered && PhaseProgress01 >= E.TriggerAtProgress01)
			{
				E.bTriggered = true;
				OpenEncounterByTag(E.EventTag);
				return;
			}
		}
	}

	HandlePhaseAdvance();
}

void UStoneSettlerActionComponent::HandlePhaseAdvance()
{
	float PhaseSeconds = 0.f;
	switch (Phase)
	{
	case EStoneActionPhase::Outbound: PhaseSeconds = OutboundSeconds; break;
	case EStoneActionPhase::Arrival:  PhaseSeconds = ArrivalSeconds; break;
	case EStoneActionPhase::Return:   PhaseSeconds = ReturnSeconds; break;
	default: PhaseSeconds = 0.f; break;
	}

	if (PhaseSeconds <= 0.f)
	{
		if (Phase == EStoneActionPhase::Outbound)      EnterPhase(EStoneActionPhase::Arrival);
		else if (Phase == EStoneActionPhase::Arrival)  EnterPhase(EStoneActionPhase::Return);
		else if (Phase == EStoneActionPhase::Return)   EnterPhase(EStoneActionPhase::Completed);
		return;
	}

	if (PhaseElapsedBaseSeconds >= PhaseSeconds)
	{
		if (Phase == EStoneActionPhase::Outbound)      EnterPhase(EStoneActionPhase::Arrival);
		else if (Phase == EStoneActionPhase::Arrival)  EnterPhase(EStoneActionPhase::Return);
		else if (Phase == EStoneActionPhase::Return)   EnterPhase(EStoneActionPhase::Completed);
	}
}

void UStoneSettlerActionComponent::EnterPhase(EStoneActionPhase NewPhase)
{
	Phase = NewPhase;
	PhaseElapsedBaseSeconds = 0.f;

	AStoneSettlerChar* Settler = Cast<AStoneSettlerChar>(GetOwner());
	if (!Settler)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] EnterPhase: Owner is not AStoneSettlerChar. Owner=%s Phase=%d"),
			*GetNameSafe(GetOwner()), (int32)NewPhase);
	}

	// Arrival encounter (per-settler) – no RunSubsystem.
	if (Phase == EStoneActionPhase::Arrival)
	{
		// Minimal: use the ActionDef's ActionTag as the "context tag" for arrival events.
		// Your EventData assets should include ActionDef.ActionTag in EventTags.
		if (CurrentDef && CurrentDef->ActionTag.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Arrival -> opening encounter by ActionTag '%s'. Owner=%s Def=%s"),
				*CurrentDef->ActionTag.ToString(), *GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef));

			OpenEncounterByTag(CurrentDef->ActionTag);
		}
	}

	OnActionStateChanged.Broadcast();
}

FGameplayTag UStoneSettlerActionComponent::GetLegRandomEventTag(EStoneActionPhase InPhase) const
{
	if (!CurrentDef) return FGameplayTag();

	if (CurrentDef->ActionTag.IsValid())
	{
		return CurrentDef->ActionTag;
	}

	UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] GetLegRandomEventTag: Def has no ActionTag. No random encounters will trigger. Def=%s"),
		*GetNameSafe(CurrentDef));

	return FGameplayTag();
}

EStoneActionAbortResult UStoneSettlerActionComponent::CheckAndConsumeAbortTags()
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return EStoneActionAbortResult::None;
	}

	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	if (ASC->HasMatchingGameplayTag(T.Action_Abort))
	{
		ASC->RemoveLooseGameplayTag(T.Action_Abort);
		return EStoneActionAbortResult::Abort;
	}

	if (ASC->HasMatchingGameplayTag(T.Action_ReturnImmediately))
	{
		ASC->RemoveLooseGameplayTag(T.Action_ReturnImmediately);
		return EStoneActionAbortResult::ReturnImmediately;
	}

	return EStoneActionAbortResult::None;
}

void UStoneSettlerActionComponent::OpenEncounterByTag(FGameplayTag EventTag)
{
	if (!EventTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] OpenEncounterByTag failed: EventTag invalid. Owner=%s"), *GetNameSafe(GetOwner()));
		return;
	}

	if (bEncounterOpen)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[SettlerAction] OpenEncounterByTag ignored: encounter already open. Owner=%s"), *GetNameSafe(GetOwner()));
		return;
	}

	UAssetManager& AM = UAssetManager::Get();
	static const FPrimaryAssetType StoneEventType(TEXT("StoneEvent"));

	TArray<FPrimaryAssetId> Ids;
	AM.GetPrimaryAssetIdList(StoneEventType, Ids);

	if (Ids.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] No primary assets registered for type '%s'. Check AssetManager settings / PrimaryAssetId on UStoneEventData."),
			*StoneEventType.ToString());
		return;
	}

	TArray<UStoneEventData*> Matching;
	Matching.Reserve(8);

	FStreamableManager& SM = AM.GetStreamableManager();

	for (const FPrimaryAssetId& Id : Ids)
	{
		// Resolve the asset path for the primary asset and load synchronously (demo-safe).
		const FSoftObjectPath Path = AM.GetPrimaryAssetPath(Id);
		if (!Path.IsValid())
		{
			UE_LOG(LogTemp, Verbose, TEXT("[SettlerAction] PrimaryAssetPath invalid for %s"), *Id.ToString());
			continue;
		}

		UObject* Obj = SM.LoadSynchronous(Path, /*bManageActiveHandle*/ false);
		UStoneEventData* Event = Cast<UStoneEventData>(Obj);
		if (!Event)
		{
			continue;
		}

		if (Event->EventTags.HasTagExact(EventTag))
		{
			Matching.Add(Event);
		}
	}

	if (Matching.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] No UStoneEventData found for tag '%s'. Owner=%s"),
			*EventTag.ToString(), *GetNameSafe(GetOwner()));
		return;
	}

	const int32 PickIndex = RNG.RandRange(0, Matching.Num() - 1);
	UStoneEventData* Picked = Matching[PickIndex];

	// Guard: if nobody is listening, auto-resolve immediately so the action timeline is
	// not blocked forever. This can happen when the SettlerSlotDetails widget is closed
	// or has not yet bound to OnEncounterOpened for this settler.
	if (!OnEncounterOpened.IsBound())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[SettlerAction] Encounter '%s' would open for Tag='%s' but OnEncounterOpened has no listeners. "
			     "Auto-resolving to prevent action stall. Owner=%s"),
			*GetNameSafe(Picked), *EventTag.ToString(), *GetNameSafe(GetOwner()));
		// Do NOT set bEncounterOpen - there is nothing to resolve, just skip silently.
		return;
	}

	CurrentEncounterTag = EventTag;
	bLastEncounterAborted = false;
	bEncounterOpen = true;

	UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Encounter opened: Tag='%s' Picked='%s' Owner=%s"),
		*EventTag.ToString(), *GetNameSafe(Picked), *GetNameSafe(GetOwner()));

	OnEncounterOpened.Broadcast(Picked);
}

void UStoneSettlerActionComponent::NotifyEncounterClosed(bool bAborted)
{
	if (!bEncounterOpen)
	{
		return;
	}

	bEncounterOpen = false;
	CurrentEncounterTag = FGameplayTag();

	OnEncounterClosed.Broadcast(bAborted);
}

void UStoneSettlerActionComponent::ResolveCurrentEncounter(bool bAborted)
{
	if (!bEncounterOpen)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[SettlerAction] ResolveCurrentEncounter ignored: no encounter open. Owner=%s"), *GetNameSafe(GetOwner()));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Encounter resolved: Aborted=%s Owner=%s"),
		bAborted ? TEXT("true") : TEXT("false"),
		*GetNameSafe(GetOwner()));

	NotifyEncounterClosed(bAborted);
}

