// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/StoneUserWidget.h"
#include "StoneEventPanelWidget.generated.h"

class UVerticalBox;
class UProgressBar;
class UStoneWidgetController;
class UStoneOverlayWidgetController;
class UStoneActionSubsystem;
class UStoneChoiceButtonWidget;
class UStoneCustomTextBlock;
class UStoneEventData;
struct FStoneSnapshot;

/**
 * Event panel (View) – deterministic UI refresh driven by OverlayWidgetController delegates.
 *
 * Responsibilities:
 * - show/hide event content
 * - set Title/Body
 * - rebuild choices each time an event changes (no stale buttons)
 * - optionally show progress (Action/Travel/Expedition), if available
 *
 * Blueprints should only provide layout (BindWidget) + set ChoiceButtonClass.
 */
UCLASS(Abstract, BlueprintType)
class BONELAW_API UStoneEventPanelWidget : public UStoneUserWidget
{
	GENERATED_BODY()

public:
	/** Called by owning overlay after controller is created/assigned. */
	UFUNCTION(BlueprintCallable, Category="Stone|UI")
	void SetOverlayController(UStoneWidgetController* InController);

	/** Explicit manual refresh hook (optional). Normally you don't need to call this from BP. */
	UFUNCTION(BlueprintCallable, Category="Stone|UI")
	void RefreshFromEvent(const UStoneEventData* Event);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ===== BindWidgets (custom Stone widgets) =====
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UStoneCustomTextBlock> TB_Title = nullptr;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UStoneCustomTextBlock> TB_Body = nullptr;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional, AllowPrivateAccess="true"))
	TObjectPtr<UProgressBar> PB_Progress = nullptr;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UVerticalBox> VB_Choices = nullptr;

	/** Choice button widget class (BP child of UStoneChoiceButtonWidget). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stone|UI")
	TSubclassOf<UStoneChoiceButtonWidget> ChoiceButtonClass;

	// Optional BP hooks for animation/sfx
	UFUNCTION(BlueprintImplementableEvent, Category="Stone|UI")
	void BP_OnEventShown();

	UFUNCTION(BlueprintImplementableEvent, Category="Stone|UI")
	void BP_OnEventHidden();

private:
	UPROPERTY()
	TObjectPtr<UObject> OverlayController = nullptr;

	UPROPERTY()
	TObjectPtr<UStoneActionSubsystem> ActionSubsystem = nullptr;

	UPROPERTY()
	TObjectPtr<const UStoneEventData> CurrentEvent = nullptr;

	// Delegate bindings
	UFUNCTION()
	void HandleOverlayEventChanged(const UStoneEventData* Event);

	UFUNCTION()
	void HandleOverlaySnapshotChanged(const FStoneSnapshot& Snapshot);

	UFUNCTION()
	void HandleActionStateChanged();

	UFUNCTION()
	void HandleActionProgressChanged(float Progress01);

	/** Choice button click -> applies choice on RunSubsystem. */
	UFUNCTION()
	void HandleChoiceClicked(int32 ChoiceIndex);

	void BindAll();
	void UnbindAll();

	void ClearUI();
	void RebuildChoices();
	void RefreshProgressVisual();
	
	// Helper
	UStoneOverlayWidgetController* GetOverlayController() const;
};
