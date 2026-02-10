// Copyright by MykeUhu
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "StoneInputConfig.generated.h"

class UInputAction;

/**
 * Input Action + GameplayTag binding
 * Matches Aura pattern for ability/action inputs
 */
USTRUCT(BlueprintType)
struct FStoneInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();
};

/**
 * Stone Input Configuration DataAsset
 * 
 * Defines all input actions and their associated gameplay tags.
 * Used by PlayerController to bind Enhanced Input to gameplay logic.
 */
UCLASS()
class BONELAW_API UStoneInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// Find an InputAction by its tag
	const UInputAction* FindActionByTag(const FGameplayTag& InputTag) const;

	// All input actions with tags (for ability-style inputs)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stone|Input")
	TArray<FStoneInputAction> AbilityInputActions;
};
