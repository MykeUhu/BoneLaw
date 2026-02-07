#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Data/StoneTypes.h"
#include "Game/StoneRunTraceBuffer.h"
#include "Game/Events/StoneEventResolver.h"

#include "Core/StoneContentSettings.h"
#include "Data/StoneEventPackData.h"
#include "TimerManager.h"
#include "Core/StoneGameplayTags.h"
#include "StoneRunSubsystem.generated.h"

class UStonePackLibrary;
struct FStoneChoiceData;
class UStoneWorldlineWeightPolicy;
class UStoneWorldlineDirector;
class UStoneEventLibrary;
class UStoneEventData;
class UStoneScheduler;
class UStoneEventResolver;
class UStoneOutcomeExecutor;
class UStoneSaveGame;
class AStonePlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneSnapshotChanged, const FStoneSnapshot&, Snapshot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneEventChanged, const UStoneEventData*, Event);

USTRUCT(BlueprintType)
struct FStoneRunConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RNGSeed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer StartingTags;

	// StableName -> Value (uses TagsToAttributes from UStoneAttributeSet)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> StartingAttributeValues;

	// Which packs are active at start
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> StartingPackIds;

	// If true, all packs are known but locked by requirements (recommended)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableAutoPackUnlocks = true;
};

UENUM(BlueprintType)
enum class EStoneTravelPhase : uint8
{
	None UMETA(DisplayName="None"),
	Outbound UMETA(DisplayName="Outbound"),
	Arrival UMETA(DisplayName="Arrival"),
	Return UMETA(DisplayName="Return"),
	Completed UMETA(DisplayName="Completed")
};

UENUM(BlueprintType)
enum class EStoneRealtimeActionType : uint8
{
	None UMETA(DisplayName="None"),
	ExploreExpedition UMETA(DisplayName="Explore Expedition"),
	Travel UMETA(DisplayName="Travel")
};


