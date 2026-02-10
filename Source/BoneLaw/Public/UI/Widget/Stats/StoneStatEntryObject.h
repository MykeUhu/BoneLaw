#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "StoneStatEntryObject.generated.h"

UENUM(BlueprintType)
enum class EStoneStatDelta : uint8
{
	None,
	Up,
	Down
};

/**
 * ListView item object for the Statistics UI.
 * - Holds SSOT UI metadata from UAttributeInfo (name/desc/style/axis labels)
 * - Holds runtime values (value + delta) from GAS broadcasts
 */
UCLASS(BlueprintType)
class BONELAW_API UStoneStatEntryObject : public UObject
{
	GENERATED_BODY()

public:
	// SSOT identity
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag AttributeTag;

	// SSOT UI
	UPROPERTY(BlueprintReadOnly)
	FText Name;

	UPROPERTY(BlueprintReadOnly)
	FText Description;

	UPROPERTY(BlueprintReadOnly)
	EStoneAttributeDisplayStyle DisplayStyle = EStoneAttributeDisplayStyle::Default;

	// Axis-style only (optional; empty for Default)
	UPROPERTY(BlueprintReadOnly)
	FText AxisLeftLabel;

	UPROPERTY(BlueprintReadOnly)
	FText AxisRightLabel;

	UPROPERTY(BlueprintReadOnly)
	float AxisMin = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float AxisMax = 100.f;

	// Runtime value
	UPROPERTY(BlueprintReadOnly)
	float Value = 0.f;

	// Convenience UI text (pre-formatted)
	UPROPERTY(BlueprintReadOnly)
	FText ValueText;

	// Direction since last update
	UPROPERTY(BlueprintReadOnly)
	EStoneStatDelta Delta = EStoneStatDelta::None;

	// Used to compute Delta and can be useful for UI effects
	UPROPERTY(BlueprintReadOnly)
	float PreviousValue = 0.f;

	UFUNCTION(BlueprintPure, Category="Stone|Stats")
	bool IsAxis() const { return DisplayStyle == EStoneAttributeDisplayStyle::Axis; }

	void UpdateFromInfo(const FStoneAttributeInfo& Info)
	{
		PreviousValue = Value;

		AttributeTag = Info.AttributeTag;
		Name = Info.AttributeName;
		Description = Info.AttributeDescription;
		DisplayStyle = Info.DisplayStyle;

		AxisLeftLabel = Info.AxisLeftLabel;
		AxisRightLabel = Info.AxisRightLabel;
		AxisMin = Info.AxisMin;
		AxisMax = Info.AxisMax;

		Value = Info.AttributeValue;

		// Delta
		if (FMath::IsNearlyEqual(PreviousValue, Value))
		{
			Delta = EStoneStatDelta::None;
		}
		else
		{
			Delta = (Value > PreviousValue) ? EStoneStatDelta::Up : EStoneStatDelta::Down;
		}

		// ValueText formatting
		if (DisplayStyle == EStoneAttributeDisplayStyle::Axis)
		{
			const float Clamped = FMath::Clamp(Value, AxisMin, AxisMax);
			const int32 Rounded = FMath::RoundToInt(Clamped);
			const int32 MaxRounded = FMath::RoundToInt(AxisMax);
			ValueText = FText::FromString(FString::Printf(TEXT("%d / %d"), Rounded, MaxRounded));
		}
		else
		{
			// For non-axis stats: show as integer if it is close, otherwise 1 decimal.
			const float Rounded = FMath::RoundToFloat(Value);
			if (FMath::IsNearlyEqual(Value, Rounded, 0.001f))
			{
				ValueText = FText::AsNumber((int32)Rounded);
			}
			else
			{
				ValueText = FText::FromString(FString::Printf(TEXT("%.1f"), Value));
			}
		}
	}
};
