// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "StoneAIController.generated.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;
class UStoneSettlerActionComponent;

UCLASS()
class BONELAW_API AStoneAIController : public AAIController
{
	GENERATED_BODY()

public:
	AStoneAIController();

	/**
	 * Convenience accessor for BT Tasks.
	 * Resolves the ActionComponent from the currently controlled Settler pawn.
	 * Preferred over casting GetPawn() manually in every BTT Blueprint.
	 * Returns nullptr if the pawn has no ActionComponent (logs a warning).
	 */
	UFUNCTION(BlueprintPure, Category="Stone|Action")
	UStoneSettlerActionComponent* GetActionComponent() const;
	
	// --- AI | Blackboard keys -----------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category="AI|Blackboard")
	FName BBKey_ActionDef = TEXT("ActionDef");

	UPROPERTY(EditDefaultsOnly, Category="AI|Blackboard")
	FName BBKey_TargetActor = TEXT("TargetActor");

	UPROPERTY(EditDefaultsOnly, Category="AI|Blackboard")
	FName BBKey_TravelLocation = TEXT("TravelLocation");

	UPROPERTY(EditDefaultsOnly, Category="AI|Blackboard")
	FName BBKey_RandomWanderLocation = TEXT("RandomWanderLocation");

	UPROPERTY(EditDefaultsOnly, Category="AI|Blackboard")
	FName BBKey_HomeLocation = TEXT("HomeLocation");

	UPROPERTY(EditDefaultsOnly, Category="AI|Blackboard")
	FName BBKey_TravelState = TEXT("TravelState");

	UFUNCTION(BlueprintCallable, Category="AI|Blackboard")
	void ResetActionBlackboardKeys();

protected:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};
