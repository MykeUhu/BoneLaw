// Copyright by MykeUhu
#include "Core/Character/StoneNPChar.h"

#include "AbilitySystem/StoneAbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"

AStoneNPChar::AStoneNPChar()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// NPC owns ASC/AS directly (Aura enemy style)
	AbilitySystemComponent = CreateDefaultSubobject<UStoneAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UStoneAttributeSet>(TEXT("AttributeSet"));

	// Optional but common:
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void AStoneNPChar::BeginPlay()
{
	Super::BeginPlay();

	InitAbilityActorInfo();
}

void AStoneNPChar::InitAbilityActorInfo()
{
	if (!AbilitySystemComponent || !AttributeSet) return;

	// NPC: owner=this, avatar=this
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->AbilityActorInfoSet();

	// Server applies, replicates out
	InitializeDefaultAttributes();
	AddCharacterAbilities();

	BP_OnAbilitySystemInitialized();
}
