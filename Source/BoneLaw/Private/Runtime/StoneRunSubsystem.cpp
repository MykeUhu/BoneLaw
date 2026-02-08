#include "Runtime/StoneRunSubsystem.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "Core/StoneGameplayTags.h"
#include "Core/StonePlayerState.h"
#include "Data/StoneEventData.h"
#include "Data/StoneEventPackData.h"
#include "Game/StoneOutcomeExecutor.h"
#include "Game/StoneScheduler.h"
#include "Kismet/GameplayStatics.h"
#include "Game/StoneWorldlineDirector.h"
#include "Game/StoneWorldlineWeightPolicy.h"
#include "Game/Events/StoneEventResolver.h"
#include "Library/StoneEventLibrary.h"
#include "Library/StonePackLibrary.h"

#include "Runtime/StoneActionSubsystem.h"

#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	static void StopWorldActionSubsystemIfRunning(UWorld* World, const TCHAR* Caller)
	{
		if (!World)
		{
			return;
		}

		UStoneActionSubsystem* ActionSS = World->GetSubsystem<UStoneActionSubsystem>();
		if (!ActionSS)
		{
			return;
		}

		if (ActionSS->IsActionRunning())
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneRunSubsystem] %s: stopping StoneActionSubsystem action (demo rule: no stacked actions)."), Caller);
			ActionSS->StopCurrentAction(/*bForceReturnHomeEvent*/ false);
		}
	}
}

void UStoneRunSubsystem::SetSimulationSpeed(float NewSpeed)
{
	SimulationSpeed = FMath::Clamp(NewSpeed, 0.f, 10.f);
}

bool UStoneRunSubsystem::IsOnExpedition() const
{
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();
	return bExpeditionActive || RunTags.HasTag(T.State_OnExpedition);
}

float UStoneRunSubsystem::GetExpeditionProgress01() const
{
	if (!bExpeditionActive || ExpeditionDurationSeconds <= 0.f)
	{
		return 0.f;
	}
	return FMath::Clamp(ExpeditionElapsedSeconds / ExpeditionDurationSeconds, 0.f, 1.f);
}

void UStoneRunSubsystem::SetOnExpeditionTag(bool bOn)
{
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();
	if (bOn)
	{
		RunTags.AddTag(T.State_OnExpedition);
		// Travelling means: not in cave.
		RunTags.RemoveTag(T.State_InCave);
	}
	else
	{
		RunTags.RemoveTag(T.State_OnExpedition);
	}
}

void UStoneRunSubsystem::StartExpeditionTick()
{
	// Backwards-compatible wrapper.
	StartRealtimeActionTick();
}


void UStoneRunSubsystem::StopExpeditionTick()
{
	// Backwards-compatible wrapper.
	StopRealtimeActionTick();
}


void UStoneRunSubsystem::ReturnToRealtimeTravelState()
{
	// Between real-time events, we want the event UI to disappear.
	CurrentEvent = nullptr;
	OnEventChanged.Broadcast(nullptr);
}

void UStoneRunSubsystem::QueueNextRealtimeEvent(bool bAllowImmediate)
{
	if (!bExpeditionActive)
	{
		return;
	}

	// Hard floor to prevent "machine gun" even if BP passes 0/0.
	// Timer ticks at 0.25s (4 Hz). We want pacing that feels like a game, not a clicker.
	constexpr float MinGapFloorSeconds = 5.0f;

	const float MinGap = FMath::Max(MinGapFloorSeconds, RealtimeMinEventGapSeconds);
	const float MaxGap = FMath::Max(MinGap, RealtimeMaxEventGapSeconds);

	RealtimeNextEventCountdown = bAllowImmediate ? 0.f : RNG.FRandRange(MinGap, MaxGap);
}


bool UStoneRunSubsystem::TryPickReturnEventId(FName& OutEventId)
{
	OutEventId = NAME_None;
	if (!Resolver || EventPoolIds.Num() == 0)
	{
		return false;
	}

	const FStoneGameplayTags& T = FStoneGameplayTags::Get();
	UAbilitySystemComponent* ASC = GetASC();

	FStoneSnapshot TempSnap = Snapshot;
	TempSnap.FocusTag = FocusTag;
	TempSnap.Time = Time;
	TempSnap.RunTags = RunTags;

	struct FCandidate { FName Id; int32 W; };
	TArray<FCandidate> Candidates;
	int32 TotalWeight = 0;

	for (const FName& Id : EventPoolIds)
	{
		UStoneEventData* Ev = LoadEventById(Id);
		if (!Ev) continue;

		if (!Ev->EventTags.HasTag(T.Event_ExploreReturn))
		{
			continue;
		}

		if (!Resolver->EvaluateRequirement(Ev->Requirement, ASC, RunTags))
		{
			continue;
		}

		const int32 W = Resolver->ComputeFinalWeight(Ev, TempSnap);
		if (W <= 0) continue;

		Candidates.Add({ Id, W });
		TotalWeight += W;
	}

	if (Candidates.Num() == 0 || TotalWeight <= 0)
	{
		return false;
	}

	const int32 Roll = RNG.RandRange(1, TotalWeight);
	int32 Acc = 0;
	for (const auto& C : Candidates)
	{
		Acc += C.W;
		if (Roll <= Acc)
		{
			OutEventId = C.Id;
			return true;
		}
	}

	OutEventId = Candidates.Last().Id;
	return true;
}

void UStoneRunSubsystem::ForceReturnEvent()
{
	if (bExpeditionReturnQueued)
	{
		return;
	}
	
	FName ReturnId;
	if (TryPickReturnEventId(ReturnId) && !ReturnId.IsNone())
	{
		RealtimeForcedQueue.Add(ReturnId);
	}
	
	bExpeditionReturnQueued = true;
}

void UStoneRunSubsystem::StartExploreExpedition(FName ExplorePackId, float DurationSeconds, float MinEventGapSeconds, float MaxEventGapSeconds, bool bTriggerFirstEventImmediately)
{
	if (ExplorePackId.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRunSubsystem] StartExploreExpedition: ExplorePackId is None"));
		return;
	}

	// Demo rule: do not stack actions from different systems.
	StopWorldActionSubsystemIfRunning(GetWorld(), TEXT("StartExploreExpedition"));

// Demo rule: do not stack real-time actions.
if (bTravelActive)
{
	StopTravelAction(/*bForceReturnHomeEvent*/ false);
}

	// Ensure core systems exist.
	if (!EnsurePlayerStateCache())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRunSubsystem] StartExploreExpedition failed: No PlayerState."));
		return;
	}
	EnsureEventLibrary(true);
	EnsurePackLibrary(true);

	// Activate pack as TEMPORARY: it should be active only while the action is active.
	ActivatePackTemporary(ExplorePackId);

	// Reset expedition state.
	RealtimeForcedQueue.Reset();
	bExpeditionReturnQueued = false;
	bExpeditionActive = true;
	ExpeditionPackId = ExplorePackId;

	ExpeditionDurationSeconds = FMath::Max(1.f, DurationSeconds);
	ExpeditionElapsedSeconds = 0.f;

	// Clamp to avoid machine-gun events even if UI passes 0/0.
	constexpr float MinGapFloorSeconds = 5.0f;
	RealtimeMinEventGapSeconds = FMath::Max(MinGapFloorSeconds, MinEventGapSeconds);
	RealtimeMaxEventGapSeconds = FMath::Max(RealtimeMinEventGapSeconds, MaxEventGapSeconds);
	RealtimeNextEventCountdown = 0.f;

	const FStoneGameplayTags& T = FStoneGameplayTags::Get();
	RunTags.AddTag(T.State_OnAction);
	SetOnExpeditionTag(true);

	// Hide any currently shown event and start travelling.
	ReturnToRealtimeTravelState();
	QueueNextRealtimeEvent(/*bAllowImmediate*/ bTriggerFirstEventImmediately);
	StartRealtimeActionTick();

	RebuildSnapshot();
	BroadcastSnapshot();
}


