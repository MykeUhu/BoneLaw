// Copyright by MykeUhu


#include "UI/HUD/StoneLoadScreenHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/ViewModel/MVVM_LoadScreen.h"
#include "UI/Widget/StoneLoadScreenWidget.h"

void AStoneLoadScreenHUD::BeginPlay()
{
	Super::BeginPlay();
	
	LoadScreenViewModel = NewObject<UMVVM_LoadScreen>(this, LoadScreenViewModelClass);
	LoadScreenViewModel->InitializeLoadSlots();

	LoadScreenWidget = CreateWidget<UStoneLoadScreenWidget>(GetWorld(), LoadScreenWidgetClass);
	LoadScreenWidget->AddToViewport();
	LoadScreenWidget->BlueprintInitializeWidget();

	LoadScreenViewModel->LoadData();
}
