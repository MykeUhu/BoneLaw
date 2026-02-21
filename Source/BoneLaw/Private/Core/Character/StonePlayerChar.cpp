// Copyright by MykeUhu

#include "Core/Character/StonePlayerChar.h"

#include "Core/StonePlayerState.h"
#include "Core/StonePlayerController.h"
#include "AbilitySystem/StoneAbilitySystemComponent.h"
#include "AbilitySystem/StoneAbilitySystemLibrary.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "UI/HUD/StoneHUD.h"

AStonePlayerChar::AStonePlayerChar()
{
	PrimaryActorTick.bCanEverTick = false;
	
	CharacterClass = EStoneCharacterClass::Scout;
}

void AStonePlayerChar::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Init ability actor info for the Server
	InitAbilityActorInfo();
}

void AStonePlayerChar::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Client: init ability actor info
	InitAbilityActorInfo();
}

// Interface Implementations
AActor* AStonePlayerChar::GetAvatar_Implementation()
{
	return Super::GetAvatar_Implementation();
}

EStoneCharacterClass AStonePlayerChar::GetCharacterClass_Implementation()
{
	return Super::GetCharacterClass_Implementation();
}

FOnASCRegistered& AStonePlayerChar::GetOnASCRegisteredDelegate()
{
	return Super::GetOnASCRegisteredDelegate();
}


// end Interface Implementations


void AStonePlayerChar::InitAbilityActorInfo()
{
	AStonePlayerState* StonePlayerState = GetPlayerState<AStonePlayerState>();
	check(StonePlayerState);

	StonePlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(StonePlayerState, this);
	Cast<UStoneAbilitySystemComponent>(StonePlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();

	AbilitySystemComponent = StonePlayerState->GetAbilitySystemComponent();
	AttributeSet = StonePlayerState->GetAttributeSet();

	// ✅ IMPORTANT: initialize defaults on the server (Aura pattern)
	if (HasAuthority())
	{
		static bool bDidInitDefaults = false; // (better: make this a member bool)
		if (!bDidInitDefaults)
		{
			UStoneAbilitySystemLibrary::InitializeDefaultAttributes(
				this,
				CharacterClass,
				StonePlayerState->GetPlayerLevel(),
				AbilitySystemComponent
			);
			bDidInitDefaults = true;
		}
	}

	OnAscRegistered.Broadcast(AbilitySystemComponent);

	if (AStonePlayerController* StonePlayerController = Cast<AStonePlayerController>(GetController()))
	{
		if (AStoneHUD* StoneHUD = Cast<AStoneHUD>(StonePlayerController->GetHUD()))
		{
			StoneHUD->InitOverlay(StonePlayerController, StonePlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
}
