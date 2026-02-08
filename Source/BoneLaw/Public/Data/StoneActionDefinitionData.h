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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stone|Action")
	float BaseDurationSeconds = 300.f;

	// Fair RNG: geplante Slots in Base-Time (unabhängig von 1x/10x/AbilityTimeMulti)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stone|Action|Random", meta=(ClampMin="0"))
	int32 OutboundRandomCountMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stone|Action|Random", meta=(ClampMin="0"))
	int32 OutboundRandomCountMax = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stone|Action|Random", meta=(ClampMin="0.0", ClampMax="1.0"))
	float OutboundRandomChance01 = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stone|Action|Random", meta=(ClampMin="0.0", ClampMax="1.0"))
	float OutboundRandomAtMin01 = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stone|Action|Random", meta=(ClampMin="0.0", ClampMax="1.0"))
	float OutboundRandomAtMax01 = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stone|Action|Random", meta=(ClampMin="0"))
	int32 ReturnRandomCountMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stone|Action|Random", meta=(ClampMin="0"))
	int32 ReturnRandomCountMax = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stone|Action|Random", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ReturnRandomChance01 = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stone|Action|Random", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ReturnRandomAtMin01 = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stone|Action|Random", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ReturnRandomAtMax01 = 0.75f;

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
