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
#include "Runtime/StoneActionSubsystem.h"
#include "Data/StoneActionDefinitionData.h"

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
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[StonePlayerController] StartExploreExpedition failed: World is null"));
		return;
	}

	UStoneActionSubsystem* ActionSS = World->GetSubsystem<UStoneActionSubsystem>();
	if (!ActionSS)
	{
		UE_LOG(LogTemp, Error, TEXT("[StonePlayerController] StartExploreExpedition failed: StoneActionSubsystem missing"));
		return;
	}

	// SSOT: pack id comes from PC defaults (can be overridden per BP child).
	const FName PackId = DefaultExplorePack;
	if (PackId.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[StonePlayerController] StartExploreExpedition failed: DefaultExplorePack is None"));
		return;
	}

	// Build a transient action definition (no hardcoded content paths; caller provides runtime parameters).
	UStoneActionDefinitionData* Def = NewObject<UStoneActionDefinitionData>(this);
	Def->ActionType = EStoneActionType::Explore;
	Def->DisplayName = FText::FromString(TEXT("Explore"));
	Def->BaseDurationSeconds = FMath::Max(1.f, DurationSeconds);
	Def->BaseDurationSeconds = FMath::Max(1.f, DurationSeconds);

	// Map (Min/Max gap) -> (event count + chance). Keeps BP callsites stable.
	const float MinGap = FMath::Max(0.1f, MinEventGapSeconds);
	const float MaxGap = FMath::Max(MinGap, MaxEventGapSeconds);
	const float AvgGap = 0.5f * (MinGap + MaxGap);
	const float ExpectedEvents = Def->BaseDurationSeconds / FMath::Max(0.1f, AvgGap);

	Def->PackIdsToActivate = { PackId };

	const bool bStarted = ActionSS->StartAction(Def);
	UE_LOG(LogTemp, Log, TEXT("[StonePlayerController] StartExploreExpedition -> %s (Pack=%s, Duration=%.1fs)"),
		bStarted ? TEXT("STARTED") : TEXT("FAILED"),
		*PackId.ToString(),
		Def->BaseDurationSeconds);
}

void AStonePlayerController::StopExpedition(bool bForceReturnEvent)
{
	if (UWorld* World = GetWorld())
	{
		if (UStoneActionSubsystem* ActionSS = World->GetSubsystem<UStoneActionSubsystem>())
		{
			ActionSS->StopCurrentAction(bForceReturnEvent);
			UE_LOG(LogTemp, Log, TEXT("[StonePlayerController] StopExpedition -> StopCurrentAction(ForceReturnHome=%s)"),
				bForceReturnEvent ? TEXT("true") : TEXT("false"));
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[StonePlayerController] StopExpedition: ActionSubsystem missing (nothing to stop)"));
}

UStoneAbilitySystemComponent* AStonePlayerController::GetASC()
{
	if (StoneAbilitySystemComponent == nullptr)
	{
		StoneAbilitySystemComponent = Cast<UStoneAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return StoneAbilitySystemComponent;
}
