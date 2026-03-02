// Copyright by MykeUhu

#include "Core/Components/StoneSettlerActionComponent.h"

// Project
#include "Core/StoneGameplayTags.h"
#include "Data/StoneEventData.h"
#include "Data/StoneActionDefinitionData.h"
#include "Game/Events/StoneEventResolver.h"

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
		UE_LOG(LogTemp, Error, TEXT("[SettlerAction] GetASC: Owner is null."));
		return nullptr;
	}

	const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OwnerActor);
	if (!ensureMsgf(ASI, TEXT("[SettlerAction] GetASC: Owner '%s' does not implement IAbilitySystemInterface."), *GetNameSafe(OwnerActor)))
	{
		return nullptr;
	}

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ensureMsgf(ASC, TEXT("[SettlerAction] GetASC: Owner '%s' returned null ASC."), *GetNameSafe(OwnerActor)))
	{
		return nullptr;
	}

	return ASC;
}

bool UStoneSettlerActionComponent::StartAction(UStoneActionDefinitionData* ActionDef)
{
	if (!ActionDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] StartAction failed: ActionDef is null. Owner=%s"),
			*GetNameSafe(GetOwner()));
		return false;
	}

	// Same action already running -> idempotent success.
	if (bActionRunning && CurrentDef == ActionDef)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[SettlerAction] StartAction ignored: already running. Owner=%s Def=%s"),
			*GetNameSafe(GetOwner()), *GetNameSafe(ActionDef));
		return false; // wichtig: verhindert “BT denkt success und läuft weiter”
	}

	// Different action running -> interrupt & restart.
	// If any action is running, stop it exactly once (clean state).
	if (bActionRunning)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[SettlerAction] StartAction: stopping previous action '%s' before starting '%s'. Owner=%s"),
			*GetNameSafe(CurrentDef), *GetNameSafe(ActionDef), *GetNameSafe(GetOwner()));

		StopInternal(false, /*bForceReturnHomeEvent*/ false);
	}

	// From here: clean start
	StopCurrentAction(false);

	CurrentDef = ActionDef;
	bActionRunning = true;
	bReturnHomeQueued = false;

	// Create/Init ActionRuntime (per-action brain)
	{
		UAbilitySystemComponent* ASC = GetASC();
		if (!ASC)
		{
			UE_LOG(LogTemp, Error, TEXT("[SettlerAction] StartAction failed: ASC is null. Owner=%s Def=%s"),
				*GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef));

			StopInternal(false, false);
			return false;
		}

		ActionRuntime = nullptr;

		ActionRuntime = NewObject<UStoneActionRuntime>(this);
		if (!ActionRuntime)
		{
			UE_LOG(LogTemp, Error, TEXT("[SettlerAction] StartAction failed: could not create ActionRuntime. Owner=%s"),
				*GetNameSafe(GetOwner()));

			StopInternal(false, false);
			return false;
		}

		CurrentSeed = RNG.RandRange(1, INT32_MAX);

		if (!ActionRuntime->Init(ASC, CurrentSeed))
		{
			UE_LOG(LogTemp, Error, TEXT("[SettlerAction] StartAction failed: ActionRuntime Init failed. Owner=%s Def=%s Seed=%d"),
				*GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef), CurrentSeed);

			ActionRuntime = nullptr;
			StopInternal(false, false);
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("[SettlerAction] ActionRuntime initialized. Owner=%s Def=%s Seed=%d"),
			*GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef), CurrentSeed);
	}

	// Apply action-scoped tags (optional)
	AppliedStateTags = CurrentDef->GrantedStateTags;
	if (AppliedStateTags.Num() > 0)
	{
		if (UAbilitySystemComponent* ASC = GetASC())
		{
			ASC->AddLooseGameplayTags(AppliedStateTags);
			UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Applied GrantedStateTags x%d to ASC. Owner=%s Def=%s"),
				AppliedStateTags.Num(), *GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef));
		}
	}

	BaseDurationSeconds = FMath::Max(1.f, CurrentDef->BaseDurationSeconds);

	OutboundSeconds = BaseDurationSeconds * CurrentDef->OutboundShare01;
	ReturnSeconds   = BaseDurationSeconds * CurrentDef->ReturnShare01;
	ArrivalSeconds  = FMath::Max(0.f, BaseDurationSeconds - OutboundSeconds - ReturnSeconds);

	Phase = EStoneActionPhase::Outbound;
	PhaseElapsedBaseSeconds = 0.f;
	TotalElapsedBaseSeconds = 0.f;

	OutboundEncounterSlots.Reset();
	ReturnEncounterSlots.Reset();

	bEncounterOpen = false;
	CurrentEncounterTag = FGameplayTag();
	bLastEncounterAborted = false;
	CurrentEncounterEvent = nullptr;

	// Pre-roll encounter slots
	for (int32 i = 0; i < CurrentDef->OutboundRandomCountMax; ++i)
	{
		if (RNG.FRand() <= CurrentDef->OutboundRandomChance01)
		{
			FStonePlannedEncounter Slot;
			Slot.EventTag = GetRandomActionTag(EStoneActionPhase::Outbound);
			Slot.TriggerAtProgress01 = RNG.FRandRange(CurrentDef->OutboundRandomAtMin01, CurrentDef->OutboundRandomAtMax01);
			Slot.bTriggered = false;
			OutboundEncounterSlots.Add(Slot);
		}
	}

	for (int32 i = 0; i < CurrentDef->ReturnRandomCountMax; ++i)
	{
		if (RNG.FRand() <= CurrentDef->ReturnRandomChance01)
		{
			FStonePlannedEncounter Slot;
			Slot.EventTag = GetRandomActionTag(EStoneActionPhase::Return);
			Slot.TriggerAtProgress01 = RNG.FRandRange(CurrentDef->ReturnRandomAtMin01, CurrentDef->ReturnRandomAtMax01);
			Slot.bTriggered = false;
			ReturnEncounterSlots.Add(Slot);
		}
	}

	OutboundEncounterSlots.Sort([](const FStonePlannedEncounter& A, const FStonePlannedEncounter& B) { return A.TriggerAtProgress01 < B.TriggerAtProgress01; });
	ReturnEncounterSlots.Sort([](const FStonePlannedEncounter& A, const FStonePlannedEncounter& B) { return A.TriggerAtProgress01 < B.TriggerAtProgress01; });

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ActionTickHandle, this, &UStoneSettlerActionComponent::TickAction, kActionTickInterval, true);
	}

	OnActionStateChanged.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Started action: %s Owner=%s Base=%.2fs Out=%.2fs Work=%.2fs Ret=%.2fs"),
		*CurrentDef->GetName(), *GetNameSafe(GetOwner()), BaseDurationSeconds, OutboundSeconds, ArrivalSeconds, ReturnSeconds);

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
	// If nothing is running, still sanitize transient pointers (SSOT cleanup).
	if (!bActionRunning)
	{
		ActionRuntime = nullptr;
		CurrentEncounterEvent = nullptr;
		bEncounterOpen = false;
		CurrentEncounterTag = FGameplayTag();
		AppliedStateTags.Reset();
		return;
	}

	const TObjectPtr<UStoneActionDefinitionData> FinishedDef = CurrentDef;

	// Stop ticking first
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActionTickHandle);
	}

	// Close encounter once, cleanly
	const bool bWasEncounterOpen = bEncounterOpen;
	if (bWasEncounterOpen)
	{
		NotifyEncounterClosed(bLastEncounterAborted);
	}

	// Destroy runtime-only state
	ActionRuntime = nullptr;
	CurrentEncounterEvent = nullptr;

	// Remove action-scoped tags we applied
	if (AppliedStateTags.Num() > 0)
	{
		if (UAbilitySystemComponent* ASC = GetASC())
		{
			ASC->RemoveLooseGameplayTags(AppliedStateTags);
		}
		AppliedStateTags.Reset();
	}

	// Reset action state
	bActionRunning = false;
	bReturnHomeQueued = false;

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

	bEncounterOpen = false;
	CurrentEncounterTag = FGameplayTag();
	bLastEncounterAborted = false;

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

	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	// Arrival encounter (per-settler).
	// OpenEncounterByTag internally builds RequiredTags = {Action.Phase.Arrival, CurrentDef->ActionTag}
	// so only events that have BOTH tags will match (e.g. EV_Forest_Arrival_* only fires for Forest actions).
	if (Phase == EStoneActionPhase::Arrival)
	{
		const FGameplayTag ArrivalPhaseTag = T.Action_Phase_Arrival;

		if (!ArrivalPhaseTag.IsValid())
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[SettlerAction] EnterPhase(Arrival): Action.Phase.Arrival is invalid. Check native tag registration."));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[SettlerAction] Arrival phase -> opening encounter. PhaseTag='%s' ActionTag='%s' Owner=%s Def=%s"),
				*ArrivalPhaseTag.ToString(),
				CurrentDef ? *CurrentDef->ActionTag.ToString() : TEXT("none"),
				*GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef));

			OpenEncounterByTag(ArrivalPhaseTag);
		}
	}

	OnActionStateChanged.Broadcast();
}

