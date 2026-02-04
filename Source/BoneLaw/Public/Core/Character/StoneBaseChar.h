// Copyright by MykeUhu
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "StoneBaseChar.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;

class UStoneAbilitySystemComponent;
class UStoneAttributeSet;

UCLASS(Abstract)
class BONELAW_API AStoneBaseChar : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AStoneBaseChar();

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UStoneAbilitySystemComponent* GetStoneASC() const { return AbilitySystemComponent; }
	UStoneAttributeSet* GetStoneAttributeSet() const { return AttributeSet; }

protected:
	/**
	 * Aura-Pattern:
	 * - PlayerChar: ASC/AS live on PlayerState -> init in PossessedBy + OnRep_PlayerState
	 * - NPChar: ASC/AS live on the actor -> init in BeginPlay (or PostInitializeComponents)
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UStoneAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UStoneAttributeSet> AttributeSet = nullptr;

	// -------- Default Attribute GEs (asset-driven, Aura style) --------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Defaults")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Defaults")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Defaults")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Defaults")
	TSubclassOf<UGameplayEffect> DefaultCultureAttributes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Defaults")
	TSubclassOf<UGameplayEffect> DefaultKnowledgeAttributes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Defaults")
	TSubclassOf<UGameplayEffect> DefaultWorldlineAttributes;

	// -------- Startup Abilities (optional now, Aura style) --------
	UPROPERTY(EditDefaultsOnly, Category="GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditDefaultsOnly, Category="GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;

	/**
	 * Aura leaves this abstract/empty in base and implements in Player/Enemy.
	 * We do the same: derived classes MUST call InitAbilityActorInfo once they have ASC+AS.
	 */
	virtual void InitAbilityActorInfo();

	// Aura helper
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.f) const;

	// Aura InitializeDefaultAttributes()
	virtual void InitializeDefaultAttributes();

	// Aura AddCharacterAbilities()
	virtual void AddCharacterAbilities();

	// Optional BP hook (safe extension point; Aura does HUD init here in C++)
	UFUNCTION(BlueprintImplementableEvent, Category="GAS")
	void BP_OnAbilitySystemInitialized();
};