void UStoneRunSubsystem::StopExpedition(bool bForceReturnEvent)
{
	if (!bExpeditionActive)
	{
		return;
	}

	if (bForceReturnEvent)
	{
		ForceReturnEvent();
		// If no event is shown right now, present one immediately.
		if (!CurrentEvent)
		{
			PickNextEvent(/*scheduled*/true, /*random*/true, /*force*/true);
		}
	}

	bExpeditionActive = false;
	ExpeditionPackId = NAME_None;
	ExpeditionDurationSeconds = 0.f;
	ExpeditionElapsedSeconds = 0.f;
	RealtimeNextEventCountdown = 0.f;
	bExpeditionReturnQueued = false;
	RealtimeForcedQueue.Reset();

	const FStoneGameplayTags& T = FStoneGameplayTags::Get();
	RunTags.RemoveTag(T.State_OnExpedition);
	RunTags.RemoveTag(T.State_OnAction);

	StopRealtimeActionTick();

	// Remove any packs that were activated only for this action.
	DeactivateTemporaryPacks();

	RebuildSnapshot();
	BroadcastSnapshot();
}


void UStoneRunSubsystem::TickExpedition()
{
	// Backwards-compatible wrapper (timer may still be bound to this in older content).
	TickRealtimeActions();
}



void UStoneRunSubsystem::SetAttrByStableName(const FName& StableName, float Value) const
{
	if (StableName.IsNone())
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetASC())
	{
		// We accept both leaf names ("Health") and fully qualified tag names ("Attributes.Vital.Health").
		TMap<FName, float> Temp;
		Temp.Add(StableName, Value);
		// Outdated
		// UStoneAbilitySystemLibrary::ApplyAttributeMapToASC(ASC, Temp);
	}
}

void UStoneRunSubsystem::GetResolvedChoices(TArray<FStoneChoiceResolved>& OutResolved) const
{
	OutResolved.Reset();
	if (!CurrentEvent || !Resolver) return;
	Resolver->ResolveChoices(CurrentEvent, GetASC(), RunTags, OutResolved);
}

void UStoneRunSubsystem::Deinitialize()
{
	StopRealtimeActionTick();

	bExpeditionActive = false;
	bExpeditionReturnQueued = false;
	RealtimeForcedQueue.Reset();

	bTravelActive = false;
	TravelPhase = EStoneTravelPhase::None;

	TemporaryPackIds.Reset();
	PendingEventIds.Reset();

	// Clear the cached PlayerState reference (PlayerState lifetime is managed by UE, not us)
	CachedPlayerState.Reset();
	
	Super::Deinitialize();
}


void UStoneRunSubsystem::TraceAdd(EStoneTraceType Type, const FString& Details, FName EventId, int32 ChoiceIndex)
{
	if (!Trace) return;

	FStoneTraceEntry E;
	E.Type = Type;
	E.Day = Time.DayIndex;
	E.bNight = Time.bIsNight;
	E.TotalChoices = Time.TotalChoices;
	E.EventId = EventId;
	E.ChoiceIndex = ChoiceIndex;
	E.Details = Details;
	E.RealTimeUTC = FDateTime::UtcNow();

	Trace->Add(E);
}

void UStoneRunSubsystem::EnsureEventLibrary(bool bPreloadAllSync)
{
	if (EventLibrary) return;

	EventLibrary = NewObject<UStoneEventLibrary>(this);
	EventLibrary->Initialize();

	// Studio default: preload all sync at boot for “no hitch” UI-driven game
	// (If content grows huge later, switch to PreloadByIds(EventPoolIds, true) + staged loads.)
	EventLibrary->PreloadAll(bPreloadAllSync);
}

UStoneEventData* UStoneRunSubsystem::GetEventFast(FName EventId) const
{
	return EventLibrary ? EventLibrary->GetEvent(EventId) : nullptr;
}

bool UStoneRunSubsystem::EnsurePlayerStateCache()
{
	// Already cached and valid?
	if (CachedPlayerState.IsValid())
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Get local player controller
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneRunSubsystem] EnsurePlayerStateCache: No PlayerController found."));
		return false;
	}

	AStonePlayerState* PS = PC->GetPlayerState<AStonePlayerState>();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneRunSubsystem] EnsurePlayerStateCache: PlayerState is not AStonePlayerState."));
		return false;
	}

	CachedPlayerState = PS;
	return true;
}

AStonePlayerState* UStoneRunSubsystem::GetPlayerState() const
{
	return CachedPlayerState.Get();
}

UAbilitySystemComponent* UStoneRunSubsystem::GetASC() const
{
	if (CachedPlayerState.IsValid())
	{
		return CachedPlayerState->GetAbilitySystemComponent();
	}
	return nullptr;
}

void UStoneRunSubsystem::EnsureWorldlineDirector()
{
	if (Worldline) return;

	EnsurePlayerStateCache();

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRunSubsystem] EnsureWorldlineDirector failed (ASC missing)."));
		return;
	}

	Worldline = NewObject<UStoneWorldlineDirector>(this);
	Worldline->Initialize(ASC);
}

void UStoneRunSubsystem::UpdateWorldlineAndUnlocks()
{
	EnsureWorldlineDirector();

	TArray<FStoneScheduledEvent> NewSchedules;
	Worldline->UpdateWorldline(RunTags, Time, NewSchedules);

	// Enqueue any worldline forced/high events
	if (Scheduler)
	{
		for (const auto& Ev : NewSchedules)
		{
			Scheduler->Enqueue(Ev, Time);
			TraceAdd(EStoneTraceType::ScheduledEnqueued,
				FString::Printf(TEXT("Worldline scheduled %s (prio=%d)"), *Ev.EventId.ToString(), (int32)Ev.Priority),
				Ev.EventId, INDEX_NONE);
		}
	}

	// Now packs can auto-unlock by required tags
	TryAutoUnlockPacks();
}

void UStoneRunSubsystem::EnsureWeightPolicy()
{
	if (WeightPolicy) return;

	EnsureWorldlineDirector();
	WeightPolicy = NewObject<UStoneWorldlineWeightPolicy>(this);
	WeightPolicy->Initialize(Worldline);
}

