// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoneLoadScreenWidget.generated.h"

/**
 * 
 */
UCLASS()
class BONELAW_API UStoneLoadScreenWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void BlueprintInitializeWidget();
};
