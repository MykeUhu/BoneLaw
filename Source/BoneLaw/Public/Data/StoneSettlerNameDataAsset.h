// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StoneSettlerNameDataAsset.generated.h"

/**
 * DataAsset for settler name generation (replaces hardcoded arrays)
 * Supports two-part names: FirstName + optional Title
 * Example: "Ugga" or "Ugga Stonefist"
 */
UCLASS(BlueprintType)
class BONELAW_API UStoneSettlerNameDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Pool of first names (e.g. Ugga, Higg, Borga, Tukka, Rugg, Moga). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Names")
	TArray<FString> FirstNames;

	/** Optional pool of titles/epithets (e.g. Stonefist, Firekeeper, Hunter). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Names")
	TArray<FString> Titles;

	/** If true, generates "FirstName Title" (e.g. "Ugga Stonefist"). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Names")
	bool bUseTitles = false;

	/**
	 * Generate a random settler name from the configured pools.
	 * Uses deterministic random if you provide a seed.
	 * @param RandomStream Optional random stream for deterministic generation
	 * @return Generated name (e.g. "Ugga" or "Ugga Stonefist")
	 */
	UFUNCTION(BlueprintCallable, Category="Names")
	FString GenerateRandomNameWithStream(const FRandomStream& RandomStream) const;

	/**
	 * Generate a random settler name using global RNG.
	 * @return Generated name
	 */
	UFUNCTION(BlueprintCallable, Category="Names")
	FString GenerateRandomName() const;

	/**
	 * Validate that the data asset has at least one first name configured.
	 * @return True if valid, false if empty
	 */
	UFUNCTION(BlueprintCallable, Category="Names")
	bool IsValid() const;
};