float UStoneRunSubsystem::ComputeCrisisMultiplier(const FStoneSnapshot& Snap, const UStoneEventData* Event) const
{
	if (!Event) return 1.f;

	const FStoneGameplayTags& T = FStoneGameplayTags::Get();
	float Mul = 1.f;

	// Hunger crisis -> hunt/forage events rise
	if (Snap.Food < 25.f)
	{
		if (Event->EventTags.HasTag(T.Event_Hunt) || Event->EventTags.HasTag(T.Event_Forage))
		{
			Mul *= 1.4f;
		}
	}

	// Warmth crisis -> shelter/fire events rise (especially at night)
	if (Snap.Warmth < 25.f)
	{
		if (Event->EventTags.HasTag(T.Event_Shelter) || Event->EventTags.HasTag(T.Event_Fire))
		{
			Mul *= Snap.Time.bIsNight ? 1.6f : 1.25f;
		}
	}

	// Health crisis -> illness/injury events rise (system pressure)
	if (Snap.Health < 35.f)
	{
		if (Event->EventTags.HasTag(T.Event_Illness) || Event->EventTags.HasTag(T.Event_Injury))
		{
			Mul *= 1.35f;
		}
	}

	return Mul;
}

UStoneEventData* UStoneRunSubsystem::LoadEventById(FName EventId) const
{
	if (EventId.IsNone())
	{
		return nullptr;
	}

	// For demo: synchronous safety net. In the steady state we preload all at boot.
	if (!EventLibrary)
	{
		const_cast<UStoneRunSubsystem*>(this)->EnsureEventLibrary(true);
	}

	if (!EventLibrary)
	{
		return nullptr;
	}

	UStoneEventData* Ev = EventLibrary->GetEvent(EventId);
	if (!Ev)
	{
		TArray<FName> One;
		One.Add(EventId);
		EventLibrary->PreloadByIds(One, true);
		Ev = EventLibrary->GetEvent(EventId);
	}
	return Ev;
}

void UStoneRunSubsystem::EnsurePackLibrary(bool bPreloadAllSync)
{
	if (PackLibrary) return;

	PackLibrary = NewObject<UStonePackLibrary>(this);
	PackLibrary->Initialize();
	PackLibrary->PreloadAll(bPreloadAllSync);

	KnownPackIds.Reset();
	PackLibrary->GetAllKnownPackIds(KnownPackIds);
}

void UStoneRunSubsystem::TryAutoUnlockPacks()
{
	if (!bAutoPackUnlocksEnabled)
	{
		return;
	}

	EnsurePackLibrary(true);
	EnsureEventLibrary(true);

	if (!PackLibrary)
	{
		return;
	}

	TArray<FName> PreloadEvents;

	for (const FName& PackId : KnownPackIds)
	{
		if (PackId.IsNone() || ActivePackIds.Contains(PackId))
		{
			continue;
		}

		UStoneEventPackData* Pack = PackLibrary->GetPack(PackId);
		if (!Pack || !Pack->bAutoUnlockWhenRequirementsMet)
		{
			continue;
		}

		if (!RunTags.HasAll(Pack->RequiredTagsAll))
		{
			continue;
		}
		if (!Pack->BlockedTagsAny.IsEmpty() && RunTags.HasAny(Pack->BlockedTagsAny))
		{
			continue;
		}

		ActivePackIds.Add(PackId);

		TraceAdd(EStoneTraceType::PackUnlocked,
			FString::Printf(TEXT("Unlocked pack %s"), *PackId.ToString()),
			NAME_None, INDEX_NONE);

		for (const FStonePackEntry& Entry : Pack->Events)
		{
			if (!Entry.EventId.IsNone())
			{
				EventPoolIds.AddUnique(Entry.EventId);
				if (Pack->bPreloadOnUnlock)
				{
					PreloadEvents.AddUnique(Entry.EventId);
				}
			}
		}
	}

	if (PreloadEvents.Num() > 0 && EventLibrary)
	{
		EventLibrary->PreloadByIds(PreloadEvents, true);
	}
}

void UStoneRunSubsystem::StartNewRun(const FStoneRunConfig& Config)
{
	UE_LOG(LogTemp, Log, TEXT("[StoneRunSubsystem] StartNewRun called"));

	if (!EnsurePlayerStateCache())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRunSubsystem] StartNewRun failed: Could not find PlayerState with GAS."));
		return;
	}

	Scheduler = NewObject<UStoneScheduler>(this);
	Resolver = NewObject<UStoneEventResolver>(this);
	OutcomeExecutor = NewObject<UStoneOutcomeExecutor>(this);

	Time = FStoneTimeState{};
	RunTags = Config.StartingTags;
	FocusTag = FGameplayTag();
	bAutoPackUnlocksEnabled = Config.bEnableAutoPackUnlocks;

	RNG.Initialize(Config.RNGSeed);

	Trace = NewObject<UStoneRunTraceBuffer>(this);
	Trace->Init(800);
	Trace->Clear();

	UAbilitySystemComponent* ASC = GetASC();
	check(ASC);

	// Initial day tags
	ApplyDayNightTags(false);

	// Apply starting attributes (Aura pattern: StableName -> Value) if provided.
	if (Config.StartingAttributeValues.Num() > 0)
	{
	}

	// Scheduler reset
	Scheduler->Reset(Time);

	// Build initial pool from Packs/Settings (SSOT, no hardcoded EventIds)
	BuildInitialEventPool(Config);

	// Force initial pick even if we are "idle" tags-wise
	PickNextEvent(/*scheduled*/true, /*random*/true, /*force*/true);

	RebuildSnapshot();
	BroadcastSnapshot();
}

void UStoneRunSubsystem::SetFocus(FGameplayTag InFocusTag)
{
	FocusTag = InFocusTag;
	RebuildSnapshot();
	BroadcastSnapshot();
}

bool UStoneRunSubsystem::TryConsumeScheduledForcedEvent(FName& OutEventId)
{
	if (!Scheduler) return false;

	FStoneScheduledEvent Due;
	if (Scheduler->PopNextDue(Due, Time) && Due.Priority == EStoneEventPriority::Forced)
	{
		OutEventId = Due.EventId;
		return true;
	}
	return false;
}

bool UStoneRunSubsystem::ShouldIdleBetweenEvents() const
{
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	// In cave => idle (unless expedition is active)
	if (RunTags.HasTag(T.State_InCave) && !RunTags.HasTag(T.State_OnExpedition))
	{
		return true;
	}
	return false;
}

