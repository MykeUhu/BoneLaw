#pragma once

#include "CoreMinimal.h"
#include "StoneGameModeBase.h"
#include "StoneGameplayGameMode.generated.h"

/**
 * GameMode for actual gameplay.
 * - Bootstraps the settler roster (creates a starter settler on new game).
 * - Spawns all settler pawns and binds autosave to OnActionFinishedNative.
 * - After every completed settler action SaveGameplayState() is called automatically.
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
