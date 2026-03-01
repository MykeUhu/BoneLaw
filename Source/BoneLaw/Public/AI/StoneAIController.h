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

protected:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};
