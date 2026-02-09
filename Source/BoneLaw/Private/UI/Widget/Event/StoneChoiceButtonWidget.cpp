// Copyright by MykeUhu

#include "UI/Widget/Event/StoneChoiceButtonWidget.h"

void UStoneChoiceButtonWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Avoid stacking bindings if the widget is reconstructed.
    OnClicked.RemoveDynamic(this, &UStoneChoiceButtonWidget::HandleBaseClicked);
    OnClicked.AddDynamic(this, &UStoneChoiceButtonWidget::HandleBaseClicked);
}

void UStoneChoiceButtonWidget::NativeDestruct()
{
    OnClicked.RemoveDynamic(this, &UStoneChoiceButtonWidget::HandleBaseClicked);
    Super::NativeDestruct();
}

void UStoneChoiceButtonWidget::InitChoice(int32 InChoiceIndex, const FText& InLabel, bool bInEnabled, bool bInSoftFail, const FText& InDisabledReason, 
                                          const TArray<FStoneOutcome>& InOutcomes, UTexture2D* InIcon, bool bInForcesReturn)
{
	ChoiceIndex = InChoiceIndex;
	bSoftFail = bInSoftFail;
	DisabledReason = InDisabledReason;
	Outcomes = InOutcomes;
	ChoiceIcon = InIcon;
	bForcesReturn = bInForcesReturn;

	SetButtonText(InLabel);
	SetIsEnabled(bInEnabled);

	// Blueprint can now access Outcomes/Icon/ForcesReturn via getters and update preview widgets
	BP_OnOutcomesUpdated();

	SetVisibility(ESlateVisibility::Visible);
}

void UStoneChoiceButtonWidget::HandleBaseClicked()
{
    if (!GetIsEnabled())
    {
        UE_LOG(LogTemp, Verbose, TEXT("[StoneUI][ChoiceButton] Click ignored (disabled) Index=%d"), ChoiceIndex);
        return;
    }

    if (ChoiceIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("[StoneUI][ChoiceButton] Clicked but ChoiceIndex unset"));
        return;
    }

    OnChoiceClicked.Broadcast(ChoiceIndex);
}