UCLASS()
class BONELAW_API UStoneRunSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// === Lifecycle ===
	UFUNCTION(BlueprintCallable, Category="Stone|Run")
	void StartNewRun(const FStoneRunConfig& Config);

	UFUNCTION(BlueprintCallable, Category="Stone|Run")
	void LoadRun(const UStoneSaveGame* Save);

	UFUNCTION(BlueprintCallable, Category="Stone|Run")
	UStoneSaveGame* CreateSave() const;

	// === Gameplay ===
	UFUNCTION(BlueprintCallable, Category="Stone|Run")
	void SetFocus(FGameplayTag InFocusTag);

	UFUNCTION(BlueprintCallable, Category="Stone|Run")
	void ApplyChoice(int32 ChoiceIndex);

	UFUNCTION(BlueprintCallable, Category="Stone|Run")
	const UStoneEventData* GetCurrentEvent() const { return CurrentEvent; }

	UFUNCTION(BlueprintCallable, Category="Stone|Run")
	FStoneSnapshot GetSnapshot() const { return Snapshot; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category="Stone|Run")
	FStoneSnapshotChanged OnSnapshotChanged;

	UPROPERTY(BlueprintAssignable, Category="Stone|Run")
	FStoneEventChanged OnEventChanged;


	
	// === Simulation Speed (for real-time actions like expeditions) ===
	UFUNCTION(BlueprintCallable, Category="Stone|Sim")
	void SetSimulationSpeed(float NewSpeed);

	UFUNCTION(BlueprintPure, Category="Stone|Sim")
	float GetSimulationSpeed() const { return SimulationSpeed; }

	// === Expeditions (real-time, spaced events) ===
	// Starts a real-time expedition that reveals events over time.
	// - DurationSeconds is the trip duration at 1.0 speed (e.g. 300 = 5 minutes).
	// - Events will pop up at random intervals between MinEventGapSeconds and MaxEventGapSeconds.
	// - Pause works automatically: if SimulationSpeed == 0, nothing advances and no events trigger.
	// - At the end, we force a "return" event if available (prefers events tagged Event_ExploreReturn).
	UFUNCTION(BlueprintCallable, Category="Stone|Expedition")
	void StartExploreExpedition(FName ExplorePackId, float DurationSeconds = 300.f, float MinEventGapSeconds = 20.f, float MaxEventGapSeconds = 60.f, bool bTriggerFirstEventImmediately = false);

	UFUNCTION(BlueprintCallable, Category="Stone|Expedition")
	void StopExpedition(bool bForceReturnEvent = false);

	UFUNCTION(BlueprintPure, Category="Stone|Expedition")
	bool IsOnExpedition() const;

	UFUNCTION(BlueprintPure, Category="Stone|Expedition")
	float GetExpeditionProgress01() const;
	

	// === Travel Actions (real-time, phased: outbound -> arrival -> return) ===
	// Travel is action-driven and player-facing:
	// - Uses SimulationSpeed (0 = paused) to allow UI-driven pause.
	// - Activates the given Pack only for the duration of the travel action (temporary pack).
	// - Forced phase events are selected by tags:
	//   * Event.Travel.Arrival (must happen at destination)
	//   * Event.Travel.ReturnHome (end of travel; if missing, travel simply completes)
	// - Optional random travel events can occur on outbound/return, gated by cooldown+chance:
	//   * Event.Travel.Outbound, Event.Travel.Return
	UFUNCTION(BlueprintCallable, Category="Stone|Action|Travel")
	void StartTravelAction(FName TravelPackId, float TotalSecondsAtSpeed1 = 300.f, float RandomMinGapSeconds = 30.f, float RandomMaxGapSeconds = 90.f, float RandomChance01 = 0.25f, bool bTriggerFirstOutboundEventImmediately = false);

	UFUNCTION(BlueprintCallable, Category="Stone|Action|Travel")
	void StopTravelAction(bool bForceReturnHomeEvent = false);

	UFUNCTION(BlueprintPure, Category="Stone|Action|Travel")
	bool IsTravelActive() const { return bTravelActive; }

	UFUNCTION(BlueprintPure, Category="Stone|Action|Travel")
	EStoneTravelPhase GetTravelPhase() const { return TravelPhase; }

	UFUNCTION(BlueprintPure, Category="Stone|Action|Travel")
	float GetTravelProgress01() const;

	UFUNCTION(BlueprintPure, Category="Stone|Action|Travel")
	float GetTravelLegProgress01() const;

	// === Ambient / Idle random events (usually queued, not auto-presented) ===
	// Call this from Ultra Dynamic Skies "OnHourChanged" (or similar) to roll for a rare ambient event.
	// If bAutoPresent == false, the event will be queued and must be opened by the UI explicitly.
	UFUNCTION(BlueprintCallable, Category="Stone|Ambient")
	bool TryRollAmbientEvent(float Chance01 = 0.15f, bool bAutoPresent = false);

	UFUNCTION(BlueprintPure, Category="Stone|Ambient")
	int32 GetPendingEventCount() const { return PendingEventIds.Num(); }

	UFUNCTION(BlueprintCallable, Category="Stone|Ambient")
	bool OpenNextPendingEvent();

	// ==========================================================================
	// TIME SYSTEM (UDS Integration)
	// ==========================================================================
	// All time data comes from Ultra Dynamic Sky (UDS) via Blueprint.
	// C++ does NOT calculate time internally - only tracks counters.
	//
	// Blueprint (GameMode or PlayerController) binds to UDS events:
	//   - UDS OnSunrise  -> Call OnSunrise()
	//   - UDS OnSunset   -> Call OnSunset()  
	//   - UDS OnHourChanged -> Call OnHourChanged(Hour)
	// ==========================================================================

	/** Called by Blueprint when UDS fires OnSunrise. Increments DayIndex, sets bIsNight=false. */
	UFUNCTION(BlueprintCallable, Category="Stone|Time")
	void OnSunrise();

	/** Called by Blueprint when UDS fires OnSunset. Increments TotalNightsPassed, sets bIsNight=true. */
	UFUNCTION(BlueprintCallable, Category="Stone|Time")
	void OnSunset();

	/** Called by Blueprint when UDS fires OnHourChanged. Updates CurrentHour and may roll ambient event. */
	UFUNCTION(BlueprintCallable, Category="Stone|Time")
	void OnHourChanged(int32 NewHour);

	/** Returns current time state (read-only). */
	UFUNCTION(BlueprintPure, Category="Stone|Time")
	const FStoneTimeState& GetTimeState() const { return Time; }

	/** Returns true if currently night. */
	UFUNCTION(BlueprintPure, Category="Stone|Time")
	bool IsNight() const { return Time.bIsNight; }

	/** Returns current day index (starts at 1). */
	UFUNCTION(BlueprintPure, Category="Stone|Time")
	int32 GetCurrentDay() const { return Time.DayIndex; }

	// Helper (uses TagsToAttributes from UStoneAttributeSet for StableName lookup)
	void SetAttrByStableName(const FName& StableName, float Value) const;

	UFUNCTION(BlueprintCallable, Category="Stone|Run")
	void GetResolvedChoices(TArray<FStoneChoiceResolved>& OutResolved) const;


	// === Pack Control (Action-driven) ===
	UFUNCTION(BlueprintCallable, Category="Stone|Packs")
	void ActivatePackTemporary(FName PackId);

	UFUNCTION(BlueprintCallable, Category="Stone|Packs")
	void DeactivateTemporaryPacks();

	// Deactivate only the provided temporary packs.
	// Safe no-op for packs that were not activated temporarily (e.g. packs that were already active).
	UFUNCTION(BlueprintCallable, Category="Stone|Packs")
	void DeactivateTemporaryPacksByIds(const TArray<FName>& PackIds);

	UFUNCTION(BlueprintPure, Category="Stone|Packs")
	const TArray<FName>& GetActivePackIds() const { return ActivePackIds; }

	UFUNCTION(BlueprintCallable, Category="Stone|Trace")
	UStoneRunTraceBuffer* GetTraceBuffer() const { return Trace; }
	
	// Queue event requests by tag (Action uses bAutoPresent=true; ambient uses false)
	void QueueEventByTag(const FGameplayTag& EventTag, bool bAutoPresent);

	// State queries (for action ticking)
	bool HasOpenEvent() const;

	UFUNCTION(BlueprintPure, Category="Stone|Action")
	bool IsAnyRealtimeActionActive() const { return bExpeditionActive || bTravelActive; }

	// Tag state changes (if you already do this elsewhere, reuse)
	void AddStateTags(const FGameplayTagContainer& TagsToAdd);
	void RemoveStateTags(const FGameplayTagContainer& TagsToRemove);
	FGameplayTagContainer GetCurrentStateTags() const;

