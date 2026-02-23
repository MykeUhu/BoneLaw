// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StoneAttributeGroupLibrary.generated.h"

UCLASS()
class BONELAW_API UStoneAttributeGroupLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns the number of usable groups (excludes COUNT). */
	UFUNCTION(BlueprintPure, Category="UI|Attributes")
	static int32 GetAttributeGroupCount();

	/** Converts group -> index (0..MaxIndex). */
	UFUNCTION(BlueprintPure, Category="UI|Attributes")
	static int32 GetAttributeGroupIndex(EAttributeWidgetGroups Group);

	/** Converts index -> group (clamped to valid range). */
	UFUNCTION(BlueprintPure, Category="UI|Attributes")
	static EAttributeWidgetGroups GetAttributeGroupByIndex(int32 Index);

	/**
	 * Computes prev/next and navigation booleans for the given current group.
	 * bWrap=false => stops at ends (HasPrev/HasNext false).
	 * bWrap=true  => wraps around (prev of first = last, next of last = first).
	 */
	UFUNCTION(BlueprintPure, Category="UI|Attributes")
	static void GetAttributeGroupNav(
		EAttributeWidgetGroups Current,
		bool bWrap,
		EAttributeWidgetGroups& OutPrev,
		bool& bHasPrev,
		EAttributeWidgetGroups& OutNext,
		bool& bHasNext,
		int32& OutIndex,
		int32& OutMaxIndex
	);

	/** Steps Current by Delta (-1 / +1 etc). Honors bWrap. */
	UFUNCTION(BlueprintPure, Category="UI|Attributes")
	static EAttributeWidgetGroups StepAttributeGroup(
		EAttributeWidgetGroups Current,
		int32 Delta,
		bool bWrap,
		bool& bOutChanged
	);
};