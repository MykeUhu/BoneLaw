#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/World.h"
#include "StoneMapConfigDataAsset.generated.h"

/**
 * SSOT Map configuration used by GameModes (LoadScreen + Gameplay).
 * Stores the default map and a key->map mapping using soft references.
 */
UCLASS(BlueprintType)
class BONELAW_API UStoneMapConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Key of the default map (must exist in Maps or DefaultMap must be set). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stone|Maps")
	FName DefaultMapKey = NAME_None;

	/** Default map (optional fallback if DefaultMapKey is not present). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stone|Maps")
	TSoftObjectPtr<UWorld> DefaultMap;

	/** Map dictionary: Key -> World asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stone|Maps")
	TMap<FName, TSoftObjectPtr<UWorld>> Maps;

	/** Returns true if there is a valid world for Key. */
	UFUNCTION(BlueprintCallable, Category="Stone|Maps")
	bool TryGetMapByKey(FName Key, TSoftObjectPtr<UWorld>& OutMap) const;

	/** Returns true if a default map can be resolved (by key or fallback). */
	UFUNCTION(BlueprintCallable, Category="Stone|Maps")
	bool TryGetDefaultMap(TSoftObjectPtr<UWorld>& OutMap) const;

	/** Reverse-lookup: resolves Key by asset-name (e.g. "L_Foo"). Returns NAME_None if not found. */
	UFUNCTION(BlueprintCallable, Category="Stone|Maps")
	FName FindKeyByMapAssetName(const FString& MapAssetName) const;

	/** Lightweight validation helper (logs issues). */
	bool ValidateConfig() const;
};
