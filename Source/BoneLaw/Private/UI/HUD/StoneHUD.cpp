#include "UI/HUD/StoneHUD.h"

#include "UI/Widget/StoneUserWidget.h"
#include "UI/WidgetController/StoneOverlayWidgetController.h"
#include "Runtime/StoneRunSubsystem.h"
#include "Kismet/GameplayStatics.h"

UStoneOverlayWidgetController* AStoneHUD::GetOverlayWidgetController(const FStoneWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UStoneOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	return OverlayWidgetController;
}

void AStoneHUD::InitOverlay(
	APlayerController* PC,
	APlayerState* PS, 
	UAbilitySystemComponent* ASC, 
	UAttributeSet* AS,
	UStoneRunSubsystem* SSys)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized, please fill out BP_StoneHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class uninitialized, please fill out BP_StoneHUD"));
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UStoneUserWidget>(Widget);
	
	const FStoneWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS, SSys);
	UStoneOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);
	
	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}