void UStoneRunSubsystem::PickNextEvent(bool bAllowScheduledOverride, bool bAllowRandomFromPool, bool bForceRandomEvenIfIdle)
{
	// 0) if realtime queue has something ready, show it first
	if (RealtimeForcedQueue.Num() > 0)
	{
		const FName NextId = RealtimeForcedQueue[0];
		RealtimeForcedQueue.RemoveAt(0);

		CurrentEvent = LoadEventById(NextId);
		OnEventChanged.Broadcast(CurrentEvent);
		return;
	}

	// 1) forced scheduled (choice/day-night scheduler)
	if (bAllowScheduledOverride)
	{
		FName ForcedId;
		if (TryConsumeScheduledForcedEvent(ForcedId))
		{
			CurrentEvent = LoadEventById(ForcedId);
			OnEventChanged.Broadcast(CurrentEvent);
			return;
		}
	}

	// 1.5) Pending queued events (from actions/travel/ambient) must be shown before random picks.
	// This ensures Arrival events (queued while Outbound was open) are presented deterministically.
	if (!CurrentEvent && PendingEventIds.Num() > 0)
	{
		const FName NextId = PendingEventIds[0];
		PendingEventIds.RemoveAt(0);

		CurrentEvent = LoadEventById(NextId);
		OnEventChanged.Broadcast(CurrentEvent);

		UE_LOG(LogTemp, Log, TEXT("[StoneRunSubsystem] PickNextEvent: opened pending event '%s' (%d remaining)"),
			*NextId.ToString(), PendingEventIds.Num());
		return;
	}

	// 2) idle mode (cave) -> do NOT pick random
	if (!bForceRandomEvenIfIdle && ShouldIdleBetweenEvents())
	{
		CurrentEvent = nullptr;
		OnEventChanged.Broadcast(nullptr);
		return;
	}

	// 3) if random-from-pool not allowed -> idle
	if (!bAllowRandomFromPool)
	{
		CurrentEvent = nullptr;
		OnEventChanged.Broadcast(nullptr);
		return;
	}

	// 4) weighted random from pool (EXACTLY your old logic, moved here)
	UAbilitySystemComponent* ASC = GetASC();
	FStoneSnapshot TempSnap = Snapshot;
	TempSnap.FocusTag = FocusTag;
	TempSnap.Time = Time;
	TempSnap.RunTags = RunTags;

	int32 TotalWeight = 0;

	struct FCandidate { FName Id; int32 W; };
	TArray<FCandidate> Candidates;
	Candidates.Reserve(EventPoolIds.Num());

	for (const FName& Id : EventPoolIds)
	{
		UStoneEventData* Ev = LoadEventById(Id);
		if (!Ev) continue;

		if (!Resolver->EvaluateRequirement(Ev->Requirement, ASC, RunTags))
		{
			continue;
		}

		const int32 W = Resolver->ComputeFinalWeight(Ev, TempSnap);
		if (W <= 0) continue;

		Candidates.Add({ Id, W });
		TotalWeight += W;
	}

	UE_LOG(LogTemp, Warning, TEXT("[StoneRunSubsystem] PickNextEvent: Pool=%d Candidates=%d"),
		EventPoolIds.Num(), Candidates.Num());

	if (Candidates.Num() == 0)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[StoneRunSubsystem] No eligible candidates. Pool=%d. Likely requirements or empty packs."),
			EventPoolIds.Num());

		CurrentEvent = nullptr;
		OnEventChanged.Broadcast(nullptr);
		return;
	}

	const int32 Roll = RNG.RandRange(1, TotalWeight);
	int32 Acc = 0;
	FName Picked = Candidates[0].Id;

	for (const auto& C : Candidates)
	{
		Acc += C.W;
		if (Roll <= Acc)
		{
			Picked = C.Id;
			break;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[StoneRunSubsystem] PickNextEvent: PickedId=%s Roll=%d/%d"),
		*Picked.ToString(), Roll, TotalWeight);

	CurrentEvent = LoadEventById(Picked);
	OnEventChanged.Broadcast(CurrentEvent);
}

// ==========================================================================
// TIME SYSTEM (UDS Integration)
// All time is controlled by Ultra Dynamic Sky via Blueprint callbacks.
// C++ only tracks counters - no internal time calculation.
// ==========================================================================

void UStoneRunSubsystem::OnSunrise()
{
	// Called by Blueprint when UDS fires OnSunrise
	Time.DayIndex += 1;
	Time.bIsNight = false;
	
	ApplyDayNightTags(false);
	
	UE_LOG(LogTemp, Log, TEXT("[StoneRunSubsystem] OnSunrise: Day %d started"), Time.DayIndex);
	
	RebuildSnapshot();
	BroadcastSnapshot();
}

void UStoneRunSubsystem::OnSunset()
{
	// Called by Blueprint when UDS fires OnSunset
	Time.TotalNightsPassed += 1;
	Time.bIsNight = true;
	
	ApplyDayNightTags(true);
	
	// Notify scheduler about night transition
	if (Scheduler)
	{
		Scheduler->NotifyChoiceAdvanced(Time, 0, true, true);
	}
	
	UE_LOG(LogTemp, Log, TEXT("[StoneRunSubsystem] OnSunset: Night %d started"), Time.TotalNightsPassed);
	
	RebuildSnapshot();
	BroadcastSnapshot();
}

void UStoneRunSubsystem::OnHourChanged(int32 NewHour)
{
	// Called by Blueprint when UDS fires OnHourChanged
	Time.CurrentHour = NewHour;
	
	// Roll for ambient event with configurable chance (default 5%)
	// This creates rare time-based events without event spam
	constexpr float AmbientEventChance = 0.05f;
	TryRollAmbientEvent(AmbientEventChance, /*bAutoPresent*/ false);
}

void UStoneRunSubsystem::ApplyDayNightTags(bool bNowNight)
{
	const FStoneGameplayTags& Tags = FStoneGameplayTags::Get();

	RunTags.RemoveTag(Tags.State_Day);
	RunTags.RemoveTag(Tags.State_Night);

	RunTags.AddTag(bNowNight ? Tags.State_Night : Tags.State_Day);
}

void UStoneRunSubsystem::IncrementChoiceCounter()
{
	Time.TotalChoices += 1;
	
	// Notify scheduler about choice advancement
	if (Scheduler)
	{
		Scheduler->NotifyChoiceAdvanced(Time, 1, false, Time.bIsNight);
	}
}

void UStoneRunSubsystem::ExecuteChoiceOutcomes(const FStoneChoiceData& Choice, bool bSoftFailPath)
{
	FStoneOutcomeContext Ctx;
	Ctx.ASC = GetASC();
	Ctx.Tags = &RunTags;
	Ctx.EventPoolIds = &EventPoolIds;
	Ctx.Scheduler = Scheduler;
	Ctx.Time = &Time;
	Ctx.FocusTag = &FocusTag;

	OutcomeExecutor->Execute(bSoftFailPath ? Choice.FailOutcomes : Choice.Outcomes, Ctx);

	for (const auto& S : Choice.Schedules)
	{
		Scheduler->Enqueue(S, Time);
	}
}

