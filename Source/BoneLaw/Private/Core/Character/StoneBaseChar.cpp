// Copyright by MykeUhu
#include "Core/Character/StoneBaseChar.h"

#include "AbilitySystem/StoneAbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

AStoneBaseChar::AStoneBaseChar()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

UAbilitySystemComponent* AStoneBaseChar::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AStoneBaseChar::InitAbilityActorInfo()
{
	// Intentionally empty (Aura style).
	// PlayerChar / NPChar must implement/call their own init once ASC+AS are valid.
}

void AStoneBaseChar::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level) const
{
	if (!AbilitySystemComponent || !EffectClass) return;

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, Level, ContextHandle);
	if (SpecHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void AStoneBaseChar::InitializeDefaultAttributes()
{
	// Aura applies on server (then replicated). Keep the same rule.
	if (!HasAuthority()) return;

	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
	ApplyEffectToSelf(DefaultCultureAttributes, 1.f);
	ApplyEffectToSelf(DefaultKnowledgeAttributes, 1.f);
	ApplyEffectToSelf(DefaultWorldlineAttributes, 1.f);
}

void AStoneBaseChar::AddCharacterAbilities()
{
	UStoneAbilitySystemComponent* StoneASC = GetStoneASC();
	if (!StoneASC) return;

	if (!HasAuthority()) return;

	StoneASC->AddCharacterAbilities(StartupAbilities);
	StoneASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
}
