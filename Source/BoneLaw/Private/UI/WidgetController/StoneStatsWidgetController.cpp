// Copyright by MykeUhu

#include "UI/WidgetController/StoneStatsWidgetController.h"

#include "AbilitySystem/StoneAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "UI/WidgetController/StoneAttributeListener.h"

void UStoneStatsWidgetController::BindCallbacksToDependencies()
{
	check(AbilitySystemComponent);
	check(GetStoneAS());
	checkf(AttributeInfo, TEXT("UStoneStatsWidgetController: AttributeInfo DataAsset is not set (assign it in the BP controller class)."));

	// Clear any prior listeners (controller is cached on HUD; but handle re-init safely)
	AttributeListeners.Reset();

	for (const auto& Pair : GetStoneAS()->TagsToAttributes)
	{
		const FGameplayTag AttributeTag = Pair.Key;
		const FGameplayAttribute Attribute = Pair.Value();

		UStoneAttributeListener* Listener = NewObject<UStoneAttributeListener>(this);
		Listener->Init(this, AbilitySystemComponent, AttributeTag, Attribute);
		AttributeListeners.Add(Listener);
	}
}

void UStoneStatsWidgetController::BroadcastInitialValues()
{
	check(AbilitySystemComponent);
	check(GetStoneAS());
	checkf(AttributeInfo, TEXT("UStoneStatsWidgetController: AttributeInfo DataAsset is not set (assign it in the BP controller class)."));

	for (const auto& Pair : GetStoneAS()->TagsToAttributes)
	{
		const FGameplayTag AttributeTag = Pair.Key;
		const FGameplayAttribute Attribute = Pair.Value();
		const float CurrentValue = Attribute.GetNumericValue(AttributeSet);

		// Aura-like BP event (Tag + Value)
		OnStatChanged.Broadcast(AttributeTag, CurrentValue);

		// Full metadata stream (Name/Icon/Style + Value)
		BroadcastAttributeInfo(AttributeTag, Attribute);
	}
}

void UStoneStatsWidgetController::HandleAnyAttributeChanged(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute, float NewValue) const
{
	// Aura-like BP event (Tag + Value)
	OnStatChanged.Broadcast(AttributeTag, NewValue);

	BroadcastAttributeInfo(AttributeTag, Attribute);
}

void UStoneStatsWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	FStoneAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag, /*bLogNotFound*/ true);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}
