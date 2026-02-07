// Copyright by MykeUhu

#include "Core/Character/StonePlayerChar.h"

#include "Core/StonePlayerState.h"
#include "Core/StonePlayerController.h"
#include "AbilitySystem/StoneAbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "UI/HUD/StoneHUD.h"

AStonePlayerChar::AStonePlayerChar()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AStonePlayerChar::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server: init ability actor info
	InitAbilityActorInfo();
}

void AStonePlayerChar::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Client: init ability actor info
	InitAbilityActorInfo();
}

void AStonePlayerChar::InitAbilityActorInfo()
{
	// Player uses PlayerState as Owner, this as Avatar (Aura pattern)
	AStonePlayerState* StonePlayerState = GetPlayerState<AStonePlayerState>();
	if (!StonePlayerState) return;

	AbilitySystemComponent = Cast<UStoneAbilitySystemComponent>(StonePlayerState->GetAbilitySystemComponent());
	AttributeSet = Cast<UStoneAttributeSet>(StonePlayerState->GetAttributeSet());
	if (!AbilitySystemComponent || !AttributeSet) return;

	AbilitySystemComponent->InitAbilityActorInfo(StonePlayerState, this);
	Cast<UStoneAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	
	OnAscRegistered.Broadcast(AbilitySystemComponent);

	// Init HUD overlay from PlayerChar (4 params only)
	if (AStonePlayerController* StonePC = Cast<AStonePlayerController>(GetController()))
	{
		if (AStoneHUD* StoneHUD = Cast<AStoneHUD>(StonePC->GetHUD()))
		{
			StoneHUD->InitOverlay(StonePC, StonePlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	// Aura flow: defaults + abilities after ActorInfo is ready
	InitializeDefaultAttributes();
	AddCharacterAbilities();

	BP_OnAbilitySystemInitialized();
}

void AStonePlayerChar::InitAbilityActorInfoFromPlayerState()
{
	InitAbilityActorInfo();
}
