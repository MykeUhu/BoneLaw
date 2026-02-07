// Copyright by MykeUhu

#include "Core/StonePlayerState.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/StoneAbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "Net/UnrealNetwork.h"

AStonePlayerState::AStonePlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UStoneAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UStoneAttributeSet>("AttributeSet");
	
	SetNetUpdateFrequency(100.f);
}

void AStonePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AStonePlayerState, Level);
}

// === IAbilitySystemInterface ===
UAbilitySystemComponent* AStonePlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// === Typed Accessors ===
UStoneAbilitySystemComponent* AStonePlayerState::GetStoneAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UStoneAttributeSet* AStonePlayerState::GetStoneAttributeSet() const
{
	return AttributeSet;
}

UAttributeSet* AStonePlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

void AStonePlayerState::AddToLevel(int32 InLevel)
{
	Level += InLevel;
	OnLevelChangedDelegate.Broadcast(Level, true);
}

void AStonePlayerState::SetLevel(int32 InLevel)
{
	Level = InLevel;
	OnLevelChangedDelegate.Broadcast(Level, false);
}

void AStonePlayerState::OnRep_Level(int32 OldLevel)
{
	OnLevelChangedDelegate.Broadcast(Level, true);
}
