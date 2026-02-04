#include "Core/StonePlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/StoneAbilitySystemComponent.h"
#include "Core/StoneGameMode.h"
#include "GameFramework/PlayerController.h"
#include "Core/StoneGameplayTags.h"
#include "GameFramework/Character.h"
#include "UI/HUD/StoneHUD.h"
#include "UI/Widget/StoneUserWidget.h"
#include "Runtime/StoneRunSubsystem.h"

AStonePlayerController::AStonePlayerController()
{
	bReplicates = true;
}

void AStonePlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AStonePlayerController::StartStoneRun()
{
	UStoneRunSubsystem* RunSS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UStoneRunSubsystem>() : nullptr;
	checkf(RunSS, TEXT("StoneRunSubsystem missing"));

	FStoneRunConfig Config;

	// SSOT: Prefer explicit PC override, otherwise take the GameMode default.
	FName StartPackToUse = DefaultStartPack;
	if (StartPackToUse.IsNone())
	{
		if (const AStoneGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AStoneGameMode>() : nullptr)
		{
			StartPackToUse = GM->DefaultStartPack;
		}
	}
	if (!StartPackToUse.IsNone())
	{
		Config.StartingPackIds.Add(StartPackToUse);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[StonePC] StartStoneRun: No start pack configured (PC.DefaultStartPack and GameMode.DefaultStartPack are None)."));
	}

	Config.bEnableAutoPackUnlocks = true;

	UE_LOG(LogTemp, Warning, TEXT("[StonePC] StartStoneRun: Pack=%s"), *StartPackToUse.ToString());
	RunSS->StartNewRun(Config);
}

void AStonePlayerController::SetSimSpeed(float NewSpeed)
{
	SimSpeed = FMath::Clamp(NewSpeed, 0.f, 10.f);

	if (UStoneRunSubsystem* RunSS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UStoneRunSubsystem>() : nullptr)
	{
		RunSS->SetSimulationSpeed(SimSpeed);
	}
}

void AStonePlayerController::StartExploreExpedition(float DurationSeconds, float MinEventGapSeconds, float MaxEventGapSeconds, bool bTriggerFirstEventImmediately)
{
	UStoneRunSubsystem* RunSS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UStoneRunSubsystem>() : nullptr;
	checkf(RunSS, TEXT("StoneRunSubsystem missing"));

	// SSOT: pack id comes from PC defaults (can be overridden per BP child).
	const FName PackId = DefaultExplorePack;
	RunSS->StartExploreExpedition(PackId, DurationSeconds, MinEventGapSeconds, MaxEventGapSeconds, bTriggerFirstEventImmediately);
}

void AStonePlayerController::StopExpedition(bool bForceReturnEvent)
{
	if (UStoneRunSubsystem* RunSS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UStoneRunSubsystem>() : nullptr)
	{
		RunSS->StopExpedition(bForceReturnEvent);
	}
}

UStoneAbilitySystemComponent* AStonePlayerController::GetASC()
{
	if (StoneAbilitySystemComponent == nullptr)
	{
		StoneAbilitySystemComponent = Cast<UStoneAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return StoneAbilitySystemComponent;
}