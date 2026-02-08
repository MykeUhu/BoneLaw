// Copyright by MykeUhu
#include "Core/Character/StoneBaseChar.h"

#include "AbilitySystem/StoneAbilitySystemComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Core/StoneGameplayTags.h"

AStoneBaseChar::AStoneBaseChar()
{
	PrimaryActorTick.bCanEverTick = false; // event-driven; no per-frame
	bReplicates = true;
}

void AStoneBaseChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
void AStoneBaseChar::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AStoneBaseChar, bIsStunned);
	//DOREPLIFETIME(AStoneBaseChar, bIsBurned);
	//DOREPLIFETIME(AStoneBaseChar, bIsBeingShocked);
}

UAbilitySystemComponent* AStoneBaseChar::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AStoneBaseChar::BeginPlay()
{
	Super::BeginPlay();
	
}

// Interface Implementations
AActor* AStoneBaseChar::GetAvatar_Implementation()
{
	return this;
}

EStoneCharacterClass AStoneBaseChar::GetCharacterClass_Implementation()
{
	return CharacterClass;
}

FOnASCRegistered& AStoneBaseChar::GetOnASCRegisteredDelegate()
{
	return OnAscRegistered;
}
// end Interface Implementations

void AStoneBaseChar::InitAbilityActorInfo()
{
}

void AStoneBaseChar::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AStoneBaseChar::InitializeDefaultAttributes() const
{
	// Applies on server (then replicated). Keep the same rule.
	if (!HasAuthority()) return;

	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
	ApplyEffectToSelf(DefaultCultureAttributes, 1.f);
	ApplyEffectToSelf(DefaultKnowledgeAttributes, 1.f);
	ApplyEffectToSelf(DefaultWorldlineAttributes, 1.f);
}