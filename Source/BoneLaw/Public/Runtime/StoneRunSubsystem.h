#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Data/StoneTypes.h"
#include "Game/StoneRunTraceBuffer.h"
#include "StoneRunSubsystem.generated.h"

class UStonePackLibrary;
class UStoneEventLibrary;
class UStoneEventPackData;
class UStoneEventData;
class AStonePlayerState;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneSnapshotChanged, const FStoneSnapshot&, Snapshot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneEventChanged, const UStoneEventData*, Event);

/**
 * Global run state (very small): time counters + run tags + pack libraries.
 *
 * IMPORTANT SSOT RULES
 * - Attributes live on the owning ASC (PlayerState / agent ASC). This subsystem never stores attribute truth.
 * - Save/Load lives in SaveGame. This subsystem does not snapshot for persistence.
 * - Encounters are handled per-settler by UStoneSettlerActionComponent / UStoneActionRuntime.
 *
 * NOTE: A few UI-facing APIs remain as compatibility stubs (OnEventChanged / GetResolvedChoices)
 * so the UI can be migrated incrementally.
 */
USTRUCT(BlueprintType)
struct FStoneRunConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RNGSeed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer StartingTags;

	/** Which packs are active at start (SSOT provided by caller). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> StartingPackIds;

	/** If true, all packs are known but locked by requirements (recommended). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableAutoPackUnlocks = true;
};

UCLASS()
class BONELAW_API UStoneRunSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// === Lifecycle ===
	UFUNCTION(BlueprintCallable, Category="Stone|Run")
	void StartNewRun(const FStoneRunConfig& Config);

	// === Simulation Speed (global scalar; action components may choose to use it) ===
	UFUNCTION(BlueprintCallable, Category="Stone|Sim")
	void SetSimulationSpeed(float NewSpeed);

	UFUNCTION(BlueprintCallable, Category="Stone|Sim")
	void SetWorldTimeSpeedMultiplier(float NewWorldTimeSpeed);

	UFUNCTION(BlueprintPure, Category="Stone|Sim")
	float GetSimulationSpeed() const;

	// === Snapshot (UI convenience; not persistence) ===
	UFUNCTION(BlueprintCallable, Category="Stone|Run")
	FStoneSnapshot GetSnapshot() const { return Snapshot; }

	UPROPERTY(BlueprintAssignable, Category="Stone|Run")
	FStoneSnapshotChanged OnSnapshotChanged;

	/** Compatibility only (legacy global event panel). Per-settler actions should drive UI going forward. */
	UPROPERTY(BlueprintAssignable, Category="Stone|Run")
	FStoneEventChanged OnEventChanged;

	// === Time System (UDS Integration) ===
	UFUNCTION(BlueprintCallable, Category="Stone|Time")
	void OnSunrise();

	UFUNCTION(BlueprintCallable, Category="Stone|Time")
	void OnSunset();

	UFUNCTION(BlueprintCallable, Category="Stone|Time")
	void OnHourChanged(int32 NewHour);

	UFUNCTION(BlueprintPure, Category="Stone|Time")
	const FStoneTimeState& GetTimeState() const { return Time; }

	UFUNCTION(BlueprintPure, Category="Stone|Time")
	bool IsNight() const { return Time.bIsNight; }

	UFUNCTION(BlueprintPure, Category="Stone|Time")
	int32 GetCurrentDay() const { return Time.DayIndex; }

	// === Run Tags (global meta state; NOT mirrored onto any ASC) ===
	UFUNCTION(BlueprintCallable, Category="Stone|Run")
	void AddRunTags(const FGameplayTagContainer& TagsToAdd);

	UFUNCTION(BlueprintCallable, Category="Stone|Run")
	void RemoveRunTags(const FGameplayTagContainer& TagsToRemove);

	UFUNCTION(BlueprintPure, Category="Stone|Run")
	FGameplayTagContainer GetRunTags() const { return RunTags; }

	// === Pack Library ===
	UFUNCTION(BlueprintCallable, Category="Stone|Packs")
	void ActivatePackTemporary(FName PackId);

	UFUNCTION(BlueprintCallable, Category="Stone|Packs")
	void DeactivateTemporaryPacks();

	UFUNCTION(BlueprintCallable, Category="Stone|Packs")
	void DeactivateTemporaryPacksByIds(const TArray<FName>& PackIds);

	UFUNCTION(BlueprintPure, Category="Stone|Packs")
	const TArray<FName>& GetActivePackIds() const { return ActivePackIds; }

	// === Trace ===
	UFUNCTION(BlueprintCallable, Category="Stone|Trace")
	UStoneRunTraceBuffer* GetTraceBuffer() const { return Trace; }

	// === GAS ===
	UFUNCTION(BlueprintPure, Category="Stone|GAS")
	UAbilitySystemComponent* GetASC() const;

protected:
	virtual void Deinitialize() override;

private:
	// Cache local player PlayerState (PlayerState owns the ASC in your architecture)
	UPROPERTY()
	TWeakObjectPtr<AStonePlayerState> CachedPlayerState;

	bool EnsurePlayerStateCache();
	AStonePlayerState* GetPlayerState() const;

	// UI convenience only
	UPROPERTY()
	FStoneSnapshot Snapshot;

	UPROPERTY()
	FStoneTimeState Time;

	UPROPERTY()
	FGameplayTagContainer RunTags;

	// Simulation speed scalars
	UPROPERTY()
	float UserSimSpeed = 1.f;

	UPROPERTY()
	float WorldTimeSpeedMult = 1.f;

	// Packs
	UPROPERTY()
	TObjectPtr<UStonePackLibrary> PackLibrary;

	UPROPERTY()
	TObjectPtr<UStoneEventLibrary> EventLibrary;

	UPROPERTY()
	TArray<FName> ActivePackIds;

	UPROPERTY()
	TArray<FName> KnownPackIds;

	UPROPERTY()
	bool bAutoPackUnlocksEnabled = true;

	// Temporary packs are activated by gameplay (usually by an action) and reverted afterwards.
	UPROPERTY()
	TArray<FName> TemporaryPackIds;

	// Trace
	UPROPERTY()
	TObjectPtr<UStoneRunTraceBuffer> Trace;

	// Helpers
	void ApplyDayNightTags(bool bNowNight);
	void RebuildSnapshot();
	void BroadcastSnapshot();

	void EnsurePackLibrary(bool bPreloadAllSync);
	void EnsureEventLibrary(bool bPreloadAllSync);
	void BuildInitialEventPool(const FStoneRunConfig& Config);
	void AddEventsFromPackId(FName PackId, bool bPreloadIfPackRequests);
	void RebuildEventPoolFromActivePacks(bool bPreloadIfPackRequests);
	void DeactivatePackInternal(FName PackId);
	void TryAutoUnlockPacks();
};
