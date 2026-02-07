// Copyright by MykeUhu
// Following Aura pattern exactly

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "StoneHUD.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;
class UStoneUserWidget;
class UStoneOverlayWidgetController;
struct FStoneWidgetControllerParams;

/**
 * AStoneHUD - Following Aura pattern exactly
 */
UCLASS()
class BONELAW_API AStoneHUD : public AHUD
{
	GENERATED_BODY()

public:
	UStoneOverlayWidgetController* GetOverlayWidgetController(const FStoneWidgetControllerParams& WCParams);

	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

private:
	UPROPERTY()
	TObjectPtr<UStoneUserWidget> OverlayWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UStoneUserWidget> OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<UStoneOverlayWidgetController> OverlayWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UStoneOverlayWidgetController> OverlayWidgetControllerClass;
};
