// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/StoneUserWidget.h"
#include "StoneActionPanelWidget.generated.h"

class UProgressBar;
class UStoneWidgetController;
class UStoneOverlayWidgetController;
class UStoneActionSubsystem;
class UStoneActionDefinitionData;
class UStoneCustomButton;
class UStoneEventData;
struct FStoneSnapshot;

/**
 * Action panel (View) – starts actions via UStoneActionSubsystem.
 *
 * Responsibilities:
 * - Provide one or more action buttons (BP layout).
 * - Enforce deterministic enabled/disabled state (no spamming).
 * - Optionally display action progress.
 *
 * Controller contract:
 * - SetOverlayController is passed a base UStoneWidgetController* (tutorial style).
 * - Internally we cast to UStoneOverlayWidgetController to access overlay-only API/delegates.
 */
UCLASS(Abstract, BlueprintType)
class BONELAW_API UStoneActionPanelWidget : public UStoneUserWidget
{
	GENERATED_BODY()

public:
	/** Called by owning overlay after controller is created/assigned. */
	UFUNCTION(BlueprintCallable, Category="Stone|UI")
	void SetOverlayController(UStoneWidgetController* InController);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ===== BindWidgets =====
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional, AllowPrivateAccess="true"))
	TObjectPtr<UProgressBar> PB_ActionProgress = nullptr;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UStoneCustomButton> Btn_StartAction = nullptr;

	// ===== Config (set in BP child defaults) =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stone|UI")
	TObjectPtr<UStoneActionDefinitionData> ActionToStart = nullptr;

private:
	UPROPERTY()
	TObjectPtr<UObject> OverlayController = nullptr;

	UPROPERTY()
	TObjectPtr<UStoneActionSubsystem> ActionSubsystem = nullptr;

	// Delegate handlers
	UFUNCTION()
	void HandleOverlaySnapshotChanged(const FStoneSnapshot& Snapshot);

	UFUNCTION()
	void HandleOverlayEventChanged(const UStoneEventData* Event);

	UFUNCTION()
	void HandleActionStateChanged();

	UFUNCTION()
	void HandleActionProgressChanged(float Progress01);

	UFUNCTION()
	void HandleStartActionClicked();

	void BindAll();
	void UnbindAll();
	void RefreshEnabledState();
	void RefreshProgressVisual();

	// Helper
	UStoneOverlayWidgetController* GetOverlayController() const;
};
