// Copyright by MykeUhu
#include "Core/Character/StonePlayerChar.h"

#include "Core/StonePlayerState.h"
#include "AbilitySystem/StoneAbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"

AStonePlayerChar::AStonePlayerChar()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AStonePlayerChar::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Aura: server init here
	InitAbilityActorInfoFromPlayerState();
}

void AStonePlayerChar::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Aura: client init here
	InitAbilityActorInfoFromPlayerState();
}

void AStonePlayerChar::InitAbilityActorInfo()
{
	// Player uses PlayerState as Owner, this as Avatar.
	// (Kein this,this hier!)
	AStonePlayerState* PS = GetPlayerState<AStonePlayerState>();
	if (!PS) return;

	AbilitySystemComponent = Cast<UStoneAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	AttributeSet = Cast<UStoneAttributeSet>(PS->GetAttributeSet());
	if (!AbilitySystemComponent || !AttributeSet) return;

	AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	AbilitySystemComponent->AbilityActorInfoSet();

	// Aura flow: defaults + abilities after ActorInfo is ready (server only)
	InitializeDefaultAttributes();
	AddCharacterAbilities();

	BP_OnAbilitySystemInitialized();
}

void AStonePlayerChar::InitAbilityActorInfoFromPlayerState()
{
	InitAbilityActorInfo();
}
