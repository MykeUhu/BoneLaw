// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "UI/CustomElements/StoneCustomButton.h"
#include "Data/StoneTypes.h"
#include "StoneChoiceButtonWidget.generated.h"

class UTexture2D;

/**
 * Small helper widget used by StoneEventPanelWidget.
 *
 * - Wraps StoneCustomButton but adds a ChoiceIndex and a typed clicked delegate.
 * - Displays possible outcomes (attribute changes, icons, return-forced indicators).
 * - Keeps Blueprint work minimal: you can create a BP child for styling only.
 */
UCLASS(Abstract, BlueprintType)
class BONELAW_API UStoneChoiceButtonWidget : public UStoneCustomButton
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoneChoiceClicked, int32, ChoiceIndex);

	/** Broadcast when the button is clicked, with the current ChoiceIndex. */
	UPROPERTY(BlueprintAssignable, Category="Stone|UI")
	FOnStoneChoiceClicked OnChoiceClicked;

	/** Set all runtime values deterministically (no PreConstruct magic). */
	UFUNCTION(BlueprintCallable, Category="Stone|UI")
	void InitChoice(int32 InChoiceIndex, const FText& InLabel, bool bInEnabled, bool bInSoftFail, const FText& InDisabledReason, 
	                const TArray<FStoneOutcome>& InOutcomes, UTexture2D* InIcon, bool bInForcesReturn);

	UFUNCTION(BlueprintPure, Category="Stone|UI")
	int32 GetChoiceIndex() const { return ChoiceIndex; }

	UFUNCTION(BlueprintPure, Category="Stone|UI")
	bool IsSoftFail() const { return bSoftFail; }

	UFUNCTION(BlueprintPure, Category="Stone|UI")
	FText GetDisabledReason() const { return DisabledReason; }

	UFUNCTION(BlueprintPure, Category="Stone|UI")
	const TArray<FStoneOutcome>& GetOutcomes() const { return Outcomes; }

	UFUNCTION(BlueprintPure, Category="Stone|UI")
	UTexture2D* GetChoiceIcon() const { return ChoiceIcon; }

	UFUNCTION(BlueprintPure, Category="Stone|UI")
	bool ForcesReturn() const { return bForcesReturn; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** BP hook to update outcome preview visuals (attributes, icons, etc.). */
	UFUNCTION(BlueprintImplementableEvent, Category="Stone|UI", meta=(DisplayName="On Outcomes Updated"))
	void BP_OnOutcomesUpdated();

private:
	UFUNCTION()
	void HandleBaseClicked();

	UPROPERTY()
	int32 ChoiceIndex = INDEX_NONE;

	UPROPERTY()
	bool bSoftFail = false;

	UPROPERTY()
	FText DisabledReason;

	UPROPERTY()
	TArray<FStoneOutcome> Outcomes;

	UPROPERTY()
	TObjectPtr<UTexture2D> ChoiceIcon = nullptr;

	UPROPERTY()
	bool bForcesReturn = false;
};
