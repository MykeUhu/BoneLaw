#pragma once

#include "CoreMinimal.h"
#include "StoneGameModeBase.h"
#include "StoneGameplayGameMode.generated.h"

/**
 * GameMode for actual gameplay.
 * Handles settler bootstrap, roster initialization, and pawn spawn.
 */
UCLASS()
class BONELAW_API AStoneGameplayGameMode : public AStoneGameModeBase
{
	GENERATED_BODY()

public:
	AStoneGameplayGameMode();

protected:
	virtual void BeginPlay() override;

private:
	bool TryFindStarterTransform(const FName StarterTag, FTransform& OutXform) const;
};