void UStoneRunSubsystem::ApplyChoice(int32 ChoiceIndex)
{
	if (!CurrentEvent) return;
	if (!CurrentEvent->Choices.IsValidIndex(ChoiceIndex)) return;

	UAbilitySystemComponent* ASC = GetASC();
	check(ASC);

	const FStoneChoiceData& Choice = CurrentEvent->Choices[ChoiceIndex];

	// Evaluate requirement; SoftFail stays executable
	const bool bReqOk = Resolver->EvaluateRequirement(Choice.Requirement, ASC, RunTags);
	const bool bSoftFail = (!bReqOk && Choice.LockMode == EStoneChoiceLockMode::SoftFail);

	if (!bReqOk && Choice.LockMode != EStoneChoiceLockMode::SoftFail)
	{
		// Disabled/Hidden should not be callable; UI must respect resolver results.
		return;
	}

	ExecuteChoiceOutcomes(Choice, bSoftFail);

	// Track choice count (time is managed by UDS, not advanced internally)
	IncrementChoiceCounter();

	// REALTIME ACTION RULE:
	// During ANY real-time action (Expedition, Travel via ActionSubsystem, or Travel via RunSubsystem)
	// we do NOT chain into the next random event immediately.
	// Instead we:
	//   1) Open the next pending event if one was queued (e.g. Arrival queued while Outbound was open)
	//   2) If no pending: clear event and return to "travel running, no event" state
	//
	// We check BOTH the tag-based state AND the ActionSubsystem directly,
	// because ExecuteChoiceOutcomes may strip tags before we get here.
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();
	const bool bTagGate = RunTags.HasTag(T.State_OnAction) || RunTags.HasTag(T.State_OnExpedition);

	// Also check ActionSubsystem directly (it owns its own bActionRunning flag
	// and is authoritative about whether an action is still in progress).
	// UStoneActionSubsystem is a UWorldSubsystem, so we get it from UWorld.
	bool bActionSubsystemRunning = false;
	if (UWorld* W = GetWorld())
	{
		if (UStoneActionSubsystem* ActionSS = W->GetSubsystem<UStoneActionSubsystem>())
		{
			bActionSubsystemRunning = ActionSS->IsActionRunning();
		}
	}

	const bool bRealtimeActionGate = bTagGate || bTravelActive || bExpeditionActive || bActionSubsystemRunning;

	UE_LOG(LogTemp, Log,
		TEXT("[StoneRunSubsystem] ApplyChoice: Gate check -> bRealtimeActionGate=%s (TagGate=%s Travel=%s Expedition=%s ActionSS=%s) PendingEvents=%d"),
		bRealtimeActionGate ? TEXT("YES") : TEXT("NO"),
		bTagGate ? TEXT("Y") : TEXT("N"),
		bTravelActive ? TEXT("Y") : TEXT("N"),
		bExpeditionActive ? TEXT("Y") : TEXT("N"),
		bActionSubsystemRunning ? TEXT("Y") : TEXT("N"),
		PendingEventIds.Num());

	if (bRealtimeActionGate)
	{
		// Close current event
		CurrentEvent = nullptr;

		// Deterministic: show next pending immediately if any (e.g. Arrival that was queued while Outbound was open)
		if (PendingEventIds.Num() > 0)
		{
			const FName NextId = PendingEventIds[0];
			PendingEventIds.RemoveAt(0);

			CurrentEvent = LoadEventById(NextId);
			OnEventChanged.Broadcast(CurrentEvent);

			UE_LOG(LogTemp, Log, TEXT("[StoneRunSubsystem] ApplyChoice: opened pending event '%s' (%d remaining)"),
				*NextId.ToString(), PendingEventIds.Num());

			RebuildSnapshot();
			BroadcastSnapshot();
			return;
		}

		// No pending -> stay in realtime travel state (action subsystem continues ticking)
		OnEventChanged.Broadcast(nullptr);

		// For expedition legacy path: ensure countdown is running
		if (bExpeditionActive)
		{
			QueueNextRealtimeEvent(false);
		}

		RebuildSnapshot();
		BroadcastSnapshot();
		return;
	}

	// After time step: scheduled forced first; random only if not idle-in-cave
	const bool bAllowRandom = !ShouldIdleBetweenEvents();
	PickNextEvent(/*scheduled*/true, /*random*/bAllowRandom, /*force*/false);

	RebuildSnapshot();
	BroadcastSnapshot();
}

void UStoneRunSubsystem::RebuildSnapshot()
{
	Snapshot.Time = Time;
	Snapshot.RunTags = RunTags;
	Snapshot.FocusTag = FocusTag;
	Snapshot.CurrentEventId = CurrentEvent ? CurrentEvent->EventId : NAME_None;

	if (UAbilitySystemComponent* ASC = GetASC())
	{
		Snapshot.Food   = ASC->GetNumericAttribute(UStoneAttributeSet::GetFoodAttribute());
		Snapshot.Water  = ASC->GetNumericAttribute(UStoneAttributeSet::GetWaterAttribute());
		Snapshot.Health = ASC->GetNumericAttribute(UStoneAttributeSet::GetHealthAttribute());
		Snapshot.Morale = ASC->GetNumericAttribute(UStoneAttributeSet::GetMoraleAttribute());
		Snapshot.Warmth = ASC->GetNumericAttribute(UStoneAttributeSet::GetWarmthAttribute());
		Snapshot.Trust  = ASC->GetNumericAttribute(UStoneAttributeSet::GetTrustAttribute());
	}
}

void UStoneRunSubsystem::BroadcastSnapshot()
{
	OnSnapshotChanged.Broadcast(Snapshot);
}

void UStoneRunSubsystem::AddEventsFromPackId(FName PackId, bool bPreloadIfPackRequests)
{
	if (PackId.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRunSubsystem] AddEventsFromPackId: PackId is None"));
		return;
	}

	EnsurePackLibrary(true);
	EnsureEventLibrary(true);

	if (!PackLibrary)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRunSubsystem] AddEventsFromPackId: PackLibrary is null"));
		return;
	}

	UStoneEventPackData* Pack = PackLibrary->GetPack(PackId);
	if (!Pack)
	{
		// Defensive: if library wasn't preloaded for some reason, try one-shot preload.
		TArray<FName> One;
		One.Add(PackId);
		PackLibrary->PreloadByIds(One, true);
		Pack = PackLibrary->GetPack(PackId);
	}

	if (!Pack)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[StoneRunSubsystem] AddEventsFromPackId: Pack '%s' NOT FOUND.\n")
			TEXT("- Check AssetManager PrimaryAssetTypesToScan for type 'StonePack'.\n")
			TEXT("- Ensure the pack asset lives under the scanned directory and is cooked."),
			*PackId.ToString());
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[StoneRunSubsystem] Adding pack '%s' with %d entries"),
		*PackId.ToString(), Pack->Events.Num());

	ActivePackIds.AddUnique(PackId);

	TArray<FName> PreloadEvents;

	for (const FStonePackEntry& Entry : Pack->Events)
	{
		if (Entry.EventId.IsNone())
		{
			continue;
		}
		EventPoolIds.AddUnique(Entry.EventId);
		if (bPreloadIfPackRequests && Pack->bPreloadOnUnlock)
		{
			PreloadEvents.AddUnique(Entry.EventId);
		}
	}

	if (PreloadEvents.Num() > 0 && EventLibrary)
	{
		EventLibrary->PreloadByIds(PreloadEvents, true);
	}
}

