// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "StoneLoadScreenHUD.generated.h"

class UMVVM_LoadScreen;
class UStoneLoadScreenWidget;
/**
 * 
 */
UCLASS()
class BONELAW_API AStoneLoadScreenHUD : public AHUD
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget>LoadScreenWidgetClass;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UStoneLoadScreenWidget> LoadScreenWidget;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadScreen> LoadScreenViewModelClass;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMVVM_LoadScreen> LoadScreenViewModel;
	
	
protected:
	virtual void BeginPlay() override;
	
};
