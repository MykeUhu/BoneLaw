// Copyright by MykeUhu

#include "Core/Character/StoneSettlerChar.h"

#include "GameplayTagsManager.h"
#include "AbilitySystem/StoneAbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "AbilitySystem/StoneAbilitySystemLibrary.h"
#include "AI/StoneAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Net/UnrealNetwork.h"

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
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
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

void AStoneSettlerChar::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStoneSettlerChar, RosterSettlerId);
	DOREPLIFETIME(AStoneSettlerChar, RosterDisplayName);
}

void AStoneSettlerChar::SetRosterIdentity(const FGuid& InSettlerId, const FString& InDisplayName)
{
	if (!HasAuthority())
	{
		return;
	}

	RosterSettlerId = InSettlerId;
	RosterDisplayName = InDisplayName;
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
		
		// Default state
		SetState_Idle();
		
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


// -------------------------
// GAS State via GameplayEffects (SSOT: SettlerChar)
// -------------------------

void AStoneSettlerChar::ApplySavedState(const FSavedSettler& SettlerData)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Settler] ApplySavedState skipped (no authority). Settler=%s"), *GetName());
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

	// -------------------------
	// Tags (PERSIST ONLY)
	// -------------------------
	static const FGameplayTag StateRoot  = UGameplayTagsManager::Get().RequestGameplayTag(FName("State"),  /*ErrorIfNotFound*/ false);
	static const FGameplayTag StatusRoot = UGameplayTagsManager::Get().RequestGameplayTag(FName("Status"), /*ErrorIfNotFound*/ false);

	auto FilterPersistedStateTags = [&](const FGameplayTagContainer& In) -> FGameplayTagContainer
	{
		FGameplayTagContainer Out;
		for (const FGameplayTag& Tag : In)
		{
			if ((StateRoot.IsValid()  && Tag.MatchesTag(StateRoot)) ||
				(StatusRoot.IsValid() && Tag.MatchesTag(StatusRoot)))
			{
				Out.AddTag(Tag);
			}
		}
		return Out;
	};

	if (SettlerData.SettlerTags.Num() > 0)
	{
		// Remove old persisted tags (only loose removal; GE-granted tags are unaffected).
		FGameplayTagContainer OwnedNow;
		StoneASC->GetOwnedGameplayTags(OwnedNow);

		const FGameplayTagContainer PersistedNow = FilterPersistedStateTags(OwnedNow);
		if (PersistedNow.Num() > 0)
		{
			StoneASC->RemoveLooseGameplayTags(PersistedNow);
		}

		// Apply persisted saved tags
		const FGameplayTagContainer PersistedSaved = FilterPersistedStateTags(SettlerData.SettlerTags);
		if (PersistedSaved.Num() > 0)
		{
			StoneASC->AddLooseGameplayTags(PersistedSaved);
		}

		UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] ApplySavedState: tags cleared=%d applied=%d (State/Status only). Pawn=%s"),
			PersistedNow.Num(), PersistedSaved.Num(), *GetName());
	}

	// -------------------------
	// Attributes
	// -------------------------
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

	// -------------------------
	// Abilities
	// -------------------------
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