void UStoneRunSubsystem::BuildInitialEventPool(const FStoneRunConfig& Config)
{
	EnsurePackLibrary(true);
	EnsureEventLibrary(true);

	EventPoolIds.Reset();
	ActivePackIds.Reset();

	// SSOT: Start packs MUST come from Config (typically provided by UI->PC->Subsystem).
	if (Config.StartingPackIds.Num() == 0)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[StoneRunSubsystem] BuildInitialEventPool: Config.StartingPackIds is empty.\n")
			TEXT("SSOT rule: Starting packs must be provided by the caller (PlayerController/GameMode)."));
		return;
	}

	for (const FName& PackId : Config.StartingPackIds)
	{
		AddEventsFromPackId(PackId, /*bPreloadIfPackRequests*/ true);
	}

	// If enabled, immediately unlock any packs whose requirements are already satisfied at run start.
	TryAutoUnlockPacks();

	UE_LOG(LogTemp, Warning, TEXT("[StoneRunSubsystem] BuildInitialEventPool done: ActivePacks=%d Pool=%d"),
		ActivePackIds.Num(), EventPoolIds.Num());

	if (EventPoolIds.Num() == 0)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[StoneRunSubsystem] BuildInitialEventPool produced 0 EventIds.\n")
			TEXT("Most common causes:\n")
			TEXT("- AssetManager scan for StonePack/StoneEvent not configured or wrong directories\n")
			TEXT("- Pack asset has an empty Events[] list\n")
			TEXT("- Pack Entries reference EventIds that don't exist as StoneEvent assets"));
	}
}



// Old SetExternalDayNightAuthorityEnabled and SetIsNightFromExternal removed.
// Time is now fully controlled by UDS via OnSunrise(), OnSunset(), OnHourChanged().

void UStoneRunSubsystem::ActivatePackTemporary(FName PackId)
{
	if (PackId.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRunSubsystem] ActivatePackTemporary: PackId is None"));
		return;
	}

	const bool bWasActive = ActivePackIds.Contains(PackId);
	AddEventsFromPackId(PackId, /*bPreloadIfPackRequests*/ true);

	if (!bWasActive)
	{
		TemporaryPackIds.AddUnique(PackId);
	}
}

void UStoneRunSubsystem::DeactivatePackInternal(FName PackId)
{
	if (PackId.IsNone())
	{
		return;
	}

	ActivePackIds.Remove(PackId);
}

void UStoneRunSubsystem::RebuildEventPoolFromActivePacks(bool bPreloadIfPackRequests)
{
	EnsurePackLibrary(true);
	EnsureEventLibrary(true);

	EventPoolIds.Reset();

	if (!PackLibrary)
	{
		return;
	}

	TArray<FName> PreloadEvents;

	for (const FName& PackId : ActivePackIds)
	{
		if (PackId.IsNone())
		{
			continue;
		}

		UStoneEventPackData* Pack = PackLibrary->GetPack(PackId);
		if (!Pack)
		{
			continue;
		}

		for (const FStonePackEntry& Entry : Pack->Events)
		{
			if (Entry.EventId.IsNone())
			{
				continue;
			}
			EventPoolIds.AddUnique(Entry.EventId);

			if (bPreloadIfPackRequests && Pack->bPreloadOnUnlock)
			{
				PreloadEvents.AddUnique(Entry.EventId);
			}
		}
	}

	if (PreloadEvents.Num() > 0 && EventLibrary)
	{
		EventLibrary->PreloadByIds(PreloadEvents, true);
	}
}

void UStoneRunSubsystem::DeactivateTemporaryPacks()
{
	if (TemporaryPackIds.Num() == 0)
	{
		return;
	}

	for (const FName& PackId : TemporaryPackIds)
	{
		DeactivatePackInternal(PackId);
	}

	TemporaryPackIds.Reset();
	RebuildEventPoolFromActivePacks(/*bPreloadIfPackRequests*/ false);
}

void UStoneRunSubsystem::DeactivateTemporaryPacksByIds(const TArray<FName>& PackIds)
{
	if (PackIds.Num() == 0 || TemporaryPackIds.Num() == 0)
	{
		return;
	}

	bool bChanged = false;
	for (const FName& PackId : PackIds)
	{
		if (PackId.IsNone())
		{
			continue;
		}

		// Only remove packs that THIS subsystem activated temporarily.
		const int32 Index = TemporaryPackIds.Find(PackId);
		if (Index != INDEX_NONE)
		{
			TemporaryPackIds.RemoveAt(Index);
			DeactivatePackInternal(PackId);
			bChanged = true;
		}
	}

	if (bChanged)
	{
		RebuildEventPoolFromActivePacks(/*bPreloadIfPackRequests*/ false);
	}
}

void UStoneRunSubsystem::QueueEventByTag(const FGameplayTag& EventTag, bool bAutoPresent)
{
	FName PickedEventId;
	if (!TryPickEventIdByTag(EventTag, PickedEventId) || PickedEventId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneRunSubsystem] QueueEventByTag: No event found for tag: %s"), *EventTag.ToString());
		return;
	}

	// Auto present: open immediately if nothing is currently open.
	// IMPORTANT: do not also queue into PendingEventIds in that case (otherwise the same event will be opened twice).
	if (bAutoPresent && !HasOpenEvent())
	{
		CurrentEvent = LoadEventById(PickedEventId);
		OnEventChanged.Broadcast(CurrentEvent);
		UE_LOG(LogTemp, Log, TEXT("[StoneRunSubsystem] QueueEventByTag: Auto-presented '%s' for tag %s"), *PickedEventId.ToString(), *EventTag.ToString());
	}
	else
	{
		PendingEventIds.AddUnique(PickedEventId);
		UE_LOG(LogTemp, Log, TEXT("[StoneRunSubsystem] QueueEventByTag: Queued '%s' for tag %s (AutoPresent=%s, OpenEvent=%s)"),
			*PickedEventId.ToString(), *EventTag.ToString(), bAutoPresent ? TEXT("true") : TEXT("false"), HasOpenEvent() ? TEXT("true") : TEXT("false"));
	}

	RebuildSnapshot();
	BroadcastSnapshot();
}

bool UStoneRunSubsystem::HasOpenEvent() const
{
	return CurrentEvent != nullptr;
}

void UStoneRunSubsystem::AddStateTags(const FGameplayTagContainer& TagsToAdd)
{
	if (TagsToAdd.IsEmpty())
	{
		return;
	}

	RunTags.AppendTags(TagsToAdd);

	// Mirror run-state tags onto the player's ASC as loose gameplay tags.
	// Multiplayer-safe: only the server should mutate authoritative loose tag counts.
	if (UAbilitySystemComponent* ASC = GetASC())
	{
		if (AActor* OwnerActor = ASC->GetOwner())
		{
			if (OwnerActor->HasAuthority())
			{
				ASC->AddLooseGameplayTags(TagsToAdd);
			}
		}
	}
}

void UStoneRunSubsystem::RemoveStateTags(const FGameplayTagContainer& TagsToRemove)
{
	if (TagsToRemove.IsEmpty())
	{
		return;
	}

	RunTags.RemoveTags(TagsToRemove);

	// Multiplayer-safe: only the server should mutate authoritative loose tag counts.
	if (UAbilitySystemComponent* ASC = GetASC())
	{
		if (AActor* OwnerActor = ASC->GetOwner())
		{
			if (OwnerActor->HasAuthority())
			{
				ASC->RemoveLooseGameplayTags(TagsToRemove);
			}
		}
	}
}

FGameplayTagContainer UStoneRunSubsystem::GetCurrentStateTags() const
{
	return RunTags;
}


