// Copyright by MykeUhu

#include "Runtime/StoneRosterSubsystem.h"

// Engine
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"

// Project
#include "Core/StoneGameMode.h"
#include "Core/Character/StoneBaseChar.h"
#include "AbilitySystem/StoneAbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "Core/StoneGameplayTags.h"

void UStoneRosterSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("[StoneRoster] Subsystem initialized"));
}

void UStoneRosterSubsystem::Deinitialize()
{
	// Clean up any spawned settlers
	for (FSettlerRuntimeState& State : SettlerStates)
	{
		if (State.SpawnedPawn.IsValid())
		{
			State.SpawnedPawn->Destroy();
		}
	}
	SettlerStates.Empty();
	
	Super::Deinitialize();
}

// ========================================================================
// ROSTER MANAGEMENT
// ========================================================================

void UStoneRosterSubsystem::InitializeRoster(const TArray<FSavedSettler>& SavedSettlers)
{
	UE_LOG(LogTemp, Log, TEXT("[StoneRoster] Initializing roster with %d settlers"), SavedSettlers.Num());
	
	// Clear existing roster
	for (FSettlerRuntimeState& State : SettlerStates)
	{
		if (State.SpawnedPawn.IsValid())
		{
			State.SpawnedPawn->Destroy();
		}
	}
	SettlerStates.Empty();
	
	// Add all settlers
	for (const FSavedSettler& Settler : SavedSettlers)
	{
		if (!Settler.SettlerId.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneRoster] Skipping settler with invalid ID"));
			continue;
		}
		
		FSettlerRuntimeState NewState;
		NewState.SettlerId = Settler.SettlerId;
		NewState.Data = Settler;
		NewState.SpawnedPawn = nullptr; // Lazy spawn
		
		SettlerStates.Add(NewState);
		
		UE_LOG(LogTemp, Log, TEXT("[StoneRoster] Added settler %s (%s)"), 
			*Settler.DisplayName, *Settler.SettlerId.ToString());
	}
	
	OnRosterChanged.Broadcast();
}

AStoneBaseChar* UStoneRosterSubsystem::GetOrSpawnSettlerPawn(const FGuid& SettlerId)
{
	FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	if (!State)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneRoster] Settler %s not found in roster"), *SettlerId.ToString());
		return nullptr;
	}
	
	// If already spawned, return it
	if (State->SpawnedPawn.IsValid())
	{
		return State->SpawnedPawn.Get();
	}
	
	// Spawn new pawn
	AStoneBaseChar* NewPawn = SpawnSettlerPawn(State->Data);
	if (NewPawn)
	{
		State->SpawnedPawn = NewPawn;
		UE_LOG(LogTemp, Log, TEXT("[StoneRoster] Spawned pawn for settler %s"), *SettlerId.ToString());
	}
	
	return NewPawn;
}

FSavedSettler UStoneRosterSubsystem::GetSettlerInfo(const FGuid& SettlerId) const
{
	const FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	if (!State)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneRoster] Settler %s not found"), *SettlerId.ToString());
		return FSavedSettler();
	}
	
	// If pawn exists, gather live state
	if (State->SpawnedPawn.IsValid())
	{
		return GatherSettlerStateFromPawn(SettlerId, State->SpawnedPawn.Get());
	}
	
	// Otherwise return saved data
	return State->Data;
}

TArray<FGuid> UStoneRosterSubsystem::GetAllSettlerIds() const
{
	TArray<FGuid> Ids;
	for (const FSettlerRuntimeState& State : SettlerStates)
	{
		Ids.Add(State.SettlerId);
	}
	return Ids;
}

TArray<FGuid> UStoneRosterSubsystem::GetAvailableSettlerIds() const
{
	TArray<FGuid> AvailableIds;
	for (const FSettlerRuntimeState& State : SettlerStates)
	{
		if (!State.Data.bHasAssignment)
		{
			AvailableIds.Add(State.SettlerId);
		}
	}
	return AvailableIds;
}

bool UStoneRosterSubsystem::HasSettler(const FGuid& SettlerId) const
{
	return FindSettlerState(SettlerId) != nullptr;
}

// ========================================================================
// ASSIGNMENT MANAGEMENT
// ========================================================================

