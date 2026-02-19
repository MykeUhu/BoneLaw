#include "Core/GameMode/StoneLoadScreenGameMode.h"

#include "UI/HUD/StoneLoadScreenHUD.h"

AStoneLoadScreenGameMode::AStoneLoadScreenGameMode()
{
	HUDClass = AStoneLoadScreenHUD::StaticClass();
}

void AStoneLoadScreenGameMode::BeginPlay()
{
	// Safe: Base BeginPlay no longer bootstraps gameplay.
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[StoneLoadScreenGameMode] Load screen initialized (UI-only)."));
}
