#include "Core/GameMode/StoneGameplayGameMode.h"

#include "EngineUtils.h"
#include "Core/LoadScreenSaveGame.h"
#include "Core/StoneGameInstance.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/StoneRosterSubsystem.h"

AStoneGameplayGameMode::AStoneGameplayGameMode()
{
	// Inherits base setup.
}

void AStoneGameplayGameMode::BeginPlay()
{
	Super::BeginPlay();

	UStoneGameInstance* GI = Cast<UStoneGameInstance>(GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameplayGameMode] Missing StoneGameInstance."));
		return;
	}

	ULoadScreenSaveGame* Save = GetSaveSlotData(GI->LoadSlotName, GI->LoadSlotIndex);
	if (!Save)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameplayGameMode] Save is null. SlotName='%s' SlotIndex=%d"),
			*GI->LoadSlotName, GI->LoadSlotIndex);
		return;
	}

	UStoneRosterSubsystem* Roster = GetWorld() ? GetWorld()->GetSubsystem<UStoneRosterSubsystem>() : nullptr;
	if (!Roster)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameplayGameMode] Missing StoneRosterSubsystem."));
		return;
	}

	// Bootstrap: ensure at least one settler on new game
	if (Save->SavedSettlers.Num() == 0)
	{
		FSavedSettler NewSettler;
		NewSettler.SettlerId = FGuid::NewGuid();
		NewSettler.DisplayName = TEXT("Starter_Settler");

		const FName StarterTag = DefaultSettlerStartTag.IsNone() ? FName("Starter") : DefaultSettlerStartTag;
		FTransform StarterXform = FTransform::Identity;

		if (!TryFindStarterTransform(StarterTag, StarterXform))
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneGameplayGameMode] No tagged PlayerStart found. Using identity transform for starter settler."));
		}

		NewSettler.LastKnownTransform = StarterXform;
		Save->SavedSettlers.Add(NewSettler);

		const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, GI->LoadSlotName, GI->LoadSlotIndex);
		if (!bSaved)
		{
			UE_LOG(LogTemp, Error, TEXT("[StoneGameplayGameMode] SaveGameToSlot FAILED after creating starter settler. SlotName='%s' SlotIndex=%d"),
				*GI->LoadSlotName, GI->LoadSlotIndex);
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("[StoneGameplayGameMode] Created starter settler: %s"),
			*NewSettler.SettlerId.ToString(EGuidFormats::DigitsWithHyphensLower));
	}

	// Initialize roster + spawn first pawn
	Roster->InitializeRoster(Save->SavedSettlers);

	if (Save->SavedSettlers.Num() > 0)
	{
		Roster->GetOrSpawnSettlerPawn(Save->SavedSettlers[0].SettlerId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameplayGameMode] SavedSettlers still empty after bootstrap. Cannot spawn pawn."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[StoneGameplayGameMode] Gameplay initialized. Settlers=%d Slot='%s' Index=%d"),
		Save->SavedSettlers.Num(), *GI->LoadSlotName, GI->LoadSlotIndex);
}

bool AStoneGameplayGameMode::TryFindStarterTransform(const FName StarterTag, FTransform& OutXform) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// First: exact tag match (or accept any if none)
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PS = *It;
		if (!IsValid(PS))
		{
			continue;
		}

		if (StarterTag.IsNone() || PS->PlayerStartTag == StarterTag)
		{
			OutXform = PS->GetActorTransform();
			return true;
		}
	}

	// Fallback: first valid PlayerStart
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PS = *It;
		if (IsValid(PS))
		{
			OutXform = PS->GetActorTransform();
			UE_LOG(LogTemp, Warning, TEXT("[StoneGameplayGameMode] Using fallback PlayerStart (no tag match for '%s')."),
				*StarterTag.ToString());
			return true;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("[StoneGameplayGameMode] No PlayerStart found in level."));
	return false;
}
