// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "StoneAIController.generated.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;

UCLASS()
class BONELAW_API AStoneAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AStoneAIController();

protected:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};