// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoneSettlerScreenWidget.generated.h"

/**
 * 
 */
UCLASS()
class BONELAW_API UStoneSettlerScreenWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void BlueprintInitializeWidget();
};