bool UStoneRosterSubsystem::AssignSettlerToTask(const FGuid& SettlerId, const FGuid& TaskActorId, 
	const FGameplayTag& TaskTag, double DurationSeconds)
{
	FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	if (!State)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneRoster] Cannot assign: Settler %s not found"), *SettlerId.ToString());
		return false;
	}
	
	if (State->Data.bHasAssignment)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneRoster] Settler %s already has assignment"), *SettlerId.ToString());
		return false;
	}
	
	// Create assignment
	FSavedAssignment NewAssignment;
	NewAssignment.AssignmentId = FGuid::NewGuid();
	NewAssignment.TaskTag = TaskTag;
	NewAssignment.TaskActorId = TaskActorId;
	NewAssignment.StartTimeSeconds = GetWorld()->GetTimeSeconds();
	NewAssignment.DurationSeconds = DurationSeconds;
	NewAssignment.bIsAwayWorking = false;
	NewAssignment.ElapsedSeconds = 0.0;
	
	// Store return transform (current location or default)
	if (State->SpawnedPawn.IsValid())
	{
		NewAssignment.ReturnTransform = State->SpawnedPawn->GetActorTransform();
	}
	else
	{
		NewAssignment.ReturnTransform = State->Data.LastKnownTransform;
	}
	
	State->Data.CurrentAssignment = NewAssignment;
	State->Data.bHasAssignment = true;
	
	UE_LOG(LogTemp, Log, TEXT("[StoneRoster] Assigned settler %s to task %s (duration %.0fs)"), 
		*SettlerId.ToString(), *TaskTag.ToString(), DurationSeconds);
	
	OnSettlerAssignmentChanged.Broadcast(SettlerId, true);
	
	// TODO: Start navigation to task location
	// For now, immediately mark as "away working"
	State->Data.CurrentAssignment.bIsAwayWorking = true;
	
	return true;
}

void UStoneRosterSubsystem::CompleteAssignment(const FGuid& SettlerId, bool bSuccess)
{
	FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	if (!State || !State->Data.bHasAssignment)
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[StoneRoster] Completing assignment for settler %s (success: %d)"), 
		*SettlerId.ToString(), bSuccess);
	
	// Return settler to home transform
	if (State->SpawnedPawn.IsValid())
	{
		State->SpawnedPawn->SetActorTransform(State->Data.CurrentAssignment.ReturnTransform);
	}
	
	// Clear assignment
	State->Data.bHasAssignment = false;
	State->Data.CurrentAssignment = FSavedAssignment();
	
	OnSettlerAssignmentChanged.Broadcast(SettlerId, false);
}

void UStoneRosterSubsystem::CancelAssignment(const FGuid& SettlerId)
{
	CompleteAssignment(SettlerId, false);
}

bool UStoneRosterSubsystem::HasActiveAssignment(const FGuid& SettlerId) const
{
	const FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	return State && State->Data.bHasAssignment;
}

FSavedAssignment UStoneRosterSubsystem::GetAssignment(const FGuid& SettlerId) const
{
	const FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	if (State && State->Data.bHasAssignment)
	{
		return State->Data.CurrentAssignment;
	}
	return FSavedAssignment();
}

// ========================================================================
// RUNTIME LOOKUPS
// ========================================================================

UStoneAbilitySystemComponent* UStoneRosterSubsystem::GetSettlerASC(const FGuid& SettlerId) const
{
	const FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	if (!State || !State->SpawnedPawn.IsValid())
	{
		return nullptr;
	}
	
	return Cast<UStoneAbilitySystemComponent>(State->SpawnedPawn->GetAbilitySystemComponent());
}

TArray<FSavedAttribute> UStoneRosterSubsystem::GetSettlerAttributesSnapshot(const FGuid& SettlerId) const
{
	TArray<FSavedAttribute> Snapshot;
	
	UStoneAbilitySystemComponent* ASC = GetSettlerASC(SettlerId);
	if (!ASC)
	{
		return Snapshot;
	}
	
	const UStoneAttributeSet* AttrSet = Cast<UStoneAttributeSet>(ASC->GetAttributeSet(UStoneAttributeSet::StaticClass()));
	if (!AttrSet)
	{
		return Snapshot;
	}
	
	// Gather vital attributes
	const FStoneGameplayTags& Tags = FStoneGameplayTags::Get();
	Snapshot.Add(FSavedAttribute(Tags.Attributes_Vital_Health, AttrSet->GetHealth()));
	Snapshot.Add(FSavedAttribute(Tags.Attributes_Vital_Food, AttrSet->GetFood()));
	Snapshot.Add(FSavedAttribute(Tags.Attributes_Vital_Water, AttrSet->GetWater()));
	Snapshot.Add(FSavedAttribute(Tags.Attributes_Vital_Morale, AttrSet->GetMorale()));
	Snapshot.Add(FSavedAttribute(Tags.Attributes_Vital_Trust, AttrSet->GetTrust()));
	
	// Add more as needed...
	
	return Snapshot;
}

