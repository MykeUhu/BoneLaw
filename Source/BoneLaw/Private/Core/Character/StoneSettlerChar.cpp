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
	ActionComponent = CreateDefaultSubobject<UStoneSettlerActionComponent>(TEXT("ActionComponent"));

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetIsReplicated(true);
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	}
}

static const TCHAR* StoneRoleToString(const AActor* Actor)
{
	if (!Actor) return TEXT("null");
	switch (Actor->GetLocalRole())
	{
	case ROLE_Authority:        return TEXT("Authority");
	case ROLE_AutonomousProxy:  return TEXT("AutonomousProxy");
	case ROLE_SimulatedProxy:   return TEXT("SimulatedProxy");
	default:                   return TEXT("None");
	}
}

void AStoneSettlerChar::BeginPlay()
{
	Super::BeginPlay();

	// Aura-style: don't init ActorInfo here for AI pawns (do it on possession/rep)
	UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] BeginPlay Pawn=%s Role=%s Ctrl=%s ASC=%s AS=%s (NO InitAbilityActorInfo here)"),
		*GetName(),
		StoneRoleToString(this),
		*GetNameSafe(GetController()),
		*GetNameSafe(AbilitySystemComponent),
		*GetNameSafe(AttributeSet));
}

void AStoneSettlerChar::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] PossessedBy Pawn=%s Role=%s NewCtrl=%s -> InitAbilityActorInfo()"),
		*GetName(),
		StoneRoleToString(this),
		*GetNameSafe(NewController));

	InitAbilityActorInfo();

	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[StoneSettlerChar] PossessedBy early-out (not authority). Pawn=%s"), *GetName());
		return;
	}

	StoneAIController = Cast<AStoneAIController>(NewController);
	if (StoneAIController && BehaviorTree)
	{
		UBlackboardComponent* BB = StoneAIController->GetBlackboardComponent();
		if (!BB)
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneSettlerChar] No BlackboardComponent on AIController. Pawn=%s Ctrl=%s"),
				*GetName(), *GetNameSafe(StoneAIController));
		}
		else if (!BehaviorTree->BlackboardAsset)
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneSettlerChar] BehaviorTree has no BlackboardAsset. Pawn=%s BT=%s"),
				*GetName(), *GetNameSafe(BehaviorTree));
		}
		else
		{
			BB->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
		}

		const bool bRanBT = StoneAIController->RunBehaviorTree(BehaviorTree);
		UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] RunBehaviorTree Pawn=%s BT=%s Success=%d"),
			*GetName(), *GetNameSafe(BehaviorTree), bRanBT ? 1 : 0);
	}
}

void AStoneSettlerChar::OnRep_Controller()
{
	Super::OnRep_Controller();

	UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] OnRep_Controller Pawn=%s Role=%s Ctrl=%s -> InitAbilityActorInfo()"),
		*GetName(),
		StoneRoleToString(this),
		*GetNameSafe(GetController()));

	InitAbilityActorInfo();
}

void AStoneSettlerChar::InitAbilityActorInfo()
{
	if (!AbilitySystemComponent || !AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneSettlerChar] InitAbilityActorInfo aborted: ASC or AttributeSet null. Pawn=%s ASC=%s AS=%s"),
			*GetName(), *GetNameSafe(AbilitySystemComponent), *GetNameSafe(AttributeSet));
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	UStoneAbilitySystemComponent* StoneASC = Cast<UStoneAbilitySystemComponent>(AbilitySystemComponent);
	if (StoneASC)
	{
		StoneASC->AbilityActorInfoSet();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneSettlerChar] InitAbilityActorInfo: ASC is not UStoneAbilitySystemComponent. Pawn=%s ASC=%s"),
			*GetName(), *GetNameSafe(AbilitySystemComponent));
	}

	const FGameplayAbilityActorInfo* Info = AbilitySystemComponent->AbilityActorInfo.Get();
	UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] ActorInfo Pawn=%s Role=%s Valid=%d Owner=%s Avatar=%s Rep=%d RepMode=%d"),
		*GetName(),
		StoneRoleToString(this),
		Info ? 1 : 0,
		Info ? *GetNameSafe(Info->OwnerActor.Get()) : TEXT("null"),
		Info ? *GetNameSafe(Info->AvatarActor.Get()) : TEXT("null"),
		AbilitySystemComponent->GetIsReplicated() ? 1 : 0,
		(int32)AbilitySystemComponent->ReplicationMode);

	// Server-only defaults + once
	if (HasAuthority() && !bDidDefaultInit)
	{
		UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] DefaultInit START Pawn=%s Class=%d Level=%d"),
			*GetName(), (int32)CharacterClass, Level);

		InitializeDefaultAttributes();
		GiveStartupAbilities();

		bDidDefaultInit = true;

		UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] DefaultInit DONE Pawn=%s"), *GetName());
	}

	OnAscRegistered.Broadcast(AbilitySystemComponent);
}

