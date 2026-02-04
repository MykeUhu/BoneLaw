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

void UStoneChoiceButtonWidget::InitChoice(int32 InChoiceIndex, const FText& InLabel, bool bInEnabled, bool bInSoftFail, const FText& InDisabledReason)
{
    ChoiceIndex = InChoiceIndex;
    bSoftFail = bInSoftFail;
    DisabledReason = InDisabledReason;

    SetButtonText(InLabel);
    SetIsEnabled(bInEnabled);

    // Optional: softfail looks enabled but could show a hint on hover (wenn dein CustomButton das unterstützt)
    // z.B. SetToolTipText(bSoftFail ? DisabledReason : FText::GetEmpty());

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
