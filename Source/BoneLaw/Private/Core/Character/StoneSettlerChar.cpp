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
	// GAS init happens in PossessedBy (authority) / OnRep_Controller (clients) - not here.
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
	if (StoneAIController && BehaviorTree && BehaviorTree->BlackboardAsset)
	{
		UBlackboardComponent* BB = StoneAIController->GetBlackboardComponent();
		if (!ensureMsgf(BB, TEXT("[StoneSettlerChar] PossessedBy: No BlackboardComponent. Pawn=%s Ctrl=%s"),
			*GetName(), *GetNameSafe(StoneAIController)))
		{
			return;
		}

		BB->InitializeBlackboard(*BehaviorTree->BlackboardAsset);

		// Hard reset action keys on possess (fixes load/restart ghost state)
		StoneAIController->ResetActionBlackboardKeys();

		StoneAIController->RunBehaviorTree(BehaviorTree);
	}
}

void AStoneSettlerChar::OnRep_Controller()
{
	Super::OnRep_Controller();
	InitAbilityActorInfo();
}

void AStoneSettlerChar::InitAbilityActorInfo()
{
	check(AbilitySystemComponent);
	check(AttributeSet);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (UStoneAbilitySystemComponent* StoneASC = Cast<UStoneAbilitySystemComponent>(AbilitySystemComponent))
	{
		StoneASC->AbilityActorInfoSet();
	}

	if (HasAuthority() && !bDidDefaultInit)
	{
		InitializeDefaultAttributes();
		GiveStartupAbilities();
		bDidDefaultInit = true;
		SetState_Idle();
	}

	// If we received save data before possession/ActorInfo was ready, apply it now deterministically.
	if (HasAuthority() && bHasDeferredSavedState && !bDidApplySavedState)
	{
		const FSavedSettler Copy = DeferredSavedState;
		bHasDeferredSavedState = false;
		DeferredSavedState = FSavedSettler();

		ApplySavedState(Copy);
	}

	OnAscRegistered.Broadcast(AbilitySystemComponent);
}

void AStoneSettlerChar::InitializeDefaultAttributes() const
{
	UStoneAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}

void AStoneSettlerChar::GiveStartupAbilities() const
{
	// CastChecked is safe here: constructor guarantees AbilitySystemComponent is UStoneAbilitySystemComponent.
	UStoneAbilitySystemComponent* StoneASC = CastChecked<UStoneAbilitySystemComponent>(AbilitySystemComponent);
	UStoneAbilitySystemLibrary::GiveStartupAbilities(this, StoneASC, CharacterClass);
}


// -------------------------
// GAS State via GameplayEffects (SSOT: SettlerChar)
// -------------------------

