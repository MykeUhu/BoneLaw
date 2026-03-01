#include "Core/GameMode/StoneGameModeBase.h"

#include "Core/LoadScreenSaveGame.h"
#include "Core/StoneGameInstance.h"
#include "Core/GameMode/Data/StoneMapConfigDataAsset.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/StoneHUD.h"
#include "Core/StonePlayerController.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"
#include "Runtime/StoneRosterSubsystem.h"

AStoneGameModeBase::AStoneGameModeBase()
{
	// Base is UI-friendly by default (LoadScreen will override HUD)
	DefaultPawnClass = nullptr;

	PlayerControllerClass = AStonePlayerController::StaticClass();
	HUDClass = AStoneHUD::StaticClass();
}

void AStoneGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Base must never do gameplay bootstrap (settlers/roster/spawn).
	// It may validate shared config and save system prerequisites.

	if (!LoadScreenSaveGameClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameModeBase] LoadScreenSaveGameClass is not set."));
	}

	if (!MapConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameModeBase] MapConfig (UStoneMapConfigDataAsset) is not set. TravelToMap will fail."));
	}
	else
	{
		// Validate only once at runtime (no fake success logs).
		MapConfig->ValidateConfig();
	}
}

bool AStoneGameModeBase::TryResolveMapByKey(FName MapKey, TSoftObjectPtr<UWorld>& OutMap) const
{
	OutMap = nullptr;

	if (!MapConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameModeBase] TryResolveMapByKey failed: MapConfig is null."));
		return false;
	}

	if (MapKey.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneGameModeBase] TryResolveMapByKey called with None key."));
		return false;
	}

	if (!MapConfig->TryGetMapByKey(MapKey, OutMap))
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameModeBase] Map key '%s' not found (or null) in MapConfig '%s'."),
			*MapKey.ToString(), *GetNameSafe(MapConfig));
		return false;
	}

	return true;
}

bool AStoneGameModeBase::TryResolveDefaultMap(TSoftObjectPtr<UWorld>& OutMap) const
{
	OutMap = nullptr;

	if (!MapConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameModeBase] TryResolveDefaultMap failed: MapConfig is null."));
		return false;
	}

	if (!MapConfig->TryGetDefaultMap(OutMap))
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameModeBase] Default map could not be resolved from MapConfig '%s'."),
			*GetNameSafe(MapConfig));
		return false;
	}

	return true;
}

ULoadScreenSaveGame* AStoneGameModeBase::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	if (!LoadScreenSaveGameClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[GetSaveSlotData] LoadScreenSaveGameClass is null. SlotName='%s' SlotIndex=%d"), *SlotName, SlotIndex);
		return nullptr;
	}

	const bool bExists = UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex);

	USaveGame* SaveGameObject = nullptr;
	if (bExists)
	{
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);
		if (!SaveGameObject)
		{
			UE_LOG(LogTemp, Error, TEXT("[GetSaveSlotData] LoadGameFromSlot returned NULL. SlotName='%s' SlotIndex=%d"), *SlotName, SlotIndex);
			return nullptr;
		}
	}
	else
	{
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
		if (!SaveGameObject)
		{
			UE_LOG(LogTemp, Error, TEXT("[GetSaveSlotData] CreateSaveGameObject returned NULL. SaveClass='%s'"), *GetNameSafe(LoadScreenSaveGameClass));
			return nullptr;
		}
	}

	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	if (!LoadScreenSaveGame)
	{
		UE_LOG(LogTemp, Error, TEXT("[GetSaveSlotData] Cast to ULoadScreenSaveGame FAILED. ObjClass='%s'"),
			*GetNameSafe(SaveGameObject->GetClass()));
		return nullptr;
	}

	return LoadScreenSaveGame;
}

