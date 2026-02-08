#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "StoneAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /*AssetTags*/);
DECLARE_MULTICAST_DELEGATE(FAbilitiesGiven);

/**
 * Stone = event-driven. No GameplayAbilities, no slots, no input.
 * - Effect tag broadcast
 * - "ASC ready" delegate (Aura uses AbilitiesGiven for UI init timing)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BONELAW_API UStoneAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	void AbilityActorInfoSet();

	FEffectAssetTags EffectAssetTags;
	FAbilitiesGiven AbilitiesGivenDelegate;

	/** Aura-parity signal: fire once when ASC is initialized (Stone has no abilities). */
	void MarkStartupReady();

protected:
	virtual void OnRep_ActivateAbilities() override;

	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent,
		const FGameplayEffectSpec& EffectSpec,
		FActiveGameplayEffectHandle ActiveEffectHandle);

private:
	bool bStartupReady = false;
};
