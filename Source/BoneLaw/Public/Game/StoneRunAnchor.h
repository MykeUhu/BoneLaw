#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "StoneRunAnchor.generated.h"

class UStoneAttributeSet;

UCLASS()
class BONELAW_API AStoneRunAnchor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AStoneRunAnchor();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem; }

	UFUNCTION(BlueprintCallable, Category="Stone|GAS")
	void InitializeGAS();

	UFUNCTION(BlueprintCallable, Category="Stone|GAS")
	void ApplyInitialAttributes(const TMap<FGameplayAttribute, float>& InitialValues);

	UStoneAttributeSet* GetAttributeSet() const { return AttributeSet; }

private:
	UPROPERTY(VisibleAnywhere, Category="Stone|GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystem;

	UPROPERTY()
	TObjectPtr<UStoneAttributeSet> AttributeSet;
};
