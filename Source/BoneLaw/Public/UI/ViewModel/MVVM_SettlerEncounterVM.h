// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Data/StoneTypes.h"
#include "MVVM_SettlerEncounterVM.generated.h"

class AStoneSettlerChar;
class UStoneSettlerActionComponent;
class UStoneEventData;
struct FStoneChoiceResolved;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneEncounterChoiceSelected, int32, ChoiceIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneEncounterClosed, bool, bAborted);

UCLASS()
class BONELAW_API UMVVM_SettlerEncounterVM : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void BindToEncounter(AStoneSettlerChar* SettlerActor, const UStoneEventData* EventData);

	UFUNCTION(BlueprintCallable)
	void SelectChoice(int32 ChoiceIndex);

	/**
	 * Applies all outcomes for the given UI choice index directly against the bound settler's ASC.
	 * Call this from Blueprint BEFORE or AFTER SelectChoice (order: ApplyChoiceOutcomes -> SelectChoice).
	 * Builds the FStoneOutcomeContext internally - no C++ structs needed in Blueprint.
	 */
	UFUNCTION(BlueprintCallable, Category="Encounter")
	void ApplyChoiceOutcomes(int32 ChoiceIndex);

	UPROPERTY(BlueprintAssignable)
	FStoneEncounterChoiceSelected OnChoiceSelected;

	UPROPERTY(BlueprintAssignable)
	FStoneEncounterClosed OnEncounterClosed;
	
	// MVVM Setters (match field types!)
	void SetEventTitle(const FString InTitle);
	void SetEventBody(const FText InBody);
	void SetSettlerName(const FString InName);
	void SetNumChoices(int32 InNum);
	void SetbIsVisible(bool bInVisible);

	void RebuildChoices();

	// MVVM Getters
	FString GetEventTitle() const { return EventTitle; }
	FText   GetEventBody()  const { return EventBody; }
	FString GetSettlerName() const { return SettlerName; }
	int32   GetNumChoices() const { return NumChoices; }
	bool    GetbIsVisible() const { return bIsVisible; }

protected:
	virtual void BeginDestroy() override;

private:
	UFUNCTION()
	void HandleEncounterClosed(bool bAborted);

	// Bound state
	UPROPERTY(Transient) TObjectPtr<AStoneSettlerChar> BoundSettler;
	UPROPERTY(Transient) TObjectPtr<UStoneSettlerActionComponent> BoundActionComp;
	UPROPERTY(Transient) TObjectPtr<const UStoneEventData> BoundEvent;

	// MVVM Fields
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	FString EventTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	FText EventBody;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	FString SettlerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	int32 NumChoices = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	bool bIsVisible = false;

public:
	// Blueprint choice helpers (Index is UI index 0..NumChoices-1)
	UFUNCTION(BlueprintPure)
	FText GetChoiceTextAtIndex(int32 Index) const;

	UFUNCTION(BlueprintPure)
	TArray<FStoneOutcome> GetChoiceOutcomesAtIndex(int32 Index) const;

	UFUNCTION(BlueprintPure)
	bool IsChoiceLockedAtIndex(int32 Index) const;

	UFUNCTION(BlueprintPure)
	bool IsChoiceSoftFailAtIndex(int32 Index) const;

	UFUNCTION(BlueprintPure)
	FText GetChoiceDisabledReasonAtIndex(int32 Index) const;

private:
	// Resolved state (same length as BoundEvent->Choices)
	UPROPERTY(Transient)
	TArray<FStoneChoiceResolved> ResolvedChoices;

	// UI index -> Raw choice index mapping (FIX for VisibleCount problem)
	UPROPERTY(Transient)
	TArray<int32> VisibleChoiceIndices;

	int32 ToRawChoiceIndex(int32 UiIndex) const;
};