bool UStoneRunSubsystem::TryPickEventIdByTag(const FGameplayTag& RequiredTag, FName& OutEventId) const
{
	OutEventId = NAME_None;

	if (!Resolver || EventPoolIds.Num() == 0)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return false;
	}

	FStoneSnapshot TempSnap = Snapshot;
	TempSnap.FocusTag = FocusTag;
	TempSnap.Time = Time;
	TempSnap.RunTags = RunTags;

	struct FCandidate { FName Id; int32 W; };
	TArray<FCandidate> Candidates;
	int32 TotalWeight = 0;

	for (const FName& Id : EventPoolIds)
	{
		UStoneEventData* Ev = LoadEventById(Id);
		if (!Ev) continue;

		if (RequiredTag.IsValid() && !Ev->EventTags.HasTag(RequiredTag))
		{
			continue;
		}

		if (!Resolver->EvaluateRequirement(Ev->Requirement, ASC, RunTags))
		{
			continue;
		}

		const int32 W = Resolver->ComputeFinalWeight(Ev, TempSnap);
		if (W <= 0) continue;

		Candidates.Add({ Id, W });
		TotalWeight += W;
	}

	if (Candidates.Num() == 0 || TotalWeight <= 0)
	{
		return false;
	}

	const int32 Roll = RNG.RandRange(1, TotalWeight);
	int32 Acc = 0;
	for (const auto& C : Candidates)
	{
		Acc += C.W;
		if (Roll <= Acc)
		{
			OutEventId = C.Id;
			return true;
		}
	}

	OutEventId = Candidates.Last().Id;
	return true;
}

void UStoneRunSubsystem::StartTravelAction(FName InTravelPackId, float TotalSecondsAtSpeed1, float RandomMinGapSeconds, float RandomMaxGapSeconds, float RandomChance01, bool bTriggerFirstOutboundEventImmediately)
{
	if (InTravelPackId.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRunSubsystem] StartTravelAction: TravelPackId is None"));
		return;
	}

	// Demo rule: do not stack actions from different systems.
	StopWorldActionSubsystemIfRunning(GetWorld(), TEXT("StartTravelAction"));

	// Demo rule: do not stack real-time actions.
	if (bExpeditionActive)
	{
		StopExpedition(/*bForceReturnEvent*/ false);
	}

	// If another travel is active, stop it first (no stacking actions in demo).
	if (bTravelActive)
	{
		StopTravelAction(/*bForceReturnHomeEvent*/ false);
	}

	if (!EnsurePlayerStateCache())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRunSubsystem] StartTravelAction failed: No PlayerState."));
		return;
	}
	EnsureEventLibrary(true);
	EnsurePackLibrary(true);

	ActivatePackTemporary(InTravelPackId);

	const FStoneGameplayTags& T = FStoneGameplayTags::Get();
	RunTags.AddTag(T.State_OnAction);
	RunTags.AddTag(T.State_OnTravel);

	bTravelActive = true;
	TravelPackId = InTravelPackId;

	TravelTotalSeconds = FMath::Max(1.f, TotalSecondsAtSpeed1);
	TravelOutboundSeconds = TravelTotalSeconds * 0.5f;
	TravelReturnSeconds = TravelTotalSeconds - TravelOutboundSeconds;

	TravelRandomMinGapSeconds = FMath::Max(5.f, RandomMinGapSeconds);
	TravelRandomMaxGapSeconds = FMath::Max(TravelRandomMinGapSeconds, RandomMaxGapSeconds);
	TravelRandomChance01 = FMath::Clamp(RandomChance01, 0.f, 1.f);

	TravelLegElapsedSeconds = 0.f;
	TravelRandomCountdownSeconds = bTriggerFirstOutboundEventImmediately ? 0.f : RNG.FRandRange(TravelRandomMinGapSeconds, TravelRandomMaxGapSeconds);

	// Hide any current event and start outbound.
	ReturnToRealtimeTravelState();
	EnterTravelPhase(EStoneTravelPhase::Outbound);

	StartRealtimeActionTick();

	RebuildSnapshot();
	BroadcastSnapshot();
}

void UStoneRunSubsystem::StopTravelAction(bool bForceReturnHomeEvent)
{
	if (!bTravelActive)
	{
		return;
	}

	if (bForceReturnHomeEvent && !CurrentEvent)
	{
		ForceTravelReturnHomeEvent();
	}

	bTravelActive = false;
	TravelPackId = NAME_None;
	TravelPhase = EStoneTravelPhase::None;
	TravelTotalSeconds = 0.f;
	TravelOutboundSeconds = 0.f;
	TravelReturnSeconds = 0.f;
	TravelLegElapsedSeconds = 0.f;
	TravelRandomCountdownSeconds = 0.f;

	const FStoneGameplayTags& T = FStoneGameplayTags::Get();
	RunTags.RemoveTag(T.State_OnTravel);
	RunTags.RemoveTag(T.State_OnAction);

	StopRealtimeActionTick();
	DeactivateTemporaryPacks();

	RebuildSnapshot();
	BroadcastSnapshot();
}

float UStoneRunSubsystem::GetTravelProgress01() const
{
	if (!bTravelActive || TravelTotalSeconds <= 0.f)
	{
		return 0.f;
	}

	float Done = 0.f;
	switch (TravelPhase)
	{
	case EStoneTravelPhase::Outbound:
		Done = TravelLegElapsedSeconds;
		break;
	case EStoneTravelPhase::Arrival:
		Done = TravelOutboundSeconds;
		break;
	case EStoneTravelPhase::Return:
		Done = TravelOutboundSeconds + TravelLegElapsedSeconds;
		break;
	case EStoneTravelPhase::Completed:
		Done = TravelTotalSeconds;
		break;
	default:
		Done = 0.f;
		break;
	}

	return FMath::Clamp(Done / TravelTotalSeconds, 0.f, 1.f);
}