// ========================================================================
// PERSISTENCE
// ========================================================================

TArray<FSavedSettler> UStoneRosterSubsystem::GatherRosterState() const
{
	TArray<FSavedSettler> SavedRoster;
	
	for (const FSettlerRuntimeState& State : SettlerStates)
	{
		if (State.SpawnedPawn.IsValid())
		{
			// Gather live state from pawn
			SavedRoster.Add(GatherSettlerStateFromPawn(State.SettlerId, State.SpawnedPawn.Get()));
		}
		else
		{
			// Use saved data
			SavedRoster.Add(State.Data);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[StoneRoster] Gathered %d settlers for save"), SavedRoster.Num());
	return SavedRoster;
}

void UStoneRosterSubsystem::ApplyRosterState(const TArray<FSavedSettler>& SavedSettlers)
{
	InitializeRoster(SavedSettlers);
}

// ========================================================================
// PRIVATE HELPERS
// ========================================================================

UStoneRosterSubsystem::FSettlerRuntimeState* UStoneRosterSubsystem::FindSettlerState(const FGuid& SettlerId)
{
	return SettlerStates.FindByPredicate([&SettlerId](const FSettlerRuntimeState& State)
	{
		return State.SettlerId == SettlerId;
	});
}

const UStoneRosterSubsystem::FSettlerRuntimeState* UStoneRosterSubsystem::FindSettlerState(const FGuid& SettlerId) const
{
	return SettlerStates.FindByPredicate([&SettlerId](const FSettlerRuntimeState& State)
	{
		return State.SettlerId == SettlerId;
	});
}

AStoneBaseChar* UStoneRosterSubsystem::SpawnSettlerPawn(const FSavedSettler& SettlerData)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRosterSubsystem] SpawnSettlerPawn: World is null."));
		return nullptr;
	}

	AStoneGameMode* GM = World->GetAuthGameMode<AStoneGameMode>();
	if (!GM || !GM->DefaultSettlerClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRosterSubsystem] DefaultSettlerClass not set on GameMode."));
		return nullptr;
	}

	// Prefer saved transform if valid; otherwise spawn near a PlayerStart.
	FTransform SpawnXform = SettlerData.LastKnownTransform;

	const FVector Loc = SpawnXform.GetLocation();

	// "Validity" check that compiles in your build (no IsFinite on TVector<double>).
	const bool bFinite =
		FMath::IsFinite(Loc.X) &&
		FMath::IsFinite(Loc.Y) &&
		FMath::IsFinite(Loc.Z);

	const bool bBadLoc = Loc.IsNearlyZero() || !bFinite || Loc.ContainsNaN();

	if (bBadLoc)
	{
		APlayerStart* PlayerStart = nullptr;

		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			PlayerStart = *It;
			break;
		}

		if (PlayerStart)
		{
			SpawnXform = PlayerStart->GetActorTransform();

			// Offset so we don't spawn inside the start
			FVector Offset = SpawnXform.GetRotation().GetForwardVector() * 250.f;
			Offset.Z = 0.f;
			SpawnXform.AddToTranslation(Offset);

			UE_LOG(LogTemp, Log, TEXT("[StoneRosterSubsystem] Using PlayerStart fallback spawn transform for settler."));
		}
		else
		{
			SpawnXform = FTransform(FRotator::ZeroRotator, FVector(0.f, 0.f, 200.f), FVector::OneVector);
			UE_LOG(LogTemp, Warning, TEXT("[StoneRosterSubsystem] No PlayerStart found. Using hard fallback (0,0,200)."));
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AStoneBaseChar* NewPawn = World->SpawnActor<AStoneBaseChar>(GM->DefaultSettlerClass, SpawnXform, SpawnParams);
	if (!NewPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRosterSubsystem] Failed to spawn settler pawn."));
		return nullptr;
	}

	ApplySettlerStateToPawn(NewPawn, SettlerData);

	UE_LOG(LogTemp, Log, TEXT("[StoneRosterSubsystem] Spawned settler pawn: %s at %s"),
		*NewPawn->GetName(), *NewPawn->GetActorLocation().ToString());

	return NewPawn;
}

