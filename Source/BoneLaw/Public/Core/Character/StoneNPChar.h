// Copyright by MykeUhu
#pragma once

#include "CoreMinimal.h"
#include "Core/Character/StoneBaseChar.h"
#include "StoneNPChar.generated.h"

UCLASS()
class BONELAW_API AStoneNPChar : public AStoneBaseChar
{
	GENERATED_BODY()

public:
	AStoneNPChar();

protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;
};
