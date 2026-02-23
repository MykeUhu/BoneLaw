// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "StoneAttributeWidgetGroups.generated.h"

UENUM(BlueprintType)
enum class EAttributeWidgetGroups : uint8
{
	Primary		UMETA(DisplayName="Primary"),
	Secondary	UMETA(DisplayName="Secondary"),
	Culture		UMETA(DisplayName="Culture"),
	Knowledge	UMETA(DisplayName="Knowledge"),
	Axis		UMETA(DisplayName="Axis"),

	COUNT		UMETA(Hidden)
};