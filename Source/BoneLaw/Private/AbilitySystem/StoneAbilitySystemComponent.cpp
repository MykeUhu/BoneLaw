#include "AbilitySystem/StoneAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"

void UStoneAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UStoneAbilitySystemComponent::ClientEffectApplied);
}

void UStoneAbilitySystemComponent::MarkStartupReady()
{
	if (bStartupReady) return;
	bStartupReady = true;
	AbilitiesGivenDelegate.Broadcast();
}

void UStoneAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	// Stone has no abilities, but we still need the Aura-style “ASC ready” pulse for UI.
	MarkStartupReady();
}

void UStoneAbilitySystemComponent::ClientEffectApplied_Implementation(
	UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& EffectSpec,
	FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	EffectAssetTags.Broadcast(TagContainer);
}
