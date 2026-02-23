#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Runtime/StoneActionTypes.h"
#include "StoneSettlerActionComponent.generated.h"

class UStoneActionDefinitionData;
class UStoneRunSubsystem;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStoneSettlerActionStateChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneSettlerActionProgressChanged, float, Progress01);

/**
 * Per-Settler Action Component
 * Replaces the global ActionSubsystem approach with component-based actions.
 * Each settler can now have their own active action independently.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BONELAW_API UStoneSettlerActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStoneSettlerActionComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Start a new action for this settler
	 * @param ActionDef - Action definition data asset
	 * @return true if action started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Stone|Action")
	bool StartAction(UStoneActionDefinitionData* ActionDef);

	/**
	 * Stop the currently running action
	 * @param bForceReturnHomeEvent - whether to trigger return home event
	 */
	UFUNCTION(BlueprintCallable, Category = "Stone|Action")
	void StopCurrentAction(bool bForceReturnHomeEvent);

	/** Is this settler currently executing an action? */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	bool IsActionRunning() const { return bActionRunning; }

	/** Current phase of the action */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	EStoneActionPhase GetPhase() const { return Phase; }

	/** Overall action progress (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	float GetActionProgress01() const;

	/** Current phase progress (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	float GetPhaseProgress01() const;

	/** Display title of current action */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	FText GetActionTitleText() const;

	/** Display description of current action */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	FText GetActionDescriptionText() const;

	/** Current phase as text */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	FText GetPhaseText() const;

	/** Remaining time in seconds */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	float GetRemainingSeconds() const;

	/** Get this settler's ASC for attribute lookups */
	UAbilitySystemComponent* GetASC() const;

	/** Get elapsed time in current phase (for save/load) */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	float GetPhaseElapsed() const { return PhaseElapsedBaseSeconds; }

	/** Get total elapsed time (for save/load) */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	float GetTotalElapsed() const { return TotalElapsedBaseSeconds; }

	/** Get current action definition (for save/load) */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	UStoneActionDefinitionData* GetCurrentDef() const { return CurrentDef; }

	/** Set phase directly (for save/load resume) */
	UFUNCTION(BlueprintCallable, Category = "Stone|Action")
	void SetPhase(EStoneActionPhase NewPhase) { Phase = NewPhase; }

	/** Set elapsed times directly (for save/load resume) */
	UFUNCTION(BlueprintCallable, Category = "Stone|Action")
	void SetElapsedTimes(float PhaseElapsed, float TotalElapsed)
	{
		PhaseElapsedBaseSeconds = PhaseElapsed;
		TotalElapsedBaseSeconds = TotalElapsed;
	}

	UPROPERTY(BlueprintAssignable, Category = "Stone|Action")
	FStoneSettlerActionStateChanged OnActionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stone|Action")
	FStoneSettlerActionProgressChanged OnActionProgressChanged;

private:
	void TickAction();
	void AdvancePhaseTimeline(float AdvanceBaseSeconds);
	void EnterPhase(EStoneActionPhase NewPhase);
	void HandlePhaseAdvance();

	UStoneRunSubsystem* GetRun() const;
	float ResolveActionSpeedMult() const;
	FGameplayTag GetLegRandomEventTag(EStoneActionPhase InPhase) const;

	void ApplyRunSideEffects();
	void RemoveRunSideEffects();

private:
	FTimerHandle ActionTickHandle;

	UPROPERTY()
	TObjectPtr<UStoneActionDefinitionData> CurrentDef;

	bool bActionRunning = false;
	bool bReturnHomeQueued = false;

	EStoneActionPhase Phase = EStoneActionPhase::None;

	float BaseDurationSeconds = 0.f;
	float OutboundSeconds = 0.f;
	float ArrivalSeconds = 0.f;
	float ReturnSeconds = 0.f;
	float PhaseElapsedBaseSeconds = 0.f;
	float TotalElapsedBaseSeconds = 0.f;

	TArray<float> OutboundRandomTimes;
	TArray<float> ReturnRandomTimes;
	int32 OutboundIndex = 0;
	int32 ReturnIndex = 0;

	// What we applied to Run (for clean revert)
	FGameplayTagContainer AppliedStateTags;
	TArray<FName> ActivatedPackIds;

	FRandomStream RNG;
};
