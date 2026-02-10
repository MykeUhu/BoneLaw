// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "AttributeInfo.generated.h"

UENUM(BlueprintType)
enum class EStoneAttributeDisplayStyle : uint8
{
	/** Standard stat row: Name + numeric value. */
	Default UMETA(DisplayName="Default"),

	/** Axis/Drift stat: value lives on a scale (e.g. 0..100) with left/right meaning labels. */
	Axis UMETA(DisplayName="Axis")
};

USTRUCT(BlueprintType)
struct FStoneAttributeInfo
{
	GENERATED_BODY()

	/** GameplayTag key for this attribute (SSOT). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AttributeTag = FGameplayTag();

	/** UI display name. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeName = FText();

	/** Optional icon shown next to the attribute name in the Stats UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTexture2D> AttributeIcon = nullptr;

	/** UI tooltip/description. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeDescription = FText();

	/** How this attribute should be presented in the Stats UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EStoneAttributeDisplayStyle DisplayStyle = EStoneAttributeDisplayStyle::Default;

	/** For Axis-style attributes: label meaning toward the minimum side (typically 0). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AxisLeftLabel = FText();

	/** For Axis-style attributes: label meaning toward the maximum side (typically 100). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AxisRightLabel = FText();

	/** For Axis-style attributes: min value (default 0). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AxisMin = 0.f;

	/** For Axis-style attributes: max value (default 100). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AxisMax = 100.f;

	/** Runtime value (filled by WidgetController at broadcast time). */
	UPROPERTY(BlueprintReadOnly)
	float AttributeValue = 0.f;
};

/**
 * DataAsset mapping AttributeTag -> UI metadata (name/description/presentation).
 */
UCLASS()
class BONELAW_API UAttributeInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	FStoneAttributeInfo FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound = false) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FStoneAttributeInfo> AttributeInformation;
};
