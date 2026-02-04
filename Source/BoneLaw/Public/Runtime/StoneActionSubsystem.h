#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "StoneActionSubsystem.generated.h"

class UStoneRunSubsystem;
class UStoneActionDefinitionData;

UENUM(BlueprintType)
enum class EStoneActionPhase : uint8
{
	None UMETA(DisplayName="None"),
	Outbound UMETA(DisplayName="Outbound"),
	Arrival UMETA(DisplayName="Arrival"),
	Return UMETA(DisplayName="Return"),
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

	// Start/Stop (Blueprint passes the DataAsset reference)
	UFUNCTION(BlueprintCallable, Category="Stone|Action")
	bool StartAction(UStoneActionDefinitionData* ActionDef);

	UFUNCTION(BlueprintCallable, Category="Stone|Action")
	void StopCurrentAction(bool bForceReturnHomeEvent);

	// UI queries
	UFUNCTION(BlueprintPure, Category="Stone|Action")
	bool IsActionRunning() const { return bActionRunning; }

	UFUNCTION(BlueprintPure, Category="Stone|Action")
	EStoneActionPhase GetPhase() const { return Phase; }

	UFUNCTION(BlueprintPure, Category="Stone|Action")
	float GetActionProgress01() const;

	UFUNCTION(BlueprintPure, Category="Stone|Action")
	float GetLegProgress01() const;

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
	void RollRandomTravelEvent();

private:
	FTimerHandle ActionTickHandle;

	UPROPERTY()
	TObjectPtr<UStoneActionDefinitionData> CurrentDef;

	bool bActionRunning = false;
	EStoneActionPhase Phase = EStoneActionPhase::None;

	// Timing
	float BaseDurationSeconds = 0.f;
	float OutboundSeconds = 0.f;
	float ReturnSeconds = 0.f;
	float PhaseElapsedSeconds = 0.f;
	float TotalElapsedSeconds = 0.f;

	// Random pacing
	float NextRandomCountdownSeconds = 0.f;
	float RandomMinGapSeconds = 30.f;
	float RandomMaxGapSeconds = 90.f;
	float RandomChance01 = 0.25f;
	bool bAllowImmediateRandom = false;

	// Which temporary packs we activated (so we can clear them)
	TArray<FName> ActivatedPackIds;

	FRandomStream RNG;
};
