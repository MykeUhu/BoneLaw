// StoneAbilitySystemComponent.h

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "StoneAbilitySystemComponent.generated.h"

class UStoneSaveGame;

DECLARE_MULTICAST_DELEGATE(FAbilitiesGiven);
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /*AssetTags*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BONELAW_API UStoneAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	void AbilityActorInfoSet();

	// === Aura parity ===
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& InStartupAbilities);
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& InStartupPassiveAbilities);

	// optional / wenn du es wirklich nutzt:
	void AddCharacterAbilitiesFromSaveData(UStoneSaveGame* SaveData);

	// state + delegates (Aura)
	bool bStartupAbilitiesGiven = false;
	FAbilitiesGiven AbilitiesGivenDelegate;

	FEffectAssetTags EffectAssetTags;

protected:
	virtual void OnRep_ActivateAbilities() override;

	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent,
		const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);
};
