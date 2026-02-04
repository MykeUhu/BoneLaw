#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "StoneStatSet.h"
#include "StoneKnowledgeSet.generated.h"

UCLASS()
class BONELAW_API UStoneKnowledgeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category="Knowledge") FGameplayAttributeData Medicine;
	STONE_ATTR_ACCESSORS(UStoneKnowledgeSet, Medicine)

	UPROPERTY(BlueprintReadOnly, Category="Knowledge") FGameplayAttributeData Hunting;
	STONE_ATTR_ACCESSORS(UStoneKnowledgeSet, Hunting)

	UPROPERTY(BlueprintReadOnly, Category="Knowledge") FGameplayAttributeData Survival;
	STONE_ATTR_ACCESSORS(UStoneKnowledgeSet, Survival)

	UPROPERTY(BlueprintReadOnly, Category="Knowledge") FGameplayAttributeData Craft;
	STONE_ATTR_ACCESSORS(UStoneKnowledgeSet, Craft)

	UPROPERTY(BlueprintReadOnly, Category="Knowledge") FGameplayAttributeData Social;
	STONE_ATTR_ACCESSORS(UStoneKnowledgeSet, Social)

	UPROPERTY(BlueprintReadOnly, Category="Knowledge") FGameplayAttributeData Courage;
	STONE_ATTR_ACCESSORS(UStoneKnowledgeSet, Courage)

	UPROPERTY(BlueprintReadOnly, Category="Knowledge") FGameplayAttributeData Spiritual;
	STONE_ATTR_ACCESSORS(UStoneKnowledgeSet, Spiritual)
};
