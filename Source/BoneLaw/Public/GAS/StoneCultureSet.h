#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "StoneStatSet.h"
#include "StoneCultureSet.generated.h"

UCLASS()
class BONELAW_API UStoneCultureSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category="Culture") FGameplayAttributeData Empathy;
	STONE_ATTR_ACCESSORS(UStoneCultureSet, Empathy)

	UPROPERTY(BlueprintReadOnly, Category="Culture") FGameplayAttributeData Hierarchy;
	STONE_ATTR_ACCESSORS(UStoneCultureSet, Hierarchy)

	UPROPERTY(BlueprintReadOnly, Category="Culture") FGameplayAttributeData Violence;
	STONE_ATTR_ACCESSORS(UStoneCultureSet, Violence)

	UPROPERTY(BlueprintReadOnly, Category="Culture") FGameplayAttributeData Spirituality;
	STONE_ATTR_ACCESSORS(UStoneCultureSet, Spirituality)

	UPROPERTY(BlueprintReadOnly, Category="Culture") FGameplayAttributeData Innovation;
	STONE_ATTR_ACCESSORS(UStoneCultureSet, Innovation)

	UPROPERTY(BlueprintReadOnly, Category="Culture") FGameplayAttributeData Collectivism;
	STONE_ATTR_ACCESSORS(UStoneCultureSet, Collectivism)

	UPROPERTY(BlueprintReadOnly, Category="Culture") FGameplayAttributeData Xenophobia;
	STONE_ATTR_ACCESSORS(UStoneCultureSet, Xenophobia)

	UPROPERTY(BlueprintReadOnly, Category="Culture") FGameplayAttributeData TabooStrictness;
	STONE_ATTR_ACCESSORS(UStoneCultureSet, TabooStrictness)

	// -100 plants .. +100 meat
	UPROPERTY(BlueprintReadOnly, Category="Culture") FGameplayAttributeData DietBalance;
	STONE_ATTR_ACCESSORS(UStoneCultureSet, DietBalance)
};
