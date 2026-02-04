#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "StoneAttributeRegistry.generated.h"

UENUM(BlueprintType)
enum class EStoneAttrId : uint8
{
	// Visible stats
	Food,
	Water,
	Health,
	Morale,
	Warmth,
	Trust,

	// Culture (hidden)
	Empathy,
	Hierarchy,
	Violence,
	Spirituality,
	Innovation,
	Collectivism,
	Xenophobia,
	TabooStrictness,
	DietBalance,

	// Knowledge
	Med,
	Hunt,
	Survival,
	Craft,
	Social,
	Courage,
	Spiritual
};

USTRUCT(BlueprintType)
struct FStoneAttrBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EStoneAttrId Id = EStoneAttrId::Food;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName StableName; // for save migration & debug

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttribute Attribute;
};

UCLASS(BlueprintType)
class BONELAW_API UStoneAttributeRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	// Studio rule: registry is explicit and validated.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FStoneAttrBinding> Bindings;

	bool TryGet(EStoneAttrId Id, FGameplayAttribute& OutAttr) const;
	bool TryGetByStableName(const FName& StableName, FGameplayAttribute& OutAttr) const;

	// Convenience: build maps once (cached)
	void BuildCaches() const;

private:
	mutable bool bCachesBuilt = false;
	mutable TMap<EStoneAttrId, FGameplayAttribute> IdToAttr;
	mutable TMap<FName, FGameplayAttribute> NameToAttr;
};