FGameplayTag UStoneSettlerActionComponent::GetRandomActionTag(EStoneActionPhase InPhase) const
{
	// NOTE: despite the name, this returns the PHASE TAG for a given phase.
	// The "random" part is that during pre-roll, each slot picks its own phase tag.
	// The actual event selection (randomness) happens in OpenEncounterByTag using HasAll(ActionTag + PhaseTag).
	// FStonePlannedEncounter::EventTag is therefore always a Phase tag, never a concrete event tag.
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	FGameplayTag Result;

	switch (InPhase)
	{
	case EStoneActionPhase::Outbound: Result = T.Action_Phase_Outbound; break;
	case EStoneActionPhase::Arrival:  Result = T.Action_Phase_Arrival;  break;
	case EStoneActionPhase::Return:   Result = T.Action_Phase_Return;   break;
	default:                          Result = FGameplayTag();           break;
	}

	if (!Result.IsValid() && (InPhase == EStoneActionPhase::Outbound || InPhase == EStoneActionPhase::Arrival || InPhase == EStoneActionPhase::Return))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[SettlerAction] GetRandomActionTag: PhaseTag invalid for Phase=%d. Check StoneGameplayTags::Action.Phase.* registration. Owner=%s Def=%s"),
			(int32)InPhase, *GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef));
	}

	return Result;
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

