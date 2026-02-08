// Copyright by MykeUhu

#include "Core/Character/StonePlayerChar.h"

#include "Core/StonePlayerState.h"
#include "Core/StonePlayerController.h"
#include "AbilitySystem/StoneAbilitySystemComponent.h"
#include "AbilitySystem/StoneAbilitySystemLibrary.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "Core/StoneGameMode.h"
#include "Core/LoadScreenSaveGame.h"
#include "Kismet/GameplayStatics.h"
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
	
	
	LoadProgress();
	
	if (AStoneGameMode* StoneGameMode = Cast<AStoneGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		StoneGameMode->LoadWorldState(GetWorld());
	} 
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

void AStonePlayerChar::SaveProgress_Implementation(const FName& CheckpointTag)
{
	AStoneGameMode* StoneGameMode = Cast<AStoneGameMode>(UGameplayStatics::GetGameMode(this));
	if (StoneGameMode)
	{
		ULoadScreenSaveGame* SaveData = StoneGameMode->RetrieveInGameSaveData();
		if (SaveData == nullptr) return;

		SaveData->PlayerStartTag = CheckpointTag;

		if (AStonePlayerState* StonePlayerState = Cast<AStonePlayerState>(GetPlayerState()))
		{
			SaveData->PlayerLevel = StonePlayerState->GetPlayerLevel();
		}
		SaveData->Strength = UStoneAttributeSet::GetStrengthAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Intelligence = UStoneAttributeSet::GetIntelligenceAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Endurance = UStoneAttributeSet::GetEnduranceAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Willpower = UStoneAttributeSet::GetWillpowerAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Social    = UStoneAttributeSet::GetSocialAttribute().GetNumericValue(GetAttributeSet());

		SaveData->Food      = UStoneAttributeSet::GetFoodAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Water     = UStoneAttributeSet::GetWaterAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Health    = UStoneAttributeSet::GetHealthAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Morale    = UStoneAttributeSet::GetMoraleAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Warmth    = UStoneAttributeSet::GetWarmthAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Trust     = UStoneAttributeSet::GetTrustAttribute().GetNumericValue(GetAttributeSet());

		SaveData->MaxFood   = UStoneAttributeSet::GetMaxFoodAttribute().GetNumericValue(GetAttributeSet());
		SaveData->MaxWater  = UStoneAttributeSet::GetMaxWaterAttribute().GetNumericValue(GetAttributeSet());
		SaveData->MaxHealth = UStoneAttributeSet::GetMaxHealthAttribute().GetNumericValue(GetAttributeSet());
		SaveData->MaxMorale = UStoneAttributeSet::GetMaxMoraleAttribute().GetNumericValue(GetAttributeSet());
		SaveData->MaxTrust  = UStoneAttributeSet::GetMaxTrustAttribute().GetNumericValue(GetAttributeSet());

		SaveData->bFirstTimeLoadIn = false;

		if (!HasAuthority()) return;

		// Stone has no GameplayAbilities. Keep Aura save field empty.
		SaveData->SavedAbilities.Empty();

		StoneGameMode->SaveInGameProgressData(SaveData);
	}
}

// end Interface Implementations

void AStonePlayerChar::LoadProgress()
{
	AStoneGameMode* StoneGameMode = Cast<AStoneGameMode>(UGameplayStatics::GetGameMode(this));
	if (!StoneGameMode) return;

	ULoadScreenSaveGame* SaveData = StoneGameMode->RetrieveInGameSaveData();
	if (SaveData == nullptr) return;

	if (SaveData->bFirstTimeLoadIn)
	{
		InitializeDefaultAttributes();
		// Stone: keine Abilities
	}
	else
	{
		if (AStonePlayerState* StonePlayerState = Cast<AStonePlayerState>(GetPlayerState()))
		{
			StonePlayerState->SetLevel(SaveData->PlayerLevel);
		}

		// Aura-Pattern: SetByCaller GE aus SaveData wieder anwenden
		UStoneAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(this, AbilitySystemComponent, SaveData);
	}
}

void AStonePlayerChar::InitAbilityActorInfo()
{
	AStonePlayerState* StonePlayerState = GetPlayerState<AStonePlayerState>();
	check(StonePlayerState);
	StonePlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(StonePlayerState, this);
	Cast<UStoneAbilitySystemComponent>(StonePlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = StonePlayerState->GetAbilitySystemComponent();
	AttributeSet = StonePlayerState->GetAttributeSet();
	OnAscRegistered.Broadcast(AbilitySystemComponent);
	//AbilitySystemComponent->RegisterGameplayTagEvent(FStoneGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AStonePlayerChar::StunTagChanged);

	if (AStonePlayerController* StonePlayerController = Cast<AStonePlayerController>(GetController()))
	{
		if (AStoneHUD* StoneHUD = Cast<AStoneHUD>(StonePlayerController->GetHUD()))
		{
			StoneHUD->InitOverlay(StonePlayerController, StonePlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
}