protected:
	virtual void Deinitialize() override;

private:
	// === GAS Access via PlayerState ===
	// The PlayerState owns the AbilitySystemComponent. We cache a weak reference
	// and refresh it when needed. This ensures GAS is properly owned by PlayerState
	// per Unreal Engine best practices for multiplayer.
	UPROPERTY()
	TWeakObjectPtr<AStonePlayerState> CachedPlayerState;

	/** Attempts to find and cache the local player's PlayerState. Returns true if valid. */
	bool EnsurePlayerStateCache();

	/** Returns the cached PlayerState or nullptr if not available. */
	AStonePlayerState* GetPlayerState() const;

	UPROPERTY()
	TObjectPtr<UStoneScheduler> Scheduler;

	UPROPERTY()
	TObjectPtr<UStoneEventResolver> Resolver;

	UPROPERTY()
	TObjectPtr<UStoneOutcomeExecutor> OutcomeExecutor;

	UPROPERTY()
	TObjectPtr<UStoneEventData> CurrentEvent;

	UPROPERTY()
	FStoneSnapshot Snapshot;

	UPROPERTY()
	FStoneTimeState Time;

	UPROPERTY()
	FGameplayTagContainer RunTags;

	UPROPERTY()
	FGameplayTag FocusTag;

	UPROPERTY()
	TArray<FName> EventPoolIds;

	UPROPERTY()
	TObjectPtr<UStoneRunTraceBuffer> Trace;

	void TraceAdd(EStoneTraceType Type, const FString& Details, FName EventId = NAME_None, int32 ChoiceIndex = INDEX_NONE);

	UPROPERTY()
	TObjectPtr<UStoneEventLibrary> EventLibrary;

	UPROPERTY()
	TObjectPtr<UStoneWorldlineDirector> Worldline;

	UStoneEventData* GetEventFast(FName EventId) const;
	void EnsureEventLibrary(bool bPreloadAllSync);

	UPROPERTY()
	TObjectPtr<UStoneWorldlineWeightPolicy> WeightPolicy;

	// Deterministic RNG
	FRandomStream RNG;

	// === Helpers ===
	void BuildInitialEventPool(const FStoneRunConfig& Config);
	void AddEventsFromPackId(FName PackId, bool bPreloadIfPackRequests);

	void RebuildSnapshot();
	void BroadcastSnapshot();

	// --- Realtime / Idle flow ---
	bool ShouldIdleBetweenEvents() const;

	// allows: forced scheduled only, optional random pool
	void PickNextEvent(bool bAllowScheduledOverride, bool bAllowRandomFromPool, bool bForceRandomEvenIfIdle = false);

	UPROPERTY()
	TArray<FName> RealtimeForcedQueue;

	bool TryConsumeScheduledForcedEvent(FName& OutEventId);

	void ExecuteChoiceOutcomes(const FStoneChoiceData& Choice, bool bSoftFailPath);

	/** Increments TotalChoices counter. Called after each player action/decision. */
	void IncrementChoiceCounter();

	/** Applies day/night gameplay tags based on current state. */
	void ApplyDayNightTags(bool bNowNight);

	// Utility
	UAbilitySystemComponent* GetASC() const;

	void EnsureWorldlineDirector();
	void UpdateWorldlineAndUnlocks();

	void EnsureWeightPolicy();
	float ComputeCrisisMultiplier(const FStoneSnapshot& Snap, const UStoneEventData* Event) const;

	UStoneEventData* LoadEventById(FName EventId) const;

	UPROPERTY()
	TObjectPtr<UStonePackLibrary> PackLibrary;

	UPROPERTY()
	TArray<FName> ActivePackIds;

	UPROPERTY()
	TArray<FName> KnownPackIds;

	UPROPERTY()
	bool bAutoPackUnlocksEnabled = true;
	
	// Realtime simulation speed (0 = paused). Used by real-time systems (expeditions etc.)
	UPROPERTY()
	float SimulationSpeed = 1.f;

	// --- Expedition runtime state (real-time) ---
	UPROPERTY()
	bool bExpeditionActive = false;

	UPROPERTY()
	FName ExpeditionPackId = NAME_None;

	UPROPERTY()
	float ExpeditionDurationSeconds = 0.f;

	UPROPERTY()
	float ExpeditionElapsedSeconds = 0.f;

	// Countdown until the next event may appear (seconds, scaled by SimulationSpeed)
	UPROPERTY()
	float RealtimeNextEventCountdown = 0.f;

	UPROPERTY()
	float RealtimeMinEventGapSeconds = 20.f;

	UPROPERTY()
	float RealtimeMaxEventGapSeconds = 60.f;

	UPROPERTY()
	bool bExpeditionReturnQueued = false;