void AStoneGameModeBase::DeleteSlot(const FString& SlotName, int32 SlotIndex)
{
	const bool bExists = UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex);
	if (!bExists)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Save] DeleteSlot aborted: slot does not exist. Name='%s' Index=%d"), *SlotName, SlotIndex);
		return;
	}

	const bool bDeleted = UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);
	if (!bDeleted)
	{
		UE_LOG(LogTemp, Error, TEXT("[Save] DeleteGameInSlot FAILED. Name='%s' Index=%d"), *SlotName, SlotIndex);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Save] DeleteGameInSlot SUCCESS. Name='%s' Index=%d"), *SlotName, SlotIndex);
}

FString AStoneGameModeBase::GetMapNameFromMapAssetName(const FString& MapAssetName) const
{
	if (!MapConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameModeBase] GetMapNameFromMapAssetName failed: MapConfig is null."));
		return FString();
	}

	const FName Key = MapConfig->FindKeyByMapAssetName(MapAssetName);
	return Key.IsNone() ? FString() : Key.ToString();
}

AActor* AStoneGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	// ChoosePlayerStart is shared, but must be robust (no assumptions).
	const UStoneGameInstance* StoneGI = Cast<UStoneGameInstance>(GetGameInstance());
	const FName DesiredTag = (StoneGI && !StoneGI->PlayerStartTag.IsNone()) ? StoneGI->PlayerStartTag : DefaultPlayerStartTag;

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);

	if (Actors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneGameModeBase] ChoosePlayerStart: No APlayerStart found in level."));
		return nullptr;
	}

	// If no tag requested -> first start.
	if (DesiredTag.IsNone())
	{
		return Actors[0];
	}

	for (AActor* Actor : Actors)
	{
		APlayerStart* PS = Cast<APlayerStart>(Actor);
		if (!IsValid(PS))
		{
			continue;
		}

		if (PS->PlayerStartTag == DesiredTag)
		{
			return PS;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[StoneGameModeBase] ChoosePlayerStart: No PlayerStart with tag '%s' found. Using first."),
		*DesiredTag.ToString());
	return Actors[0];
}

void AStoneGameModeBase::SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex)
{
	if (!LoadSlot)
	{
		UE_LOG(LogTemp, Error, TEXT("[SaveSlotData] LoadSlot is null."));
		return;
	}

	if (!LoadScreenSaveGameClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[SaveSlotData] LoadScreenSaveGameClass is null."));
		return;
	}

	const FString SlotName = LoadSlot->GetLoadSlotName();
	if (SlotName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[SaveSlotData] SlotName is empty (MVVM_LoadSlot)."));
		return;
	}

	// Replace existing slot if present
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		const bool bDeleted = UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);
		if (!bDeleted)
		{
			UE_LOG(LogTemp, Error, TEXT("[SaveSlotData] Failed to delete existing slot. Name='%s' Index=%d"), *SlotName, SlotIndex);
			return;
		}
	}

	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	if (!SaveGameObject)
	{
		UE_LOG(LogTemp, Error, TEXT("[SaveSlotData] CreateSaveGameObject returned NULL. SaveClass='%s'"), *GetNameSafe(LoadScreenSaveGameClass));
		return;
	}

	ULoadScreenSaveGame* Save = Cast<ULoadScreenSaveGame>(SaveGameObject);
	if (!Save)
	{
		UE_LOG(LogTemp, Error, TEXT("[SaveSlotData] Cast to ULoadScreenSaveGame FAILED. ObjClass='%s'"), *GetNameSafe(SaveGameObject->GetClass()));
		return;
	}

	Save->PlayerName = LoadSlot->GetPlayerName();
	Save->MapName = LoadSlot->GetMapName();
	Save->SaveSlotStatus = Taken;

	const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, SlotName, SlotIndex);
	if (!bSaved)
	{
		UE_LOG(LogTemp, Error, TEXT("[SaveSlotData] SaveGameToSlot FAILED. Name='%s' Index=%d"), *SlotName, SlotIndex);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SaveSlotData] SaveGameToSlot SUCCESS. Name='%s' Index=%d MapKey='%s'"),
		*SlotName, SlotIndex, *Save->MapName);
}

