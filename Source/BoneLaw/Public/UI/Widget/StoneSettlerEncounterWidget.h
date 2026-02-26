// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoneSettlerEncounterWidget.generated.h"

class UMVVM_SettlerEncounterVM;
class AStoneSettlerChar;
class UStoneEventData;

/**
 * Base widget class for the per-settler encounter (event) popup.
 *
 * Pattern mirrors WBP_SettlerSlot_Details:
 *   1. Blueprint sub-classes this (e.g. WBP_SettlerEncounter).
 *   2. The owning screen (SettlerSlotDetails BP) listens to
 *      UStoneSettlerActionComponent::OnEncounterOpened and calls
 *      SetupVM(SettlerActor, EventData) which:
 *        a. Creates / fetches UMVVM_SettlerEncounterVM from ViewModelGameSubsystem.
 *        b. Calls BindToEncounter(SettlerActor, EventData) on the VM.
 *        c. Calls BlueprintInitializeWidget() for any layout work.
 *   3. Widget binds MVVM fields (EventTitle, EventBody, NumChoices, …) in the
 *      Designer via the standard MVVM binding panel.
 *   4. When UMVVM_SettlerEncounterVM::OnEncounterClosed fires, Blueprint hides/
 *      removes this widget.
 *
 * C++ responsibility:
 *   - Provide a typed reference to the VM (EncounterVM) so BP can call SelectChoice().
 *   - Forward SetupVM to BP via BlueprintImplementableEvent.
 *   - Provide a convenience function SelectChoice() so choice buttons can call it
 *     without a direct VM reference.
 */
UCLASS(Abstract, BlueprintType)
class BONELAW_API UStoneSettlerEncounterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// -------------------------
	// Setup API (called by parent / owning screen BP)
	// -------------------------

	/**
	 * Assign the already-configured ViewModel to this widget and trigger BP init.
	 * Call this after BindToEncounter() has already been called on the VM.
	 *
	 * @param InVM  The configured UMVVM_SettlerEncounterVM instance.
	 */
	UFUNCTION(BlueprintCallable, Category="Stone|UI|Encounter")
	void SetEncounterVM(UMVVM_SettlerEncounterVM* InVM);

	/**
	 * Blueprint hook called after the VM has been assigned.
	 * Use this to set up MVVM view bindings or play an open animation.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Stone|UI|Encounter")
	void BlueprintInitializeWidget();

	// -------------------------
	// Choice API (forwarded to VM)
	// -------------------------

	/**
	 * Select a choice by index.
	 * Forwards to EncounterVM->SelectChoice(ChoiceIndex).
	 * Call this from choice button click events in Blueprint.
	 */
	UFUNCTION(BlueprintCallable, Category="Stone|UI|Encounter")
	void SelectChoice(int32 ChoiceIndex);

	// -------------------------
	// Accessors
	// -------------------------

	/** Returns the currently assigned EncounterVM (may be nullptr before SetEncounterVM). */
	UFUNCTION(BlueprintPure, Category="Stone|UI|Encounter")
	UMVVM_SettlerEncounterVM* GetEncounterVM() const { return EncounterVM; }

protected:
	/**
	 * Typed reference to the ViewModel.
	 * Exposed as BlueprintReadOnly so Designer bindings and BP logic can access it.
	 * Written only via SetEncounterVM() (C++ controlled).
	 */
	UPROPERTY(BlueprintReadOnly, Transient, Category="Stone|UI|Encounter")
	TObjectPtr<UMVVM_SettlerEncounterVM> EncounterVM;
};