void UStoneSettlerActionComponent::OpenEncounterByTag(FGameplayTag PhaseTag)
{
	if (!PhaseTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] OpenEncounterByTag failed: PhaseTag invalid. Owner=%s"), *GetNameSafe(GetOwner()));
		return;
	}

	if (bEncounterOpen)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[SettlerAction] OpenEncounterByTag ignored: encounter already open. Owner=%s"), *GetNameSafe(GetOwner()));
		return;
	}

	if (!ActionRuntime)
	{
		UE_LOG(LogTemp, Error, TEXT("[SettlerAction] OpenEncounterByTag failed: ActionRuntime is null. Owner=%s Def=%s"),
			*GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef));
		return;
	}

	// Build required tags: PhaseTag + ActionTag
	FGameplayTagContainer RequiredTags;
	RequiredTags.AddTag(PhaseTag);

	if (CurrentDef && CurrentDef->ActionTag.IsValid())
	{
		RequiredTags.AddTag(CurrentDef->ActionTag);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[SettlerAction] OpenEncounterByTag: CurrentDef has no ActionTag - matching by PhaseTag only ('%s'). ")
			TEXT("Encounters from ALL action types will be eligible. Owner=%s Def=%s"),
			*PhaseTag.ToString(), *GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef));
	}

	UStoneEventData* Picked = ActionRuntime->PickEventByRequiredTags(RequiredTags);
	if (!Picked)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[SettlerAction] No UStoneEventData found matching RequiredTags='%s'. ")
			TEXT("Check EventData assets have BOTH ActionTag AND PhaseTag in EventTags. Owner=%s Def=%s"),
			*RequiredTags.ToStringSimple(), *GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef));
		return;
	}

	// Guard: if nobody is listening, auto-resolve immediately so the action timeline is not blocked forever.
	if (!OnEncounterOpened.IsBound())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[SettlerAction] Encounter '%s' would open for RequiredTags='%s' but OnEncounterOpened has no listeners. ")
			TEXT("Auto-resolving to prevent action stall. Owner=%s"),
			*GetNameSafe(Picked), *RequiredTags.ToStringSimple(), *GetNameSafe(GetOwner()));
		return;
	}

	CurrentEncounterTag = PhaseTag;
	bLastEncounterAborted = false;
	bEncounterOpen = true;

	UE_LOG(LogTemp, Log,
		TEXT("[SettlerAction] Encounter opened: RequiredTags='%s' Picked='%s' Owner=%s"),
		*RequiredTags.ToStringSimple(), *GetNameSafe(Picked), *GetNameSafe(GetOwner()));

	CurrentEncounterEvent = Picked;
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