void AStoneSettlerChar::InitializeDefaultAttributes() const
{
	UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] InitializeDefaultAttributes Pawn=%s Class=%d Level=%d"),
		*GetName(), (int32)CharacterClass, Level);

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

	UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] GiveStartupAbilities Pawn=%s Class=%d"),
		*GetName(), (int32)CharacterClass);

	UStoneAbilitySystemLibrary::GiveStartupAbilities(this, StoneASC, CharacterClass);
}

void AStoneSettlerChar::ApplySavedState(const FSavedSettler& SettlerData)
{
	if (!HasAuthority())
	{
		return;
	}

	UStoneAbilitySystemComponent* StoneASC = Cast<UStoneAbilitySystemComponent>(AbilitySystemComponent);
	if (!StoneASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneSettlerChar] ApplySavedState failed: ASC invalid. Pawn=%s"), *GetName());
		return;
	}

	if (!StoneASC->AbilityActorInfo.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneSettlerChar] ApplySavedState: ActorInfo invalid -> re-init. Pawn=%s"), *GetName());
		InitAbilityActorInfo();
	}

	if (!StoneASC->AbilityActorInfo.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneSettlerChar] ApplySavedState aborted: ActorInfo still invalid. Pawn=%s"), *GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] ApplySavedState Pawn=%s Tags=%d Attr=%d Abilities=%d"),
		*GetName(),
		SettlerData.SettlerTags.Num(),
		SettlerData.Attributes.Num(),
		SettlerData.GrantedAbilities.Num());

	const bool bHasAnyState =
		(SettlerData.SettlerTags.Num() > 0) ||
		(SettlerData.Attributes.Num() > 0) ||
		(SettlerData.GrantedAbilities.Num() > 0);

	if (!bHasAnyState)
	{
		UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] ApplySavedState: empty -> keep defaults. Pawn=%s"), *GetName());
		return;
	}

	// Tags
	if (SettlerData.SettlerTags.Num() > 0)
	{
		StoneASC->AddLooseGameplayTags(SettlerData.SettlerTags);
		UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] ApplySavedState: applied %d tags. Pawn=%s"),
			SettlerData.SettlerTags.Num(), *GetName());
	}

	// Attributes
	const UStoneAttributeSet* AttrSet = Cast<UStoneAttributeSet>(AttributeSet);
	if (AttrSet && SettlerData.Attributes.Num() > 0)
	{
		int32 AppliedCount = 0;

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
				AppliedCount++;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] ApplySavedState: applied %d/%d attributes. Pawn=%s"),
			AppliedCount, SettlerData.Attributes.Num(), *GetName());
	}

	// Abilities
	if (SettlerData.GrantedAbilities.Num() > 0)
	{
		int32 GrantedCount = 0;

		for (const FSavedAbility& SavedAbility : SettlerData.GrantedAbilities)
		{
			if (SavedAbility.GameplayAbility)
			{
				StoneASC->GiveAbility(FGameplayAbilitySpec(SavedAbility.GameplayAbility, SavedAbility.AbilityLevel, INDEX_NONE));
				GrantedCount++;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] ApplySavedState: granted %d/%d abilities. Pawn=%s"),
			GrantedCount, SettlerData.GrantedAbilities.Num(), *GetName());
	}

	bDidApplySavedState = true;
	UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] ApplySavedState DONE Pawn=%s"), *GetName());
}