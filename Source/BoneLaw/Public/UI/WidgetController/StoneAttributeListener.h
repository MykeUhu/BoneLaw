// Copyright by MykeUhu
// Internal helper: binds a single GameplayAttribute change delegate and calls back into UStoneStatsWidgetController.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "AbilitySystemComponent.h"
#include "StoneAttributeListener.generated.h"

class UStoneStatsWidgetController;

UCLASS()
class BONELAW_API UStoneAttributeListener : public UObject
{
	GENERATED_BODY()

public:
	void Init(UStoneStatsWidgetController* InOwner, UAbilitySystemComponent* InASC, const FGameplayTag& InTag, const FGameplayAttribute& InAttribute);

protected:
	virtual void BeginDestroy() override;

private:
	UPROPERTY()
	TObjectPtr<UStoneStatsWidgetController> Owner;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	FGameplayTag AttributeTag;

	FGameplayAttribute Attribute;

	FDelegateHandle Handle;

	void HandleChange(const FOnAttributeChangeData& Data);
};
