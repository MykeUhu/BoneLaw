#include "Game/StoneRunAnchor.h"

#include "AbilitySystem/StoneAttributeSet.h"
#include "GameplayEffect.h"

AStoneRunAnchor::AStoneRunAnchor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	AbilitySystem->SetIsReplicated(false);
}

void AStoneRunAnchor::InitializeGAS()
{
	check(AbilitySystem);

	// AttributeSet as subobject ensures correct lifetime & serialization behavior.
	AttributeSet = NewObject<UStoneAttributeSet>(this, TEXT("AttributeSet"));
	AbilitySystem->AddAttributeSetSubobject(AttributeSet.Get());

	// Owner/Avatar = this (UI-driven game, no pawn needed)
	AbilitySystem->InitAbilityActorInfo(this, this);
}

void AStoneRunAnchor::ApplyInitialAttributes(const TMap<FGameplayAttribute, float>& InitialValues)
{
	check(AbilitySystem);

	for (const auto& It : InitialValues)
	{
		AbilitySystem->SetNumericAttributeBase(It.Key, It.Value);
	}
}
