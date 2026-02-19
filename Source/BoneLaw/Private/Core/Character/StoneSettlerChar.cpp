// Copyright by MykeUhu

#include "Core/Character/StoneSettlerChar.h"

#include "AbilitySystem/StoneAbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "AbilitySystem/StoneAbilitySystemLibrary.h"
#include "AI/StoneAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AStoneSettlerChar::AStoneSettlerChar()
{
	bReplicates = true;

	// Pawn-owned ASC (AuraEnemy style)
	AbilitySystemComponent = CreateDefaultSubobject<UStoneAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UStoneAttributeSet>(TEXT("AttributeSet"));

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetIsReplicated(true);
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	}
}

void AStoneSettlerChar::BeginPlay()
{
	Super::BeginPlay();

	// Pawn owns ASC -> we can init immediately.
	InitAbilityActorInfo();
}

void AStoneSettlerChar::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();

	if (!HasAuthority())
	{
		return;
	}

	StoneAIController = Cast<AStoneAIController>(NewController);
	if (StoneAIController && BehaviorTree)
	{
		if (UBlackboardComponent* BB = StoneAIController->GetBlackboardComponent())
		{
			if (BehaviorTree->BlackboardAsset)
			{
				BB->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[StoneSettlerChar] BehaviorTree has no BlackboardAsset. Pawn=%s"), *GetName());
			}
		}

		const bool bRanBT = StoneAIController->RunBehaviorTree(BehaviorTree);
		if (!bRanBT)
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneSettlerChar] RunBehaviorTree failed. Pawn=%s BT=%s"),
				*GetName(), *GetNameSafe(BehaviorTree));
		}
	}
}

void AStoneSettlerChar::OnRep_Controller()
{
	Super::OnRep_Controller();
	InitAbilityActorInfo();
}

void AStoneSettlerChar::InitAbilityActorInfo()
{
	// Owner=this, Avatar=this (AuraEnemy)
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UStoneAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	if (HasAuthority())
	{
		InitializeDefaultAttributes();		
	}
	OnAscRegistered.Broadcast(AbilitySystemComponent);
}

void AStoneSettlerChar::InitializeDefaultAttributes() const
{
	UStoneAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}

void AStoneSettlerChar::GiveStartupAbilities() const
{
	UStoneAbilitySystemComponent* StoneASC = Cast<UStoneAbilitySystemComponent>(AbilitySystemComponent);
	if (!StoneASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneSettlerChar] GiveStartupAbilities failed: ASC invalid. Pawn=%s"), *GetName());
		return;
	}

	if (!StoneASC->AbilityActorInfo.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneSettlerChar] GiveStartupAbilities aborted: AbilityActorInfo invalid. Pawn=%s"), *GetName());
		return;
	}

	UStoneAbilitySystemLibrary::GiveStartupAbilities(this, StoneASC, CharacterClass);
}

void AStoneSettlerChar::ApplySavedState(const FSavedSettler& SettlerData)
{
	if (!HasAuthority())
	{
		// Save-state application is authoritative; clients receive via replication / effects.
		return;
	}

	UStoneAbilitySystemComponent* StoneASC = Cast<UStoneAbilitySystemComponent>(AbilitySystemComponent);
	if (!StoneASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneSettlerChar] ApplySavedState failed: ASC invalid. Pawn=%s"), *GetName());
		return;
	}

	// Ensure ActorInfo is valid before touching attributes/tags/abilities
	if (!StoneASC->AbilityActorInfo.IsValid())
	{
		InitAbilityActorInfo();
	}

	if (!StoneASC->AbilityActorInfo.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneSettlerChar] ApplySavedState aborted: AbilityActorInfo still invalid. Pawn=%s"), *GetName());
		return;
	}

	// IMPORTANT: If saved state is empty, we do NOT "overwrite to zero".
	// This is the exact bug pattern you hit.
	const bool bHasAnyState =
		(SettlerData.SettlerTags.Num() > 0) ||
		(SettlerData.Attributes.Num() > 0) ||
		(SettlerData.GrantedAbilities.Num() > 0);

	if (!bHasAnyState)
	{
		// Nothing to apply -> keep defaults.
		return;
	}

	// Tags
	if (SettlerData.SettlerTags.Num() > 0)
	{
		StoneASC->AddLooseGameplayTags(SettlerData.SettlerTags);
	}

	// Attributes
	const UStoneAttributeSet* AttrSet = Cast<UStoneAttributeSet>(AttributeSet);
	if (AttrSet && SettlerData.Attributes.Num() > 0)
	{
		for (const FSavedAttribute& SavedAttr : SettlerData.Attributes)
		{
			if (!SavedAttr.AttributeTag.IsValid())
			{
				continue;
			}

			FGameplayAttribute GameplayAttr;
			if (AttrSet->GetAttributeFromTag(SavedAttr.AttributeTag, GameplayAttr) && GameplayAttr.IsValid())
			{
				StoneASC->SetNumericAttributeBase(GameplayAttr, SavedAttr.Value);
			}
		}
	}

	// Abilities
	if (SettlerData.GrantedAbilities.Num() > 0)
	{
		for (const FSavedAbility& SavedAbility : SettlerData.GrantedAbilities)
		{
			if (SavedAbility.GameplayAbility)
			{
				StoneASC->GiveAbility(FGameplayAbilitySpec(SavedAbility.GameplayAbility, SavedAbility.AbilityLevel, INDEX_NONE));
			}
		}
	}

	bDidApplySavedState = true;
}
