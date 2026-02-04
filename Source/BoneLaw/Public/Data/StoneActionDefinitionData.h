#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "StoneActionDefinitionData.generated.h"

UENUM(BlueprintType)
enum class EStoneActionType : uint8
{
	Travel UMETA(DisplayName="Travel"),
	Gather UMETA(DisplayName="Gather"),
	Explore UMETA(DisplayName="Explore"),
	Custom UMETA(DisplayName="Custom")
};

UCLASS(BlueprintType)
class BONELAW_API UStoneActionDefinitionData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag ActionTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EStoneActionType ActionType = EStoneActionType::Custom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	// Packs active ONLY while action runs
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FName> PackIdsToActivate;

	// Duration at SimulationSpeed=1.0
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDurationSeconds = 300.f;

	// Random pacing during legs
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float RandomMinGapSeconds = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float RandomMaxGapSeconds = 90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float RandomChance01 = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bAllowImmediateRandom = false;

	// Travel split (0..1). Default 50/50.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float OutboundShare01 = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ReturnShare01 = 0.5f;

	// Optional gating by tags (falls du das willst)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer RequiredTags;

	// Optional tags while running
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer GrantedStateTags;
};
