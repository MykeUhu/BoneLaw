// Copyright by MykeUhu

#include "UI/Widget/StoneSettlerEncounterWidget.h"
#include "UI/ViewModel/MVVM_SettlerEncounterVM.h"

// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------

void UStoneSettlerEncounterWidget::SetEncounterVM(UMVVM_SettlerEncounterVM* InVM)
{
	
	if (!InVM)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EncounterWidget] SetEncounterVM called with null VM."));
		return;
	}

	EncounterVM = InVM;

	UE_LOG(LogTemp, Log, TEXT("[EncounterWidget] VM assigned: '%s'. Calling BlueprintInitializeWidget."),
		*GetNameSafe(InVM));

	// Notify Blueprint to set up view bindings / play open animation.
	BlueprintInitializeWidget();
	
}

// -------------------------------------------------------------------------
// Choice forwarding
// -------------------------------------------------------------------------

void UStoneSettlerEncounterWidget::SelectChoice(int32 ChoiceIndex)
{
	if (!EncounterVM)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EncounterWidget] SelectChoice(%d) called but EncounterVM is null."), ChoiceIndex);
		return;
	}

	EncounterVM->SelectChoice(ChoiceIndex);
}
