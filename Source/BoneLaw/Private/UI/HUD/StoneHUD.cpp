// Copyright by MykeUhu
// Following Aura pattern exactly

#include "UI/HUD/StoneHUD.h"

#include "UI/Widget/StoneUserWidget.h"
#include "UI/WidgetController/StoneOverlayWidgetController.h"
#include "UI/WidgetController/StoneStatsWidgetController.h"

UStoneOverlayWidgetController* AStoneHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UStoneOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
		
		return OverlayWidgetController;
	}
	return OverlayWidgetController;
}

UStoneStatsWidgetController* AStoneHUD::GetStatsWidgetController(const FWidgetControllerParams& WCParams)
{
	if (StatsWidgetController == nullptr)
	{
		checkf(StatsWidgetControllerClass, TEXT("Stats Widget Controller Class uninitialized, please fill out BP_StoneHUD"));
		StatsWidgetController = NewObject<UStoneStatsWidgetController>(this, StatsWidgetControllerClass);
		StatsWidgetController->SetWidgetControllerParams(WCParams);
		StatsWidgetController->BindCallbacksToDependencies();
		return StatsWidgetController;
	}
	return StatsWidgetController;
}



void AStoneHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	// ---- UI only on owning client ----
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	// Dedicated server / no viewport safety
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// Already initialized -> do nothing (prevents duplicates)
	if (OverlayWidget && OverlayWidget->IsInViewport())
	{
		return;
	}

	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized, please fill out BP_StoneHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class uninitialized, please fill out BP_StoneHUD"));

	UUserWidget* Widget = CreateWidget<UUserWidget>(PC, OverlayWidgetClass);
	OverlayWidget = Cast<UStoneUserWidget>(Widget);

	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UStoneOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	OverlayWidget->AddToViewport();
}