void UStoneSettlerActionComponent::GetCurrentEncounterChoices(TArray<FStoneChoiceResolved>& OutResolved) const
{
	OutResolved.Reset();

	if (!bEncounterOpen)
	{
		return;
	}

	if (!ActionRuntime)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] GetCurrentEncounterChoices: ActionRuntime null. Owner=%s"), *GetNameSafe(GetOwner()));
		return;
	}

	// Du hast das EventData Objekt beim OpenEncounterByTag als Picked.
	// -> Speichere es als CurrentEncounterEvent (TObjectPtr<UStoneEventData>)
	if (!CurrentEncounterEvent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] GetCurrentEncounterChoices: CurrentEncounterEvent null. Owner=%s"), *GetNameSafe(GetOwner()));
		return;
	}

	ActionRuntime->GetResolvedChoices(CurrentEncounterEvent, OutResolved);
}
bool UStoneSettlerActionComponent::ApplyEncounterChoice(int32 ChoiceIndex)
{
	if (!bEncounterOpen)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[SettlerAction] ApplyEncounterChoice ignored: no encounter open. Owner=%s"), *GetNameSafe(GetOwner()));
		return false;
	}

	if (!ActionRuntime || !CurrentEncounterEvent)
	{
		UE_LOG(LogTemp, Error, TEXT("[SettlerAction] ApplyEncounterChoice failed: Runtime/Event missing. Owner=%s"), *GetNameSafe(GetOwner()));
		return false;
	}

	const bool bApplied = ActionRuntime->ApplyChoice(CurrentEncounterEvent, ChoiceIndex);
	if (!bApplied)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerAction] ApplyEncounterChoice: choice rejected. Owner=%s Choice=%d"),
			*GetNameSafe(GetOwner()), ChoiceIndex);
		return false;
	}

	// Close encounter (and allow timeline to continue)
	NotifyEncounterClosed(/*bAborted*/ false);

	// Clear the current event pointer
	CurrentEncounterEvent = nullptr;

	OnActionStateChanged.Broadcast();
	return true;
}

void UStoneSettlerActionComponent::AbortAndResetForLoad()
{
	// Always stop timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActionTickHandle);
	}

	// If encounter is open, close it once (aborted)
	if (bEncounterOpen)
	{
		bLastEncounterAborted = true;
		NotifyEncounterClosed(true);
	}

	// Remove any tags we applied (even if action flag got desynced)
	if (AppliedStateTags.Num() > 0)
	{
		if (UAbilitySystemComponent* ASC = GetASC())
		{
			ASC->RemoveLooseGameplayTags(AppliedStateTags);
		}
		AppliedStateTags.Reset();
	}

	// Hard reset all runtime-only state
	ActionRuntime = nullptr;
	CurrentEncounterEvent = nullptr;

	bActionRunning = false;
	bReturnHomeQueued = false;

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

	bEncounterOpen = false;
	CurrentEncounterTag = FGameplayTag();
	bLastEncounterAborted = false;

	OnActionStateChanged.Broadcast();
}

void UStoneSettlerActionComponent::BuildSavedActionState(FSavedSettlerActionState& OutState) const
{
	OutState = FSavedSettlerActionState();

	// Snapshot only when running
	if (!bActionRunning || !CurrentDef)
	{
		return;
	}

	OutState.ActionDef = CurrentDef;
	OutState.Seed = CurrentSeed;
	OutState.Phase = Phase;
	OutState.PhaseElapsed = PhaseElapsedBaseSeconds;
	OutState.TotalElapsed = TotalElapsedBaseSeconds;

	OutState.bEncounterOpen = bEncounterOpen;
	OutState.CurrentEncounterEvent = CurrentEncounterEvent;

	// Pre-rolled slots (already USTRUCT/SafeGame-ready)
	OutState.OutboundSlots = OutboundEncounterSlots;
	OutState.ReturnSlots = ReturnEncounterSlots;
}

