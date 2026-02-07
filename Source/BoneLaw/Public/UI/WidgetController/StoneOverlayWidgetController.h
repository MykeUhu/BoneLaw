// Copyright by MykeUhu
// Following Aura pattern - typed getters inherited from base

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/StoneWidgetController.h"
#include "Runtime/StoneRunSubsystem.h"
#include "StoneOverlayWidgetController.generated.h"

class UStoneEventData;

// Delegates for UI binding (Stone-specific)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneOverlaySnapshotSig, const FStoneSnapshot&, Snapshot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneOverlayEventSig, const UStoneEventData*, Event);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewValue);

/**
 * UStoneOverlayWidgetController - Main HUD Widget Controller
 * Following Aura pattern + Stone-specific RunSubsystem for events
 */
UCLASS(BlueprintType, Blueprintable)
class BONELAW_API UStoneOverlayWidgetController : public UStoneWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxHealthChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnFoodChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxFoodChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnWaterChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxWaterChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnWarmthChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxWarmthChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnMoraleChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxMoraleChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnTrustChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxTrustChanged;
	
	// === Delegates for Blueprint UI binding ===
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FStoneOverlaySnapshotSig OnOverlaySnapshotChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Events")
	FStoneOverlayEventSig OnOverlayEventChanged;

	// === Stone-specific: RunSubsystem access ===
	UFUNCTION(BlueprintPure, Category="Stone|UI")
	UStoneRunSubsystem* GetRunSubsystem() const;

	UFUNCTION(BlueprintCallable, Category="Stone|UI")
	void GetResolvedChoices(TArray<FStoneChoiceResolved>& OutChoices) const;

private:
	// Cached RunSubsystem (lazy-loaded like Aura pattern)
	UPROPERTY()
	mutable TObjectPtr<UStoneRunSubsystem> CachedRunSubsystem;

	UFUNCTION()
	void HandleSnapshotChanged(const FStoneSnapshot& Snapshot);

	UFUNCTION()
	void HandleEventChanged(const UStoneEventData* Event);
};