TArray<FSavedAttribute> AStoneSettlerChar::GatherCurrentAttributes() const
{
	TArray<FSavedAttribute> Result;

	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneSettlerChar] GatherCurrentAttributes called without authority. Pawn=%s"), *GetName());
		return Result;
	}

	const UStoneAbilitySystemComponent* StoneASC = Cast<UStoneAbilitySystemComponent>(AbilitySystemComponent);
	if (!StoneASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneSettlerChar] GatherCurrentAttributes: ASC invalid. Pawn=%s"), *GetName());
		return Result;
	}

	const UStoneAttributeSet* AttrSet = Cast<UStoneAttributeSet>(AttributeSet);
	if (!AttrSet)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneSettlerChar] GatherCurrentAttributes: AttributeSet invalid. Pawn=%s"), *GetName());
		return Result;
	}

	// Iterate all registered tag->attribute mappings (covers Primary, Secondary, Vital, Culture, Knowledge, Worldline).
	// This is intentionally tag-driven so no settler-specific hardcoding is needed.
	for (const TPair<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>>& Pair : AttrSet->TagsToAttributes)
	{
		const FGameplayTag& Tag = Pair.Key;
		if (!Tag.IsValid())
		{
			continue;
		}

		const FGameplayAttribute Attr = Pair.Value();
		if (!Attr.IsValid())
		{
			continue;
		}

		// Skip meta attributes (IncomingDamage, IncomingHeal) - these are transient, never persisted.
		const FString AttrName = Attr.AttributeName;
		if (AttrName.StartsWith(TEXT("Incoming")))
		{
			continue;
		}

		const float Value = StoneASC->GetNumericAttribute(Attr);
		Result.Add(FSavedAttribute(Tag, Value));
	}

	UE_LOG(LogTemp, Log, TEXT("[StoneSettlerChar] GatherCurrentAttributes: gathered %d attributes. Pawn=%s"),
		Result.Num(), *GetName());

	return Result;
}

FActiveGameplayEffectHandle AStoneSettlerChar::ApplyStateEffect(TSubclassOf<UGameplayEffect> EffectClass, float EffectLevel)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[StoneSettlerChar] ApplyStateEffect ignored (not authority). Pawn=%s"), *GetName());
		return FActiveGameplayEffectHandle();
	}

	if (!AbilitySystemComponent || !EffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneSettlerChar] ApplyStateEffect failed: ASC or EffectClass null. Pawn=%s ASC=%s Effect=%s"),
			*GetName(), *GetNameSafe(AbilitySystemComponent), *GetNameSafe(EffectClass.Get()));
		return FActiveGameplayEffectHandle();
	}

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, EffectLevel, ContextHandle);
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneSettlerChar] ApplyStateEffect failed: Spec invalid. Pawn=%s Effect=%s"),
			*GetName(), *GetNameSafe(EffectClass.Get()));
		return FActiveGameplayEffectHandle();
	}

	return AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

void AStoneSettlerChar::RemoveStateEffect(FActiveGameplayEffectHandle& Handle)
{
	if (!HasAuthority()) return;

	if (AbilitySystemComponent && Handle.IsValid())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
	}
	Handle.Invalidate();
}

void AStoneSettlerChar::ClearStateEffects()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[StoneSettlerChar] ClearStateEffects ignored (not authority). Pawn=%s"), *GetName());
		return;
	}
	RemoveStateEffect(Handle_State_Idle);
	RemoveStateEffect(Handle_State_TravelToActionStart);
	RemoveStateEffect(Handle_State_ActionRunning);
	RemoveStateEffect(Handle_State_TravelReturning);
}

void AStoneSettlerChar::SetState_Idle()
{
	if (!HasAuthority()) return;

	ClearStateEffects();

	if (!GE_State_Idle)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneSettlerChar] SetState_Idle: GE_State_Idle is not set. Pawn=%s"), *GetName());
		return;
	}

	Handle_State_Idle = ApplyStateEffect(GE_State_Idle, 1.f);
}

void AStoneSettlerChar::SetState_TravelToActionStart()
{
	if (!HasAuthority()) return;

	// Clean slate
	ClearStateEffects();

	// Apply
	Handle_State_TravelToActionStart = ApplyStateEffect(GE_State_TravelToActionStart, 1.f);
}

void AStoneSettlerChar::SetState_ActionRunning()
{
	if (!HasAuthority()) return;

	ClearStateEffects();
	Handle_State_ActionRunning = ApplyStateEffect(GE_State_ActionRunning, 1.f);
}

void AStoneSettlerChar::SetState_TravelReturning()
{
	if (!HasAuthority()) return;

	ClearStateEffects();
	Handle_State_TravelReturning = ApplyStateEffect(GE_State_TravelReturning, 1.f);
}
