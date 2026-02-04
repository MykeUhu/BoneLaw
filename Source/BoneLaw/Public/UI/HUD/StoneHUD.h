#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "StoneHUD.generated.h"


class UStoneRunSubsystem;
class UAttributeSet;
class UAbilitySystemComponent;
class UStoneUserWidget;
class UStoneOverlayWidgetController;

struct FStoneWidgetControllerParams;

UCLASS()
class BONELAW_API AStoneHUD : public AHUD
{
	GENERATED_BODY()

public:
	UStoneOverlayWidgetController* GetOverlayWidgetController(const FStoneWidgetControllerParams& WCParams);
	void InitOverlay(
		APlayerController* PC,
		APlayerState* PS, 
		UAbilitySystemComponent* ASC, 
		UAttributeSet* AS,
		UStoneRunSubsystem* SSys);
	
protected:


private:
	UPROPERTY()
	TObjectPtr<UStoneUserWidget> OverlayWidget;
	
	UPROPERTY(EditAnywhere, Category="Stone|UI")
	TSubclassOf<UStoneUserWidget> OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<UStoneOverlayWidgetController> OverlayWidgetController;
	
	UPROPERTY(EditAnywhere, Category="Stone|UI")
	TSubclassOf<UStoneOverlayWidgetController> OverlayWidgetControllerClass;
};
