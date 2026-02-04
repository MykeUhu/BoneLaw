#pragma once

#include "CoreMinimal.h"
#include "Data/StoneStageTypes.h"
#include "GameFramework/Actor.h"
#include "StoneStageController.generated.h"

class UStoneRunSubsystem;

UCLASS()
class BONELAW_API AStoneStageController : public AActor
{
	GENERATED_BODY()

public:
	AStoneStageController();

protected:
	virtual void BeginPlay() override;

public:
	// Called when snapshot changes
	UFUNCTION()
	void HandleSnapshotChanged(const FStoneSnapshot& Snapshot);

	// Blueprint hook to actually apply visuals (UDS, fire, NPCs)
	UFUNCTION(BlueprintImplementableEvent, Category="Stone|Stage")
	void BP_ApplyStageState(const FStoneStageState& StageState);

private:
	UPROPERTY()
	TObjectPtr<UStoneRunSubsystem> Run;

	FStoneStageState MakeStageState(const FStoneSnapshot& Snap) const;
};
