#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "StoneActionSubsystem.generated.h"

class UStoneRunSubsystem;
class UStoneActionDefinitionData;

UENUM(BlueprintType)
enum class EStoneActionPhase : uint8
{
	None      UMETA(DisplayName="None"),
	Outbound  UMETA(DisplayName="Outbound"),
	Arrival   UMETA(DisplayName="Arrival"),
	Return    UMETA(DisplayName="Return"),
	Completed UMETA(DisplayName="Completed")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStoneActionStateChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneActionProgressChanged, float, Progress01);

UCLASS()
class BONELAW_API UStoneActionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Start/Stop
	UFUNCTION(BlueprintCallable, Category="Stone|Action")
	bool StartAction(UStoneActionDefinitionData* ActionDef);

	UFUNCTION(BlueprintCallable, Category="Stone|Action")
	void StopCurrentAction(bool bForceReturnHomeEvent);

	// State
	UFUNCTION(BlueprintPure, Category="Stone|Action")
	bool IsActionRunning() const { return bActionRunning; }

	UFUNCTION(BlueprintPure, Category="Stone|Action")
	EStoneActionPhase GetPhase() const { return Phase; }

	// Progress
	UFUNCTION(BlueprintPure, Category="Stone|Action")
	float GetActionProgress01() const;

	UFUNCTION(BlueprintPure, Category="Stone|Action")
	float GetLegProgress01() const;

	// ===== UI-friendly getters (SSOT: CurrentDef + Phase + Time) =====
	UFUNCTION(BlueprintPure, Category="Stone|Action")
	FText GetActionTitleText() const;

	UFUNCTION(BlueprintPure, Category="Stone|Action")
	FText GetActionDescriptionText() const;

	UFUNCTION(BlueprintPure, Category="Stone|Action")
	FText GetPhaseText() const;

	UFUNCTION(BlueprintPure, Category="Stone|Action")
	float GetRemainingSeconds() const;

	UFUNCTION(BlueprintPure, Category="Stone|Action")
	bool IsPausedByGameState() const;

	// Score-based action speed multiplier (100 score = 1.0)
	UFUNCTION(BlueprintPure, Category="Stone|Action")
	float GetCurrentActionSpeedMult() const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category="Stone|Action")
	FStoneActionStateChanged OnActionStateChanged;

	UPROPERTY(BlueprintAssignable, Category="Stone|Action")
	FStoneActionProgressChanged OnActionProgressChanged;

private:
	void TickAction();
	UStoneRunSubsystem* GetRun() const;

	void ApplyActionPacks();
	void ClearActionPacks();

	void EnterPhase(EStoneActionPhase NewPhase);
	void HandlePhaseAdvance();
	void RollRandomLegEvent();
	FGameplayTag GetLegRandomEventTag(EStoneActionPhase InPhase) const;
	float ResolveActionSpeedMult() const;

private:
	FTimerHandle ActionTickHandle;

	UPROPERTY()
	TObjectPtr<UStoneActionDefinitionData> CurrentDef;

	bool bActionRunning = false;
	EStoneActionPhase Phase = EStoneActionPhase::None;

	// Timing in BASE seconds (duration at SimulationSpeed=1 and ScoreMult=1)
	float BaseDurationSeconds = 0.f;
	float OutboundSeconds = 0.f;
	float ReturnSeconds = 0.f;
	float PhaseElapsedBaseSeconds = 0.f;
	float TotalElapsedBaseSeconds = 0.f;

	// Random pacing in BASE seconds
	float NextRandomCountdownBaseSeconds = 0.f;
	float RandomMinGapSeconds = 30.f;
	float RandomMaxGapSeconds = 90.f;
	float RandomChance01 = 0.25f;
	bool bAllowImmediateRandom = false;

	// Tags applied to run while action active
	FGameplayTagContainer ActiveStateTags;

	// Temporary packs activated by this action
	TArray<FName> ActivatedPackIds;
	
		
	// Prevent random rolls in the same tick as a phase transition (avoids boundary artifacts).
	bool bSkipRandomThisTick = false;

	// Ensure we queue "return home" gate event only once per action run.
	bool bReturnHomeQueued = false;

	FRandomStream RNG;
};
