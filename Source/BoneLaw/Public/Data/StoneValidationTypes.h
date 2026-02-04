#pragma once

#include "CoreMinimal.h"
#include "StoneValidationTypes.generated.h"

UENUM(BlueprintType)
enum class EStoneValidationSeverity : uint8
{
	Info,
	Warning,
	Error
};

USTRUCT(BlueprintType)
struct FStoneValidationIssue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EStoneValidationSeverity Severity = EStoneValidationSeverity::Error;

	UPROPERTY(BlueprintReadOnly)
	FString AssetPath;

	UPROPERTY(BlueprintReadOnly)
	FString Message;
};
