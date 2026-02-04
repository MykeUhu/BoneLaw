#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StoneGameMode.generated.h"

class UAbilityInfo;

UCLASS()
class BONELAW_API AStoneGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStoneGameMode();
	
	UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
	TObjectPtr<UAbilityInfo> AbilityInfo;

	UPROPERTY(EditDefaultsOnly)
	FName DefaultStartPack;
	
	UFUNCTION(BlueprintCallable)
	void StartStoneRun();
};
