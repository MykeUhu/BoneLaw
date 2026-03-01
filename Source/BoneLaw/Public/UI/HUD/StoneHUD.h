// Copyright by MykeUhu
// Following Aura pattern exactly

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "StoneHUD.generated.h"

class UMVVM_SettlerScreen;
class UStoneSettlerScreenWidget;
class UAttributeSet;
class UAbilitySystemComponent;
class UStoneUserWidget;
class UStoneOverlayWidgetController;
class UStoneStatsWidgetController;
struct FWidgetControllerParams;

UCLASS()
class BONELAW_API AStoneHUD : public AHUD
{
	GENERATED_BODY()

public:
	UStoneOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);
	UStoneStatsWidgetController* GetStatsWidgetController(const FWidgetControllerParams& WCParams);

	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);
	
	// MVVM
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget>SettlerScreenWidgetClass;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UStoneSettlerScreenWidget> SettlerScreenWidget;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_SettlerScreen> SettlerScreenViewModelClass;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMVVM_SettlerScreen> SettlerScreenViewModel;
	
protected:
	virtual void BeginPlay() override;

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
