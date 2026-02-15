#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StoneGameMode.generated.h"

class UMVVM_LoadSlot;
class ULoadScreenSaveGame;
class USaveGame;
class UStoneCharacterClassInfo;
class UAbilityInfo;
class AStoneBaseChar;

UCLASS()
class BONELAW_API AStoneGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStoneGameMode();
	
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<UStoneCharacterClassInfo> CharacterClassInfo;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
	TObjectPtr<UAbilityInfo> AbilityInfo;
	
	// Load Save
	ULoadScreenSaveGame* RetrieveInGameSaveData();
	void SaveInGameProgressData(ULoadScreenSaveGame* SaveObject);
	void SaveWorldState(UWorld* World, const FString& DestinationMapAssetName = FString("")) const;
	void LoadWorldState(UWorld* World) const;
	
	// new from Aura Tut
	void SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex);
	ULoadScreenSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USaveGame> LoadScreenSaveGameClass;
	// end Load Save
	
	UPROPERTY(EditDefaultsOnly)
	FName DefaultStartPack;
	
	UFUNCTION(BlueprintCallable)
	void StartStoneRun();
	
	UPROPERTY(EditDefaultsOnly)
	FString DefaultMapName;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> DefaultMap;

	UPROPERTY(EditDefaultsOnly)
	FName DefaultPlayerStartTag;

	UPROPERTY(EditDefaultsOnly)
	TMap<FString, TSoftObjectPtr<UWorld>> Maps;
	
	FString GetMapNameFromMapAssetName(const FString& MapAssetName) const;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
	UPROPERTY(EditDefaultsOnly, Category="Stone|Settlers")
	TSubclassOf<AStoneBaseChar> DefaultSettlerClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Stone|Settlers")
	FName DefaultSettlerStartTag = NAME_None;
	
protected:
	virtual void BeginPlay() override;
};
