// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StoneLoadScreenWidget.generated.h"

/**
 * 
 */
UCLASS()
class BONELAW_API UStoneLoadScreenWidget : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void BlueprintInitializeWidget();
};