// --- Temporary packs (activated only while an action is active) ---
UPROPERTY()
TArray<FName> TemporaryPackIds;

// --- Pending events (ambient/idle) ---
UPROPERTY()
TArray<FName> PendingEventIds;

// (Time is now externally controlled by UDS - no internal day/night calculation)

// --- Travel runtime state (real-time action) ---
UPROPERTY()
bool bTravelActive = false;

UPROPERTY()
FName TravelPackId = NAME_None;

UPROPERTY()
EStoneTravelPhase TravelPhase = EStoneTravelPhase::None;

UPROPERTY()
float TravelTotalSeconds = 0.f;

UPROPERTY()
float TravelOutboundSeconds = 0.f;

UPROPERTY()
float TravelReturnSeconds = 0.f;

UPROPERTY()
float TravelLegElapsedSeconds = 0.f;

// Countdown until next optional random travel event (scaled by SimulationSpeed)
UPROPERTY()
float TravelRandomCountdownSeconds = 0.f;

UPROPERTY()
float TravelRandomMinGapSeconds = 30.f;

UPROPERTY()
float TravelRandomMaxGapSeconds = 90.f;

UPROPERTY()
float TravelRandomChance01 = 0.25f;


	FTimerHandle ExpeditionTickHandle;

	void SetOnExpeditionTag(bool bOn);
	void StartExpeditionTick();
	void StopExpeditionTick();
	void TickExpedition();
	void ReturnToRealtimeTravelState();
	void QueueNextRealtimeEvent(bool bAllowImmediate);
	void ForceReturnEvent();
	bool TryPickReturnEventId(FName& OutEventId);


	// Pack pool maintenance
	void RebuildEventPoolFromActivePacks(bool bPreloadIfPackRequests);

	bool IsPackActive(FName PackId) const { return ActivePackIds.Contains(PackId); }
	void DeactivatePackInternal(FName PackId);

	// Generic tag-based picker (used for travel phases)
	bool TryPickEventIdByTag(const FGameplayTag& RequiredTag, FName& OutEventId) const;

	// Travel ticking / forced phases
	void StartRealtimeActionTick();
	void StopRealtimeActionTick();
	void TickRealtimeActions();
	void EnterTravelPhase(EStoneTravelPhase NewPhase);
	void ForceTravelArrivalEvent();
	void ForceTravelReturnHomeEvent();

	void TryAutoUnlockPacks();
	void EnsurePackLibrary(bool bPreloadAllSync);
};
