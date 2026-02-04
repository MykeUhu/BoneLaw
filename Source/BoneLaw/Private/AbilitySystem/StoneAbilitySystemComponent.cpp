// StoneAbilitySystemComponent.cpp

#include "AbilitySystem/StoneAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/StoneGameplayAbility.h"
#include "Core/StoneGameplayTags.h"

void UStoneAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UStoneAbilitySystemComponent::ClientEffectApplied);
}

void UStoneAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& InStartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : InStartupAbilities)
	{
		if (!AbilityClass) continue;

		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);

		if (const UStoneGameplayAbility* StoneAbility = Cast<UStoneGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(StoneAbility->StartupInputTag);
			AbilitySpec.DynamicAbilityTags.AddTag(FStoneGameplayTags::Get().Abilities_Status_Equipped);
		}

		GiveAbility(AbilitySpec);
	}

	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
}

void UStoneAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& InStartupPassiveAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : InStartupPassiveAbilities)
	{
		if (!AbilityClass) continue;

		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
		AbilitySpec.DynamicAbilityTags.AddTag(FStoneGameplayTags::Get().Abilities_Status_Equipped);

		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UStoneAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if (!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();
	}
}

void UStoneAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	EffectAssetTags.Broadcast(TagContainer);
}