void UStoneRosterSubsystem::ApplySettlerStateToPawn(AStoneBaseChar* Pawn, const FSavedSettler& SettlerData)
{
	if (!Pawn)
	{
		return;
	}
	
	UStoneAbilitySystemComponent* ASC = Cast<UStoneAbilitySystemComponent>(Pawn->GetAbilitySystemComponent());
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRoster] Pawn has no ASC!"));
		return;
	}

	// Ensure GAS actor info is initialized before applying attributes/tags/abilities.
	ASC->InitAbilityActorInfo(Pawn, Pawn);
	
	// Apply saved tags
	if (SettlerData.SettlerTags.Num() > 0)
	{
		ASC->AddLooseGameplayTags(SettlerData.SettlerTags);
	}
	
	// Apply saved attributes
	const UStoneAttributeSet* AttrSet = Cast<UStoneAttributeSet>(ASC->GetAttributeSet(UStoneAttributeSet::StaticClass()));
	if (AttrSet)
	{
		for (const FSavedAttribute& SavedAttr : SettlerData.Attributes)
		{
			if (!SavedAttr.AttributeTag.IsValid())
			{
				continue;
			}

			FGameplayAttribute GameplayAttr;
			if (AttrSet->GetAttributeFromTag(SavedAttr.AttributeTag, GameplayAttr))
			{
				ASC->SetNumericAttributeBase(GameplayAttr, SavedAttr.Value);
			}
		}
	}
	
	// Grant saved abilities
	for (const FSavedAbility& SavedAbility : SettlerData.GrantedAbilities)
	{
		if (SavedAbility.GameplayAbility)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(SavedAbility.GameplayAbility, SavedAbility.AbilityLevel, INDEX_NONE));
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[StoneRoster] Applied state to pawn: %d tags, %d attributes, %d abilities"),
		SettlerData.SettlerTags.Num(), SettlerData.Attributes.Num(), SettlerData.GrantedAbilities.Num());
}

FSavedSettler UStoneRosterSubsystem::GatherSettlerStateFromPawn(const FGuid& SettlerId, AStoneBaseChar* Pawn) const
{
	FSavedSettler Gathered;
	Gathered.SettlerId = SettlerId;

	if (!Pawn)
	{
		return Gathered;
	}

	Gathered.DisplayName = Pawn->GetName();
	Gathered.LastKnownTransform = Pawn->GetActorTransform();

	UStoneAbilitySystemComponent* ASC = Cast<UStoneAbilitySystemComponent>(Pawn->GetAbilitySystemComponent());
	if (!ASC)
	{
		return Gathered;
	}

	ASC->GetOwnedGameplayTags(Gathered.SettlerTags);

	const UStoneAttributeSet* AttrSet = Cast<UStoneAttributeSet>(ASC->GetAttributeSet(UStoneAttributeSet::StaticClass()));
	if (AttrSet)
	{
		for (const auto& Pair : AttrSet->TagsToAttributes)
		{
			FGameplayAttribute Attr = Pair.Value();
			if (Attr.IsValid())
			{
				const float Value = ASC->GetNumericAttribute(Attr);
				Gathered.Attributes.Add(FSavedAttribute(Pair.Key, Value));
			}
		}
	}

	// Abilities: in deinem Projekt ist das ein TArray (laut Compiler).
	const TArray<FGameplayAbilitySpec> Specs = ASC->GetActivatableAbilities();
	for (const FGameplayAbilitySpec& Spec : Specs)
	{
		if (!Spec.Ability)
		{
			continue;
		}

		FSavedAbility SavedAbility;
		SavedAbility.GameplayAbility = Spec.Ability->GetClass();
		SavedAbility.AbilityLevel = Spec.Level;

		Gathered.GrantedAbilities.Add(SavedAbility);
	}

	const FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	if (State)
	{
		Gathered.bHasAssignment = State->Data.bHasAssignment;
		Gathered.CurrentAssignment = State->Data.CurrentAssignment;
	}

	return Gathered;
}

void UStoneRosterSubsystem::StartSettlerNavigation(const FGuid& SettlerId, const FVector& TargetLocation)
{
	// TODO: Implement navigation/pathfinding
	// For Phase 2, this is a stub - actual movement implementation depends on game requirements
	UE_LOG(LogTemp, Log, TEXT("[StoneRoster] TODO: Start navigation for settler %s to %s"), 
		*SettlerId.ToString(), *TargetLocation.ToString());
}

void UStoneRosterSubsystem::OnSettlerArrival(const FGuid& SettlerId)
{
	// TODO: Called when settler reaches task location
	// Mark as "away working", start action timer, etc.
	UE_LOG(LogTemp, Log, TEXT("[StoneRoster] TODO: Settler %s arrived at task location"), 
		*SettlerId.ToString());
}
