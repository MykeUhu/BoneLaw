#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Data/StoneValidationTypes.h"
#include "StoneContentValidator.generated.h"

UCLASS()
class BONELAW_API UStoneContentValidator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * If RootPath is empty: validate via AssetManager PrimaryAsset scan (SSOT).
	 * If RootPath is set: validate only assets under that path via AssetRegistry filter (debug override).
	 */
	UFUNCTION(BlueprintCallable, Category="Stone|Validation")
	static TArray<FStoneValidationIssue> ValidateAllStoneEvents(const FString& RootPath = TEXT(""));

	UFUNCTION(BlueprintCallable, Category="Stone|Validation")
	static TArray<FStoneValidationIssue> ValidateAllStonePacks(const FString& RootPath = TEXT(""));

	UFUNCTION(BlueprintCallable, Category="Stone|Validation")
	static TArray<FStoneValidationIssue> ValidateAllStoneContent(
		const FString& EventsRoot = TEXT(""),
		const FString& PacksRoot  = TEXT("")
	);
};
