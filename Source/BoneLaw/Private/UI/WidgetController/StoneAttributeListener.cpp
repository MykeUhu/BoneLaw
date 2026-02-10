// Copyright by MykeUhu

#include "UI/WidgetController/StoneAttributeListener.h"

#include "UI/WidgetController/StoneStatsWidgetController.h"

void UStoneAttributeListener::Init(UStoneStatsWidgetController* InOwner, UAbilitySystemComponent* InASC, const FGameplayTag& InTag, const FGameplayAttribute& InAttribute)
{
	Owner = InOwner;
	ASC = InASC;
	AttributeTag = InTag;
	Attribute = InAttribute;

	check(Owner);
	check(ASC);

	// Bind without lambdas (AAA-safe: deterministic teardown, no captured state issues)
	Handle = ASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UStoneAttributeListener::HandleChange);
}

void UStoneAttributeListener::BeginDestroy()
{
	if (ASC && Handle.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(Attribute).Remove(Handle);
		Handle.Reset();
	}
	Super::BeginDestroy();
}

void UStoneAttributeListener::HandleChange(const FOnAttributeChangeData& Data)
{
	if (!Owner) return;
	Owner->HandleAnyAttributeChanged(AttributeTag, Attribute, Data.NewValue);
}
