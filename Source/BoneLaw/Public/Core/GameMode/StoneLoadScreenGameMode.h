#pragma once

#include "CoreMinimal.h"
#include "StoneGameModeBase.h"
#include "StoneLoadScreenGameMode.generated.h"

/**
 * GameMode for Load Screen / Main Menu.
 * UI-only: no roster init, no settler bootstrap, no pawn spawn.
 */
UCLASS()
class BONELAW_API AStoneLoadScreenGameMode : public AStoneGameModeBase
{
	GENERATED_BODY()

public:
	AStoneLoadScreenGameMode();

protected:
	virtual void BeginPlay() override;
};
