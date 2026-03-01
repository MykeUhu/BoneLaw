// Copyright by MykeUhu
// Following Aura pattern exactly

#include "UI/HUD/StoneHUD.h"

#include "UI/ViewModel/MVVM_SettlerScreen.h"
#include "UI/Widget/StoneSettlerScreenWidget.h"
#include "UI/Widget/StoneUserWidget.h"
#include "UI/WidgetController/StoneOverlayWidgetController.h"

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

void AStoneHUD::BeginPlay()
{
	Super::BeginPlay();

	// ---- UI only on owning client ----
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	checkf(SettlerScreenViewModelClass, TEXT("SettlerScreenViewModelClass not set in BP_StoneHUD"));
	checkf(SettlerScreenWidgetClass, TEXT("SettlerScreenWidgetClass not set in BP_StoneHUD"));

	// ViewModel must exist BEFORE widget tries to find it
	if (!SettlerScreenViewModel)
	{
		SettlerScreenViewModel = NewObject<UMVVM_SettlerScreen>(this, SettlerScreenViewModelClass);
		check(SettlerScreenViewModel);
	}

	// Create the widget WITH owning player (critical for GetHUD/GetOwningPlayer paths)
	if (!SettlerScreenWidget || !SettlerScreenWidget->IsInViewport())
	{
		SettlerScreenWidget = CreateWidget<UStoneSettlerScreenWidget>(PC, SettlerScreenWidgetClass);
		check(SettlerScreenWidget);

		SettlerScreenWidget->AddToViewport();

		// Keep your existing BP init trigger
		SettlerScreenWidget->BlueprintInitializeWidget();
	}
}

