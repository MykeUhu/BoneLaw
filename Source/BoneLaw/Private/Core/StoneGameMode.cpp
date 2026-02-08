#include "Core/StoneGameMode.h"

#include "EngineUtils.h"
#include "Core/StoneGameInstance.h"
#include "Core/LoadScreenSaveGame.h"
#include "UI/HUD/StoneHUD.h"
#include "Core/StonePlayerController.h"
#include "Core/Interaction/SaveInterface.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/StoneRunSubsystem.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

AStoneGameMode::AStoneGameMode()
{
	// UI-only: kein Pawn/Character
	DefaultPawnClass = nullptr;

	// UI Pipeline
	PlayerControllerClass = AStonePlayerController::StaticClass();
	HUDClass = AStoneHUD::StaticClass();

}

// Load Save
ULoadScreenSaveGame* AStoneGameMode::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	USaveGame* SaveGameObject = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);
	}
	else
	{
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	}
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	return LoadScreenSaveGame;
}

ULoadScreenSaveGame* AStoneGameMode::RetrieveInGameSaveData()
{
	UStoneGameInstance* StoneGameInstance = Cast<UStoneGameInstance>(GetGameInstance());

	const FString InGameLoadSlotName = StoneGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = StoneGameInstance->LoadSlotIndex;

	return GetSaveSlotData(InGameLoadSlotName, InGameLoadSlotIndex);
}

void AStoneGameMode::SaveInGameProgressData(ULoadScreenSaveGame* SaveObject)
{
	UStoneGameInstance* StoneGameInstance = Cast<UStoneGameInstance>(GetGameInstance());

	const FString InGameLoadSlotName = StoneGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = StoneGameInstance->LoadSlotIndex;
	StoneGameInstance->PlayerStartTag = SaveObject->PlayerStartTag;

	UGameplayStatics::SaveGameToSlot(SaveObject, InGameLoadSlotName, InGameLoadSlotIndex);
}

void AStoneGameMode::SaveWorldState(UWorld* World, const FString& DestinationMapAssetName) const
{FString WorldName = World->GetMapName();
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	UStoneGameInstance* StoneGI = Cast<UStoneGameInstance>(GetGameInstance());
	check(StoneGI);

	if (ULoadScreenSaveGame* SaveGame = GetSaveSlotData(StoneGI->LoadSlotName, StoneGI->LoadSlotIndex))
	{
		if (DestinationMapAssetName != FString(""))
		{
			SaveGame->MapAssetName = DestinationMapAssetName;
			SaveGame->MapName = GetMapNameFromMapAssetName(DestinationMapAssetName);
		}
		
		if (!SaveGame->HasMap(WorldName))
		{
			FSavedMap NewSavedMap;
			NewSavedMap.MapAssetName = WorldName;
			SaveGame->SavedMaps.Add(NewSavedMap);
		}

		FSavedMap SavedMap = SaveGame->GetSavedMapWithMapName(WorldName);
		SavedMap.SavedActors.Empty(); // clear it out, we'll fill it in with "actors"

		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;

			if (!IsValid(Actor) || !Actor->Implements<USaveInterface>()) continue;

			FSavedActor SavedActor;
			SavedActor.ActorName = Actor->GetFName();
			SavedActor.Transform = Actor->GetTransform();

			FMemoryWriter MemoryWriter(SavedActor.Bytes);

			FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
			Archive.ArIsSaveGame = true;

			Actor->Serialize(Archive);

			SavedMap.SavedActors.AddUnique(SavedActor);
		}

		for (FSavedMap& MapToReplace : SaveGame->SavedMaps)
		{
			if (MapToReplace.MapAssetName == WorldName)
			{
				MapToReplace = SavedMap;
			}
		}
		UGameplayStatics::SaveGameToSlot(SaveGame, StoneGI->LoadSlotName, StoneGI->LoadSlotIndex);
	}
}

void AStoneGameMode::LoadWorldState(UWorld* World) const
{
	FString WorldName = World->GetMapName();
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	UStoneGameInstance* StoneGI = Cast<UStoneGameInstance>(GetGameInstance());
	check(StoneGI);

	if (UGameplayStatics::DoesSaveGameExist(StoneGI->LoadSlotName, StoneGI->LoadSlotIndex))
	{

		ULoadScreenSaveGame* SaveGame = Cast<ULoadScreenSaveGame>(UGameplayStatics::LoadGameFromSlot(StoneGI->LoadSlotName, StoneGI->LoadSlotIndex));
		if (SaveGame == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load slot"));
			return;
		}
		
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;

			if (!Actor->Implements<USaveInterface>()) continue;

			for (FSavedActor SavedActor : SaveGame->GetSavedMapWithMapName(WorldName).SavedActors)
			{
				if (SavedActor.ActorName == Actor->GetFName())
				{
					if (ISaveInterface::Execute_ShouldLoadTransform(Actor))
					{
						Actor->SetActorTransform(SavedActor.Transform);
					}

					FMemoryReader MemoryReader(SavedActor.Bytes);

					FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
					Archive.ArIsSaveGame = true;
					Actor->Serialize(Archive); // converts binary bytes back into variables

					ISaveInterface::Execute_LoadActor(Actor);
				}
			}
		}
	}
}
// end Load Save


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

FString AStoneGameMode::GetMapNameFromMapAssetName(const FString& MapAssetName) const
{
	for (auto& Map : Maps)
	{
		if (Map.Value.ToSoftObjectPath().GetAssetName() == MapAssetName)
		{
			return Map.Key;
		}
	}
	return FString();
}

AActor* AStoneGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	UStoneGameInstance* StoneGameInstance = Cast<UStoneGameInstance>(GetGameInstance());
	
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);
	if (Actors.Num() > 0)
	{
		AActor* SelectedActor = Actors[0];
		for (AActor* Actor : Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				if (PlayerStart->PlayerStartTag == StoneGameInstance->PlayerStartTag)
				{
					SelectedActor = PlayerStart;
					break;
				}
			}
		}
		return SelectedActor;
	}
	return nullptr;
}

void AStoneGameMode::BeginPlay()
{
	Super::BeginPlay();
	Maps.Add(DefaultMapName, DefaultMap);
}