void AStoneSettlerChar::ApplySavedState(const FSavedSettler& SettlerData)
{
	if (!HasAuthority())
	{
		return;
	}

	checkf(AbilitySystemComponent, TEXT("[StoneSettlerChar] ApplySavedState: ASC is null. Pawn=%s"), *GetName());
	checkf(AttributeSet, TEXT("[StoneSettlerChar] ApplySavedState: AttributeSet is null. Pawn=%s"), *GetName());

	// If the save data contains no state, the defaults applied in InitAbilityActorInfo are correct (new game).
	const bool bHasAnyState =
		(SettlerData.SettlerTags.Num() > 0) ||
		(SettlerData.Attributes.Num() > 0) ||
		(SettlerData.GrantedAbilities.Num() > 0) ||
		(SettlerData.ActionState.HasRunningAction());

	if (!bHasAnyState)
	{
		return;
	}

	// ApplySavedState can be called immediately after SpawnActor from the RosterSubsystem.
	// Depending on BP defaults (AutoPossessAI) and timing, PossessedBy/InitAbilityActorInfo may not have run yet.
	// In that case, we must defer until ActorInfo is valid (SSOT init order).
	UStoneAbilitySystemComponent* StoneASC = CastChecked<UStoneAbilitySystemComponent>(AbilitySystemComponent);

	if (!StoneASC->AbilityActorInfo.IsValid())
	{
		DeferredSavedState = SettlerData;
		bHasDeferredSavedState = true;

		UE_LOG(LogTemp, Warning, TEXT("[StoneSettlerChar] ApplySavedState deferred: ActorInfo invalid. Pawn=%s"), *GetName());
		return;
	}

	// -------------------------
	// Tags: only persist stable tags (Status.* etc.).
	//
	// IMPORTANT: "State.*" is BT control-flow and is granted by state GameplayEffects in this project.
	// Persisting it as *loose* tags will make BT decorators think the settler is stuck in that state forever.
	// State is restored deterministically from the ActionComponent (and other runtime systems), not from save tags.
	// -------------------------
	static const FGameplayTag StatusRoot = UGameplayTagsManager::Get().RequestGameplayTag(FName("Status"), false);

	auto FilterPersistedTags = [&](const FGameplayTagContainer& In) -> FGameplayTagContainer
	{
		FGameplayTagContainer Out;
		for (const FGameplayTag& Tag : In)
		{
			if (StatusRoot.IsValid() && Tag.MatchesTag(StatusRoot))
			{
				Out.AddTag(Tag);
			}
		}
		return Out;
	};

	// Backward-compat safety:
	// Older saves may have persisted "State.*" as loose tags (which breaks BT state after load).
	// Remove any loose State tags first. This does NOT affect GE-granted State tags.
	static const FGameplayTag StateRoot = UGameplayTagsManager::Get().RequestGameplayTag(FName("State"), false);
	if (StateRoot.IsValid())
	{
		FGameplayTagContainer OwnedNow;
		StoneASC->GetOwnedGameplayTags(OwnedNow);

		FGameplayTagContainer StateTags;
		for (const FGameplayTag& Tag : OwnedNow)
		{
			if (Tag.MatchesTag(StateRoot))
			{
				StateTags.AddTag(Tag);
			}
		}

		if (StateTags.Num() > 0)
		{
			StoneASC->RemoveLooseGameplayTags(StateTags);
		}
	}

	if (SettlerData.SettlerTags.Num() > 0)
	{
		FGameplayTagContainer OwnedNow;
		StoneASC->GetOwnedGameplayTags(OwnedNow);

		const FGameplayTagContainer ToRemove = FilterPersistedTags(OwnedNow);
		if (ToRemove.Num() > 0)
		{
			StoneASC->RemoveLooseGameplayTags(ToRemove);
		}

		const FGameplayTagContainer ToApply = FilterPersistedTags(SettlerData.SettlerTags);
		if (ToApply.Num() > 0)
		{
			StoneASC->AddLooseGameplayTags(ToApply);
		}
	}

	// -------------------------
	// Attributes: tag-driven, covers all attribute groups
	// -------------------------
	const UStoneAttributeSet* AttrSet = CastChecked<UStoneAttributeSet>(AttributeSet);

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

	// -------------------------
	// Abilities
	// -------------------------
	for (const FSavedAbility& SavedAbility : SettlerData.GrantedAbilities)
	{
		if (SavedAbility.GameplayAbility)
		{
			StoneASC->GiveAbility(FGameplayAbilitySpec(SavedAbility.GameplayAbility, SavedAbility.AbilityLevel, INDEX_NONE));
		}
	}

	// --- Action resume (SSOT) ---
	if (ActionComponent)
	{
		const bool bRestored = ActionComponent->RestoreFromSavedActionState(SettlerData.ActionState);

		if (bRestored)
		{
			// Deterministic BT state restore from the action phase.
			switch (ActionComponent->GetPhase())
			{
			case EStoneActionPhase::Outbound: SetState_TravelToActionStart(); break;
			case EStoneActionPhase::Return:   SetState_TravelReturning();     break;
			case EStoneActionPhase::Arrival:  // fallthrough
			default:                          SetState_ActionRunning();      break;
			}
		}
		else
		{
			// Never keep transient travel/action states after load without runtime/def
			SetState_Idle();
		}
	}

	bDidApplySavedState = true;
}


TArray<FSavedAttribute> AStoneSettlerChar::GatherCurrentAttributes() const
{
	TArray<FSavedAttribute> Result;

	if (!HasAuthority())
	{
		return Result;
	}

	const UStoneAbilitySystemComponent* StoneASC = Cast<UStoneAbilitySystemComponent>(AbilitySystemComponent);
	const UStoneAttributeSet* AttrSet = Cast<UStoneAttributeSet>(AttributeSet);

	if (!StoneASC || !AttrSet)
	{
		return Result;
	}

	// Tag-driven iteration covers Primary, Secondary, Vital, Culture, Knowledge, Worldline.
	// No settler-specific hardcoding needed.
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

		// Skip transient meta attributes — never persisted.
		if (Attr.AttributeName.StartsWith(TEXT("Incoming")))
		{
			continue;
		}

		Result.Add(FSavedAttribute(Tag, StoneASC->GetNumericAttribute(Attr)));
	}

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
