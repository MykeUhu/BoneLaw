#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StoneGameModeBase.generated.h"

class UAttributeInfo;
class UMVVM_LoadSlot;
class ULoadScreenSaveGame;
class USaveGame;
class UStoneCharacterClassInfo;
class UAbilityInfo;
class AStoneBaseChar;
class UStoneMapConfigDataAsset;
class UStoneSettlerNameDataAsset;

UCLASS()
class BONELAW_API AStoneGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStoneGameModeBase();

	// -----------------------------
	// Aura-pattern shared assets
	// -----------------------------
	UPROPERTY(EditDefaultsOnly, Category="Character Class Defaults")
	TObjectPtr<UStoneCharacterClassInfo> CharacterClassInfo;

	UPROPERTY(EditDefaultsOnly, Category="Ability Info")
	TObjectPtr<UAbilityInfo> AbilityInfo;
	
	UPROPERTY(EditDefaultsOnly, Category="Attribute Info")
	TObjectPtr<UAttributeInfo> AttributeInfo;

	// -----------------------------
	// Save / Load (Aura-style)
	// -----------------------------
	void SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex);
	ULoadScreenSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const;
	static void DeleteSlot(const FString& SlotName, int32 SlotIndex);

	UPROPERTY(EditDefaultsOnly, Category="Stone|Save")
	TSubclassOf<USaveGame> LoadScreenSaveGameClass;

	// -----------------------------
	// Maps (SSOT DataAsset)
	// -----------------------------
	UPROPERTY(EditDefaultsOnly, Category="Stone|Maps")
	TObjectPtr<UStoneMapConfigDataAsset> MapConfig;

	/** Opens the map referenced by the load slot map key. */
	void TravelToMap(UMVVM_LoadSlot* Slot);

	/** Reverse lookup (asset name -> map key as string for legacy callers). */
	FString GetMapNameFromMapAssetName(const FString& MapAssetName) const;

	// -----------------------------
	// Run / Start defaults (used by gameplay systems)
	// Note: These are *defaults only*. They do not trigger gameplay initialization in Base anymore.
	// -----------------------------
	UPROPERTY(EditDefaultsOnly, Category="Stone|Run")
	FName DefaultStartPack = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category="Stone|Spawn")
	FName DefaultPlayerStartTag = NAME_None;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// -----------------------------
	// Settler defaults (gameplay mode uses them)
	// -----------------------------
	UPROPERTY(EditDefaultsOnly, Category="Stone|Settlers")
	TSubclassOf<AStoneBaseChar> DefaultSettlerClass;

	UPROPERTY(EditDefaultsOnly, Category="Stone|Settlers")
	FName DefaultSettlerStartTag = NAME_None;

	/** DataAsset for generating settler names (replaces hardcoded arrays). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stone|Settlers")
	TObjectPtr<UStoneSettlerNameDataAsset> SettlerNameDataAsset;

protected:
	virtual void BeginPlay() override;

	/** Validated map resolve helper. */
	bool TryResolveMapByKey(FName MapKey, TSoftObjectPtr<UWorld>& OutMap) const;

	/** Validated default map resolve helper. */
	bool TryResolveDefaultMap(TSoftObjectPtr<UWorld>& OutMap) const;
};