float UStoneRunSubsystem::GetTravelLegProgress01() const
{
	if (!bTravelActive)
	{
		return 0.f;
	}

	const float Denom = (TravelPhase == EStoneTravelPhase::Outbound) ? TravelOutboundSeconds :
		(TravelPhase == EStoneTravelPhase::Return) ? TravelReturnSeconds : 0.f;

	if (Denom <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(TravelLegElapsedSeconds / Denom, 0.f, 1.f);
}

void UStoneRunSubsystem::EnterTravelPhase(EStoneTravelPhase NewPhase)
{
	TravelPhase = NewPhase;
	TravelLegElapsedSeconds = 0.f;

	// When entering phases that must show an event, we show immediately (if possible).
	if (TravelPhase == EStoneTravelPhase::Arrival)
	{
		ForceTravelArrivalEvent();
	}
	else if (TravelPhase == EStoneTravelPhase::Completed)
	{
		ForceTravelReturnHomeEvent();
	}
}

void UStoneRunSubsystem::ForceTravelArrivalEvent()
{
	if (CurrentEvent)
	{
		return;
	}

	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	FName EventId;
	if (TryPickEventIdByTag(T.Event_Travel_Arrival, EventId) && !EventId.IsNone())
	{
		CurrentEvent = LoadEventById(EventId);
		OnEventChanged.Broadcast(CurrentEvent);
	}
	else
	{
		// If no arrival event exists, we still progress (demo-safe), but we log once.
		UE_LOG(LogTemp, Warning, TEXT("[StoneRunSubsystem] Travel arrival: no Event.Travel.Arrival event found in pool."));
		// Immediately continue to return leg.
		EnterTravelPhase(EStoneTravelPhase::Return);
	}
}

void UStoneRunSubsystem::ForceTravelReturnHomeEvent()
{
	if (CurrentEvent)
	{
		return;
	}

	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	FName EventId;
	if (TryPickEventIdByTag(T.Event_Travel_ReturnHome, EventId) && !EventId.IsNone())
	{
		CurrentEvent = LoadEventById(EventId);
		OnEventChanged.Broadcast(CurrentEvent);
	}
	else
	{
		// No return-home event: travel ends quietly (UI can react to phase Completed).
		UE_LOG(LogTemp, Warning, TEXT("[StoneRunSubsystem] Travel return-home: no Event.Travel.ReturnHome event found in pool. Travel completed without final event."));
	}
}

bool UStoneRunSubsystem::TryRollAmbientEvent(float Chance01, bool bAutoPresent)
{
	if (Chance01 <= 0.f)
	{
		return false;
	}

	// No ambient rolls if an event is open or a real-time action is active.
	if (CurrentEvent || bExpeditionActive || bTravelActive)
	{
		return false;
	}

	// If we're explicitly "idle in cave", allow ambient (later you can gate by tags).
	// If you're not in cave, you can still allow it; this is a design hook.
	const float Roll = RNG.FRand();
	if (Roll > FMath::Clamp(Chance01, 0.f, 1.f))
	{
		return false;
	}

	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	FName PickedId;
	if (!TryPickEventIdByTag(T.Event_Ambient, PickedId) || PickedId.IsNone())
	{
		return false;
	}

	if (bAutoPresent)
	{
		CurrentEvent = LoadEventById(PickedId);
		OnEventChanged.Broadcast(CurrentEvent);
	}
	else
	{
		PendingEventIds.AddUnique(PickedId);
	}

	RebuildSnapshot();
	BroadcastSnapshot();
	return true;
}

bool UStoneRunSubsystem::OpenNextPendingEvent()
{
	if (CurrentEvent || PendingEventIds.Num() == 0)
	{
		return false;
	}

	const FName NextId = PendingEventIds[0];
	PendingEventIds.RemoveAt(0);

	CurrentEvent = LoadEventById(NextId);
	OnEventChanged.Broadcast(CurrentEvent);

	RebuildSnapshot();
	BroadcastSnapshot();
	return CurrentEvent != nullptr;
}

void UStoneRunSubsystem::StartRealtimeActionTick()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (World->GetTimerManager().IsTimerActive(ExpeditionTickHandle))
	{
		return;
	}

	// 4 Hz scheduling tick
	World->GetTimerManager().SetTimer(ExpeditionTickHandle, this, &UStoneRunSubsystem::TickRealtimeActions, 0.25f, true);
}

void UStoneRunSubsystem::StopRealtimeActionTick()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExpeditionTickHandle);
	}
}

void UStoneRunSubsystem::TickRealtimeActions()
{
	// Stop ticking if no real-time actions are active.
	if (!bExpeditionActive && !bTravelActive)
	{
		StopRealtimeActionTick();
		return;
	}

	// Pause means: nothing advances and no events trigger.
	if (SimulationSpeed <= 0.f)
	{
		return;
	}

	// If an event is currently on screen, wait for player choice.
	if (CurrentEvent)
	{
		return;
	}

	constexpr float BaseDelta = 0.25f;
	const float Dt = BaseDelta * SimulationSpeed;

	// === Expedition (existing behavior, now paced) ===
	if (bExpeditionActive)
	{
		ExpeditionElapsedSeconds += Dt;
		RealtimeNextEventCountdown -= Dt;

		if (ExpeditionElapsedSeconds >= ExpeditionDurationSeconds)
		{
			ForceReturnEvent();
			bExpeditionActive = false;
			SetOnExpeditionTag(false);

			const FStoneGameplayTags& T = FStoneGameplayTags::Get();
			RunTags.RemoveTag(T.State_OnAction);

			// If we're currently travelling (no event), show the return event now.
			if (!CurrentEvent)
			{
				PickNextEvent(/*scheduled*/true, /*random*/true, /*force*/true);
			}

			// End of action -> remove temporary packs
			DeactivateTemporaryPacks();

			RebuildSnapshot();
			BroadcastSnapshot();
			return;
		}

		if (RealtimeNextEventCountdown <= 0.f)
		{
			PickNextEvent(/*scheduled*/true, /*random*/true, /*force*/true);
			QueueNextRealtimeEvent(false);
		}
	}

	// === Travel Action (phased) ===
	if (bTravelActive)
	{
		TravelRandomCountdownSeconds -= Dt;

		// Advance leg time only during travel legs (not during forced arrival phase)
		if (TravelPhase == EStoneTravelPhase::Outbound)
		{
			TravelLegElapsedSeconds += Dt;

			// Optional random outbound event
			if (TravelRandomCountdownSeconds <= 0.f)
			{
				const float Roll = RNG.FRand();
				if (Roll <= TravelRandomChance01)
				{
					const FStoneGameplayTags& T = FStoneGameplayTags::Get();
					FName OutId;
					if (TryPickEventIdByTag(T.Event_Travel_Outbound, OutId) && !OutId.IsNone())
					{
						CurrentEvent = LoadEventById(OutId);
						OnEventChanged.Broadcast(CurrentEvent);
					}
				}

				TravelRandomCountdownSeconds = RNG.FRandRange(TravelRandomMinGapSeconds, TravelRandomMaxGapSeconds);
			}

			if (TravelLegElapsedSeconds >= TravelOutboundSeconds)
			{
				EnterTravelPhase(EStoneTravelPhase::Arrival);
				RebuildSnapshot();
				BroadcastSnapshot();
				return;
			}
		}
		else if (TravelPhase == EStoneTravelPhase::Return)
		{
			TravelLegElapsedSeconds += Dt;

			// Optional random return event
			if (TravelRandomCountdownSeconds <= 0.f)
			{
				const float Roll = RNG.FRand();
				if (Roll <= TravelRandomChance01)
				{
					const FStoneGameplayTags& T = FStoneGameplayTags::Get();
					FName OutId;
					if (TryPickEventIdByTag(T.Event_Travel_Return, OutId) && !OutId.IsNone())
					{
						CurrentEvent = LoadEventById(OutId);
						OnEventChanged.Broadcast(CurrentEvent);
					}
				}

				TravelRandomCountdownSeconds = RNG.FRandRange(TravelRandomMinGapSeconds, TravelRandomMaxGapSeconds);
			}

			if (TravelLegElapsedSeconds >= TravelReturnSeconds)
			{
				EnterTravelPhase(EStoneTravelPhase::Completed);
				// Travel action is considered finished after return-home event resolves or immediately if none exists.
				// We keep bTravelActive true until UI resolves the final event (if any). If no final event, we end now.
				if (!CurrentEvent)
				{
					// No final event -> stop immediately.
					StopTravelAction(/*bForceReturnHomeEvent*/ false);
				}
				else
				{
					RebuildSnapshot();
					BroadcastSnapshot();
				}
				return;
			}
		}
	}
}
