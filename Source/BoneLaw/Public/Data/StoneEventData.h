#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/StoneTypes.h"
#include "StoneEventData.generated.h"

USTRUCT(BlueprintType)
struct FStoneChoiceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ChoiceText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FStoneRequirement Requirement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EStoneChoiceLockMode LockMode = EStoneChoiceLockMode::Disabled;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FStoneOutcome> Outcomes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FStoneOutcome> FailOutcomes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FStoneScheduledEvent> Schedules;

	/** Optional icon for this choice (shown in button). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stone|UI")
	TSoftObjectPtr<UTexture2D> ChoiceIcon;

	/** If true, this choice forces a return/end (e.g. dangerous outcome). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stone|UI")
	bool bForcesReturn = false;
};

UCLASS(BlueprintType)
class BONELAW_API UStoneEventData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EventId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true))
	FText Body;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Image;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer EventTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FStoneRequirement Requirement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FStoneChoiceData> Choices;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BaseWeight = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ArcId;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		// Asset Type name is stable for AssetManager queries.
		static const FPrimaryAssetType Type(TEXT("StoneEvent"));
		return FPrimaryAssetId(Type, GetFName());
	}
};
