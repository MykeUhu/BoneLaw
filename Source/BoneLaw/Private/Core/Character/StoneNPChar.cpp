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

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// Aura style: register effect callback on the concrete ASC type
	if (UStoneAbilitySystemComponent* StoneASC = Cast<UStoneAbilitySystemComponent>(AbilitySystemComponent))
	{
		StoneASC->AbilityActorInfoSet();
		StoneASC->MarkStartupReady();
	}

	// Server applies defaults (replicates out)
	InitializeDefaultAttributes();

	// NO abilities in Stone.
}

