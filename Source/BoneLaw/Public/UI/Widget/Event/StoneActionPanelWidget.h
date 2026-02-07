// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/StoneUserWidget.h"
#include "StoneActionPanelWidget.generated.h"

class UStoneCustomTextBlock;
class UProgressBar;
class UTextBlock;

class UStoneWidgetController;
class UStoneOverlayWidgetController;

class UStoneActionSubsystem;
class UStoneActionDefinitionData;
struct FStoneSnapshot;
class UStoneEventData;

class UStoneCustomButton;

UCLASS()
class BONELAW_API UStoneActionPanelWidget : public UStoneUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category="Stone|UI")
	void SetOverlayController(UStoneWidgetController* InController);

protected:
	// Optional action preset to start (set in BP defaults)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stone|Action")
	TObjectPtr<UStoneActionDefinitionData> ActionToStart;

	// === Bound widgets (create them in WBP_ActionPanel with EXACT names) ===
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> PB_ActionProgress;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UStoneCustomButton> Btn_StartAction;

	// NEW: informational text blocks
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UStoneCustomTextBlock> TB_ActionTitle;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UStoneCustomTextBlock> TB_ActionSubtitle;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UStoneCustomTextBlock> TB_ActionETA;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UStoneCustomTextBlock> TB_ActionStatus;

private:
	void BindAll();
	void UnbindAll();

	UFUNCTION()
	void HandleOverlaySnapshotChanged(const FStoneSnapshot& Snapshot);

	UFUNCTION()
	void HandleOverlayEventChanged(const UStoneEventData* Event);

	UFUNCTION()
	void HandleActionStateChanged();

	UFUNCTION()
	void HandleActionProgressChanged(float Progress01);

	void RefreshEnabledState();
	void RefreshProgressVisual();
	void RefreshInfoVisual();

	UFUNCTION()
	void HandleStartActionClicked();

	UStoneOverlayWidgetController* GetOverlayController() const;

private:
	UPROPERTY()
	TObjectPtr<UStoneWidgetController> OverlayController;

	UPROPERTY()
	TObjectPtr<UStoneActionSubsystem> ActionSubsystem;
};
