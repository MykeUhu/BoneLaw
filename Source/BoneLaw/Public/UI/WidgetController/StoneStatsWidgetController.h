// Copyright by MykeUhu
// Stone Stats Widget Controller - Aura AttributeMenu pattern adapted (no spend points; event-driven changes)

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/WidgetController/StoneWidgetController.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "StoneStatsWidgetController.generated.h"

class UAttributeInfo;
class UStoneAttributeListener;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneAttributeInfoSignature, const FStoneAttributeInfo&, Info);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStoneStatValueChangedSignature, FGameplayTag, StatTag, float, NewValue);

/**
 * UStoneStatsWidgetController
 * - Shows Primary/Secondary/Vital attributes inside a Statistics widget (WidgetSwitcher screen)
 * - Follows Aura AttributeMenuWidgetController pattern:
 *   - iterates AttributeSet->TagsToAttributes
 *   - broadcasts FStoneAttributeInfo with current values
 * - No +Buttons: attributes are modified by Outcomes/GameplayEffects; UI reacts via AttributeChange delegates.
 */
UCLASS(BlueprintType, Blueprintable)
class BONELAW_API UStoneStatsWidgetController : public UStoneWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValues() override;

	/** UI binds to this to receive a stream of attribute infos (one call per attribute). */
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FStoneAttributeInfoSignature AttributeInfoDelegate;

	/** Aura-like typed feeling: one BP-assignable event for any stat change (Tag + Value). */
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FStoneStatValueChangedSignature OnStatChanged;

protected:
	/** DataAsset mapping AttributeTag -> DisplayName/Description. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Attributes")
	TObjectPtr<UAttributeInfo> AttributeInfo;

private:
	friend class UStoneAttributeListener;

	/** One listener per attribute (avoids lambdas; clean unbinding; MP-safe on owning client). */
	UPROPERTY()
	TArray<TObjectPtr<UStoneAttributeListener>> AttributeListeners;

	/** Called by listeners when an attribute value changes. */
	void HandleAnyAttributeChanged(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute, float NewValue) const;

	/** Emits AttributeInfoDelegate for a given attribute tag + gameplay attribute. */
	void BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const;
};
