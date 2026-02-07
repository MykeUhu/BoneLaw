#include "Core/StoneGameMode.h"

#include "UI/HUD/StoneHUD.h"
#include "Core/StonePlayerController.h"
#include "Runtime/StoneRunSubsystem.h"

AStoneGameMode::AStoneGameMode()
{
	// UI-only: kein Pawn/Character
	DefaultPawnClass = nullptr;

	// UI Pipeline
	PlayerControllerClass = AStonePlayerController::StaticClass();
	HUDClass = AStoneHUD::StaticClass();

}

void AStoneGameMode::StartStoneRun()
{
	UStoneRunSubsystem* Run = GetGameInstance()->GetSubsystem<UStoneRunSubsystem>();
	check(Run);

	FStoneRunConfig Config;
	if (!DefaultStartPack.IsNone())
	{
		Config.StartingPackIds.Add(DefaultStartPack);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameMode] DefaultStartPack is None. Set it in BP_StoneGameMode or start via PlayerController config."));
	}
	Config.bEnableAutoPackUnlocks = true;

	Run->StartNewRun(Config);
}
