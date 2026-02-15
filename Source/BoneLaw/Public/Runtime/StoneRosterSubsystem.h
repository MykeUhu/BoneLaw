// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Data/StoneTypes.h"
#include "StoneRosterSubsystem.generated.h"

class AStoneBaseChar;
class UStoneAbilitySystemComponent;
class UStoneAttributeSet;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStoneRosterChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStoneSettlerAssignmentChanged, FGuid, SettlerId, bool, bHasAssignment);

/**
 * UStoneRosterSubsystem - Authoritative owner of settler roster + assignments
 * 
 * ARCHITECTURE (SSOT):
 * - Single source of truth for all settlers and their assignments
 * - ID-driven: settlers identified by FGuid, never by pointers
 * - NO saved ASC pointers: ASC is runtime-only, reconstructed on load
 * - UI/Actors issue commands via this subsystem, never hold authority
 * 
 * RESPONSIBILITY:
 * - Manage settler roster (spawn, despawn, lookup by ID)
 * - Manage assignments (assign task, track progress, handle completion)
 * - Provide runtime APIs for UI (get settler info, list available settlers)
 * - Persist/restore settler state via SaveGame (data-only, no pointers)
 * 
 * INTEGRATION:
 * - Works with StoneRunSubsystem for event context routing
 * - Works with StoneActionSubsystem for real-time action tracking
 * - Settlers have individual ASC instances (not shared with PlayerState)
 * 
 * USAGE:
 * - UI: Get available settlers -> AssignSettlerToTask(SettlerId, TaskActorId, TaskTag)
 * - Save/Load: GatherRosterState() / ApplyRosterState(SavedSettlers)
 * - Runtime: GetSettlerById(SettlerId) -> SettlerInfo with live ASC lookup
 */
UCLASS()
class BONELAW_API UStoneRosterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// ROSTER MANAGEMENT
	// ========================================================================
	
	/** Initialize roster with given saved settlers (typically from SaveGame). */
	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	void InitializeRoster(const TArray<FSavedSettler>& SavedSettlers);
	
	/** Spawn or get existing settler pawn for given settler ID. */
	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	AStoneBaseChar* GetOrSpawnSettlerPawn(const FGuid& SettlerId);
	
	/** Get settler info by ID (runtime lookup, includes live ASC values). */
	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	FSavedSettler GetSettlerInfo(const FGuid& SettlerId) const;
	
	/** Get all settler IDs. */
	UFUNCTION(BlueprintPure, Category="Stone|Roster")
	TArray<FGuid> GetAllSettlerIds() const;
	
	/** Get settlers available for assignment (not currently assigned). */
	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	TArray<FGuid> GetAvailableSettlerIds() const;
	
	/** Returns true if settler exists in roster. */
	UFUNCTION(BlueprintPure, Category="Stone|Roster")
	bool HasSettler(const FGuid& SettlerId) const;
	
	// ========================================================================
	// ASSIGNMENT MANAGEMENT (COMMANDER LOOP)
	// ========================================================================
	
	/** Assign settler to task. Returns true if successful. */
	UFUNCTION(BlueprintCallable, Category="Stone|Assignments")
	bool AssignSettlerToTask(const FGuid& SettlerId, const FGuid& TaskActorId, const FGameplayTag& TaskTag, double DurationSeconds = 300.0);
	
	/** Complete assignment and return settler to home. */
	UFUNCTION(BlueprintCallable, Category="Stone|Assignments")
	void CompleteAssignment(const FGuid& SettlerId, bool bSuccess = true);
	
	/** Cancel assignment (early abort). */
	UFUNCTION(BlueprintCallable, Category="Stone|Assignments")
	void CancelAssignment(const FGuid& SettlerId);
	
	/** Returns true if settler has active assignment. */
	UFUNCTION(BlueprintPure, Category="Stone|Assignments")
	bool HasActiveAssignment(const FGuid& SettlerId) const;
	
	/** Get assignment info for settler (if any). */
	UFUNCTION(BlueprintCallable, Category="Stone|Assignments")
	FSavedAssignment GetAssignment(const FGuid& SettlerId) const;
	
	// ========================================================================
	// RUNTIME LOOKUPS (for UI display, non-authoritative)
	// ========================================================================
	
	/** Get ASC for settler (runtime-only, nullptr if settler not spawned). */
	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	UStoneAbilitySystemComponent* GetSettlerASC(const FGuid& SettlerId) const;
	
	/** Get attribute snapshot for UI display. */
	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	TArray<FSavedAttribute> GetSettlerAttributesSnapshot(const FGuid& SettlerId) const;
	
	// ========================================================================
	// PERSISTENCE (Save/Load integration)
	// ========================================================================
	
	/** Gather current roster state for saving (data-only, no pointers). */
	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	TArray<FSavedSettler> GatherRosterState() const;
	
	/** Apply saved roster state (from SaveGame load). */
	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	void ApplyRosterState(const TArray<FSavedSettler>& SavedSettlers);
	
	// ========================================================================
	// DELEGATES
	// ========================================================================
	
	UPROPERTY(BlueprintAssignable, Category="Stone|Roster")
	FStoneRosterChanged OnRosterChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Stone|Roster")
	FStoneSettlerAssignmentChanged OnSettlerAssignmentChanged;

private:
	// Internal settler tracking (runtime state)
	struct FSettlerRuntimeState
	{
		FGuid SettlerId;
		FSavedSettler Data; // Authoritative data
		TWeakObjectPtr<AStoneBaseChar> SpawnedPawn; // Runtime-only reference
	};
	
	TArray<FSettlerRuntimeState> SettlerStates;
	
	// Helper: find settler state by ID
	FSettlerRuntimeState* FindSettlerState(const FGuid& SettlerId);
	const FSettlerRuntimeState* FindSettlerState(const FGuid& SettlerId) const;
	
	// Spawn settler pawn at transform
	AStoneBaseChar* SpawnSettlerPawn(const FSavedSettler& SettlerData);
	
	// Apply saved state to existing pawn (ASC init, attributes, tags, abilities)
	void ApplySettlerStateToPawn(AStoneBaseChar* Pawn, const FSavedSettler& SettlerData);
	
	// Gather settler state from pawn (for saving)
	FSavedSettler GatherSettlerStateFromPawn(const FGuid& SettlerId, AStoneBaseChar* Pawn) const;
	
	// Navigation/movement helpers (stub for now - adjust based on actual movement system)
	void StartSettlerNavigation(const FGuid& SettlerId, const FVector& TargetLocation);
	void OnSettlerArrival(const FGuid& SettlerId);
};
