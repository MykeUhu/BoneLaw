// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Data/StoneTypes.h"
#include "StoneRosterSubsystem.generated.h"

class AStoneBaseChar;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStoneRosterChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStoneSettlerAssignmentChanged, FGuid, SettlerId, bool, bHasAssignment);

/**
 * UStoneRosterSubsystem - SSOT for settlers + assignments (data).
 *
 * IMPORTANT GAS RULE:
 * - Roster does NOT own GAS init.
 * - Pawn owns InitAbilityActorInfo + default attributes + startup abilities.
 * - Roster may apply saved state ONLY if it contains real data.
 */
UCLASS()
class BONELAW_API UStoneRosterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	void InitializeRoster(const TArray<FSavedSettler>& SavedSettlers);

	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	AStoneBaseChar* GetOrSpawnSettlerPawn(const FGuid& SettlerId);

	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	FSavedSettler GetSettlerInfo(const FGuid& SettlerId) const;

	UFUNCTION(BlueprintPure, Category="Stone|Roster")
	TArray<FGuid> GetAllSettlerIds() const;

	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	TArray<FGuid> GetAvailableSettlerIds() const;

	UFUNCTION(BlueprintPure, Category="Stone|Roster")
	bool HasSettler(const FGuid& SettlerId) const;

	// Assignments (unchanged)
	UFUNCTION(BlueprintCallable, Category="Stone|Assignments")
	bool AssignSettlerToTask(const FGuid& SettlerId, const FGuid& TaskActorId, const FGameplayTag& TaskTag, double DurationSeconds = 300.0);

	UFUNCTION(BlueprintCallable, Category="Stone|Assignments")
	void CompleteAssignment(const FGuid& SettlerId, bool bSuccess = true);

	UFUNCTION(BlueprintCallable, Category="Stone|Assignments")
	void CancelAssignment(const FGuid& SettlerId);

	UFUNCTION(BlueprintPure, Category="Stone|Assignments")
	bool HasActiveAssignment(const FGuid& SettlerId) const;

	UFUNCTION(BlueprintCallable, Category="Stone|Assignments")
	FSavedAssignment GetAssignment(const FGuid& SettlerId) const;

	// Persistence
	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	TArray<FSavedSettler> GatherRosterState() const;

	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	void ApplyRosterState(const TArray<FSavedSettler>& SavedSettlers);

	UPROPERTY(BlueprintAssignable, Category="Stone|Roster")
	FStoneRosterChanged OnRosterChanged;

	UPROPERTY(BlueprintAssignable, Category="Stone|Roster")
	FStoneSettlerAssignmentChanged OnSettlerAssignmentChanged;

private:
	struct FSettlerRuntimeState
	{
		FGuid SettlerId;
		FSavedSettler Data;
		TWeakObjectPtr<AStoneBaseChar> SpawnedPawn;
	};

	TArray<FSettlerRuntimeState> SettlerStates;

	FSettlerRuntimeState* FindSettlerState(const FGuid& SettlerId);
	const FSettlerRuntimeState* FindSettlerState(const FGuid& SettlerId) const;

	AStoneBaseChar* SpawnSettlerPawn(const FSavedSettler& SettlerData);

	/** Apply save state safely through pawn API (never raw GAS init here). */
	void ApplySettlerStateToPawn(AStoneBaseChar* Pawn, const FSavedSettler& SettlerData);

	FSavedSettler GatherSettlerStateFromPawn(const FGuid& SettlerId, AStoneBaseChar* Pawn) const;

	void StartSettlerNavigation(const FGuid& SettlerId, const FVector& TargetLocation);
	void OnSettlerArrival(const FGuid& SettlerId);
};
