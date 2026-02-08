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
	None,
	Outbound,
	Arrival,
	Return,
	Completed
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

	UFUNCTION(BlueprintCallable)
	bool StartAction(UStoneActionDefinitionData* ActionDef);

	UFUNCTION(BlueprintCallable)
	void StopCurrentAction(bool bForceReturnHomeEvent);

	UFUNCTION(BlueprintPure)
	bool IsActionRunning() const { return bActionRunning; }

	UFUNCTION(BlueprintPure)
	EStoneActionPhase GetPhase() const { return Phase; }

	UFUNCTION(BlueprintPure)
	float GetActionProgress01() const;

	UFUNCTION(BlueprintPure)
	float GetPhaseProgress01() const;

	UFUNCTION(BlueprintPure)
	FText GetActionTitleText() const;

	UFUNCTION(BlueprintPure)
	FText GetActionDescriptionText() const;

	UFUNCTION(BlueprintPure)
	FText GetPhaseText() const;

	UFUNCTION(BlueprintPure)
	float GetRemainingSeconds() const;

	UPROPERTY(BlueprintAssignable)
	FStoneActionStateChanged OnActionStateChanged;

	UPROPERTY(BlueprintAssignable)
	FStoneActionProgressChanged OnActionProgressChanged;

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
	float ReturnSeconds = 0.f;
	float PhaseElapsedBaseSeconds = 0.f;
	float TotalElapsedBaseSeconds = 0.f;

	TArray<float> OutboundRandomTimes;
	TArray<float> ReturnRandomTimes;
	int32 OutboundIndex = 0;
	int32 ReturnIndex = 0;

	// what we applied to Run (so we can revert cleanly)
	FGameplayTagContainer AppliedStateTags;
	TArray<FName> ActivatedPackIds;

	FRandomStream RNG;
};