void AStoneGameModeBase::SaveGameplayState()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneGameModeBase] SaveGameplayState: called without authority. Ignoring."));
		return;
	}

	UStoneGameInstance* GI = Cast<UStoneGameInstance>(GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameModeBase] SaveGameplayState: StoneGameInstance missing."));
		return;
	}

	const FString SlotName = GI->LoadSlotName;
	const int32 SlotIndex = GI->LoadSlotIndex;

	if (SlotName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameModeBase] SaveGameplayState: LoadSlotName is empty. Aborting save."));
		return;
	}

	// Load existing save so we preserve all fields we are not overwriting.
	ULoadScreenSaveGame* Save = GetSaveSlotData(SlotName, SlotIndex);
	if (!Save)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameModeBase] SaveGameplayState: GetSaveSlotData returned null. SlotName='%s' SlotIndex=%d"),
			*SlotName, SlotIndex);
		return;
	}

	// --- Settler Roster ---
	UStoneRosterSubsystem* Roster = GetWorld() ? GetWorld()->GetSubsystem<UStoneRosterSubsystem>() : nullptr;
	if (Roster)
	{
		Save->SavedSettlers = Roster->GatherRosterState();
		UE_LOG(LogTemp, Log, TEXT("[StoneGameModeBase] SaveGameplayState: gathered %d settlers."), Save->SavedSettlers.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneGameModeBase] SaveGameplayState: StoneRosterSubsystem not found. Settler state not saved."));
	}

	// --- Run State (DEPRECATED) ---
	// Intentionally NOT saved anymore.
	// The ActionComponent system is the new SSOT for gameplay flow/state.

	// --- Write to disk ---
	const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, SlotName, SlotIndex);
	if (bSaved)
	{
		UE_LOG(LogTemp, Log, TEXT("[StoneGameModeBase] SaveGameplayState: SUCCESS. SlotName='%s' SlotIndex=%d Settlers=%d"),
			*SlotName, SlotIndex, Save->SavedSettlers.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneGameModeBase] SaveGameplayState: SaveGameToSlot FAILED. SlotName='%s' SlotIndex=%d"),
			*SlotName, SlotIndex);
	}
}

void AStoneGameModeBase::TravelToMap(UMVVM_LoadSlot* Slot)
{
	if (!Slot)
	{
		UE_LOG(LogTemp, Error, TEXT("[TravelToMap] Slot is null."));
		return;
	}

	UStoneGameInstance* GI = Cast<UStoneGameInstance>(GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("[TravelToMap] StoneGameInstance missing."));
		return;
	}

	const FString SlotName = Slot->GetLoadSlotName();
	const int32 SlotIndex = Slot->SlotIndex;

	if (SlotName.IsEmpty() || SlotIndex < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[TravelToMap] Invalid slot selection. SlotName='%s' SlotIndex=%d"),
			*SlotName, SlotIndex);
		return;
	}

	// SSOT: set active slot BEFORE traveling (no silent fallback)
	GI->LoadSlotName = SlotName;
	GI->LoadSlotIndex = SlotIndex;

	UE_LOG(LogTemp, Log, TEXT("[TravelToMap] Active slot set -> Name='%s' Index=%d"), *GI->LoadSlotName, GI->LoadSlotIndex);

	const FString MapKeyStr = Slot->GetMapName();
	const FName MapKey = MapKeyStr.IsEmpty() ? NAME_None : FName(*MapKeyStr);

	TSoftObjectPtr<UWorld> MapToOpen;
	if (MapKey.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[TravelToMap] Slot MapName is empty. Trying default map."));
		if (!TryResolveDefaultMap(MapToOpen))
		{
			return;
		}
	}
	else
	{
		if (!TryResolveMapByKey(MapKey, MapToOpen))
		{
			return;
		}
	}

	if (MapToOpen.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("[TravelToMap] Resolved map is null (MapKey='%s')."), *MapKey.ToString());
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, MapToOpen);
	UE_LOG(LogTemp, Log, TEXT("[TravelToMap] OpenLevelBySoftObjectPtr requested. MapKey='%s' Asset='%s'"),
		*MapKey.ToString(),
		*MapToOpen.ToSoftObjectPath().ToString());
}
