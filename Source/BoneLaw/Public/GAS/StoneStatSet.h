#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "StoneStatSet.generated.h"

#define STONE_ATTR_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class BONELAW_API UStoneStatSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category="Stats")
	FGameplayAttributeData Food;
	STONE_ATTR_ACCESSORS(UStoneStatSet, Food)

	UPROPERTY(BlueprintReadOnly, Category="Stats")
	FGameplayAttributeData Water;
	STONE_ATTR_ACCESSORS(UStoneStatSet, Water)

	UPROPERTY(BlueprintReadOnly, Category="Stats")
	FGameplayAttributeData Health;
	STONE_ATTR_ACCESSORS(UStoneStatSet, Health)

	UPROPERTY(BlueprintReadOnly, Category="Stats")
	FGameplayAttributeData Morale;
	STONE_ATTR_ACCESSORS(UStoneStatSet, Morale)

	UPROPERTY(BlueprintReadOnly, Category="Stats")
	FGameplayAttributeData Warmth;
	STONE_ATTR_ACCESSORS(UStoneStatSet, Warmth)

	UPROPERTY(BlueprintReadOnly, Category="Stats")
	FGameplayAttributeData Trust;
	STONE_ATTR_ACCESSORS(UStoneStatSet, Trust)
};