bool UStoneSettlerActionComponent::RestoreFromSavedActionState(const FSavedSettlerActionState& InState)
{
	// Always clean any previous runtime/timer state
	if (bActionRunning)
	{
		StopInternal(false, false);
	}

	if (InState.ActionDef.IsNull())
	{
		return false;
	}

	UStoneActionDefinitionData* LoadedDef = InState.ActionDef.LoadSynchronous();
	if (!LoadedDef)
	{
		// No resume data
		return false;
	}

	// Re-init SSOT fields
	CurrentDef = LoadedDef;
	bActionRunning = true;
	bReturnHomeQueued = false;

	CurrentSeed = (InState.Seed > 0) ? InState.Seed : 12345;
	RNG.Initialize(CurrentSeed);

	BaseDurationSeconds = FMath::Max(1.f, CurrentDef->BaseDurationSeconds);
	OutboundSeconds = BaseDurationSeconds * CurrentDef->OutboundShare01;
	ReturnSeconds   = BaseDurationSeconds * CurrentDef->ReturnShare01;
	ArrivalSeconds  = FMath::Max(0.f, BaseDurationSeconds - OutboundSeconds - ReturnSeconds);

	Phase = InState.Phase;
	PhaseElapsedBaseSeconds = InState.PhaseElapsed;
	TotalElapsedBaseSeconds = InState.TotalElapsed;

	// Runtime
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[SettlerAction] RestoreFromSavedActionState failed: ASC is null. Owner=%s"), *GetNameSafe(GetOwner()));
		AbortAndResetForLoad();
		return false;
	}

	ActionRuntime = NewObject<UStoneActionRuntime>(this);
	if (!ActionRuntime)
	{
		UE_LOG(LogTemp, Error, TEXT("[SettlerAction] RestoreFromSavedActionState failed: could not create ActionRuntime. Owner=%s"), *GetNameSafe(GetOwner()));
		AbortAndResetForLoad();
		return false;
	}

	if (!ActionRuntime->Init(ASC, CurrentSeed))
	{
		UE_LOG(LogTemp, Error, TEXT("[SettlerAction] RestoreFromSavedActionState failed: ActionRuntime Init failed. Owner=%s Def=%s Seed=%d"),
			*GetNameSafe(GetOwner()), *GetNameSafe(CurrentDef), CurrentSeed);
		AbortAndResetForLoad();
		return false;
	}

	// Re-apply action-scoped tags (GrantedStateTags) exactly like StartAction.
	AppliedStateTags = CurrentDef->GrantedStateTags;
	if (AppliedStateTags.Num() > 0)
	{
		ASC->AddLooseGameplayTags(AppliedStateTags);
	}

	// Restore planned encounters
	OutboundEncounterSlots = InState.OutboundSlots;
	OutboundEncounterSlots.Sort([](const FStonePlannedEncounter& A, const FStonePlannedEncounter& B)
	{
		return A.TriggerAtProgress01 < B.TriggerAtProgress01;
	});

	ReturnEncounterSlots = InState.ReturnSlots;
	ReturnEncounterSlots.Sort([](const FStonePlannedEncounter& A, const FStonePlannedEncounter& B)
	{
		return A.TriggerAtProgress01 < B.TriggerAtProgress01;
	});

	// Restore encounter open
	bEncounterOpen = InState.bEncounterOpen;
	CurrentEncounterEvent = nullptr;
	if (bEncounterOpen && !InState.CurrentEncounterEvent.IsNull())
	{
		CurrentEncounterEvent = InState.CurrentEncounterEvent.LoadSynchronous();
	}

	// Restart tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ActionTickHandle, this, &UStoneSettlerActionComponent::TickAction, kActionTickInterval, true);
	}

	OnActionStateChanged.Broadcast();

	// If an encounter was open at save time, re-open UI deterministically
	if (bEncounterOpen && CurrentEncounterEvent)
	{
		OnEncounterOpened.Broadcast(CurrentEncounterEvent);
	}

	return true;
}


