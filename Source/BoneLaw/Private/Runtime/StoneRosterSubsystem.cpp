// Copyright by MykeUhu

#include "Runtime/StoneRosterSubsystem.h"

// Project
#include "Core/Character/StoneSettlerChar.h"
#include "Core/Components/StoneSettlerActionComponent.h"
// Engine
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
#include "Core/Character/StoneBaseChar.h"
#include "Core/GameMode/StoneGameModeBase.h"

void UStoneRosterSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("[StoneRoster] Subsystem initialized"));
}

void UStoneRosterSubsystem::Deinitialize()
{
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

void UStoneRosterSubsystem::InitializeRoster(const TArray<FSavedSettler>& SavedSettlers)
{
	UE_LOG(LogTemp, Log, TEXT("[StoneRoster] Initializing roster with %d settlers"), SavedSettlers.Num());

	for (FSettlerRuntimeState& State : SettlerStates)
	{
		if (State.SpawnedPawn.IsValid())
		{
			State.SpawnedPawn->Destroy();
		}
	}
	SettlerStates.Empty();

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
		NewState.SpawnedPawn = nullptr;

		SettlerStates.Add(NewState);
	}

	OnRosterChanged.Broadcast();
}

AStoneBaseChar* UStoneRosterSubsystem::GetOrSpawnSettlerPawn(const FGuid& SettlerId)
{
	FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	if (!State)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneRoster] Settler not found: %s"), *SettlerId.ToString());
		return nullptr;
	}

	if (State->SpawnedPawn.IsValid())
	{
		return State->SpawnedPawn.Get();
	}

	AStoneBaseChar* NewPawn = SpawnSettlerPawn(State->Data);
	if (NewPawn)
	{
		State->SpawnedPawn = NewPawn;
	}
	
	if (AStoneSettlerChar* SettlerPawn = Cast<AStoneSettlerChar>(NewPawn))
	{
		// MP-friendly: replicate identity so clients can display name even if roster isn’t locally populated.
		SettlerPawn->SetRosterIdentity(SettlerId, State->Data.DisplayName);
	}

	return NewPawn;
}

FSavedSettler UStoneRosterSubsystem::GetSettlerInfo(const FGuid& SettlerId) const
{
	const FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	if (!State)
	{
		return FSavedSettler();
	}

	FSavedSettler Out = State->Data; // SSOT: enthält DisplayName etc.

	if (State->SpawnedPawn.IsValid())
	{
		Out.LastKnownTransform = State->SpawnedPawn->GetActorTransform();
	}

	return Out;
}

TArray<FGuid> UStoneRosterSubsystem::GetAllSettlerIds() const
{
	TArray<FGuid> Ids;
	Ids.Reserve(SettlerStates.Num());
	for (const FSettlerRuntimeState& State : SettlerStates)
	{
		Ids.Add(State.SettlerId);
	}
	return Ids;
}

TArray<FGuid> UStoneRosterSubsystem::GetAvailableSettlerIds() const
{
	TArray<FGuid> Available;
	for (const FSettlerRuntimeState& State : SettlerStates)
	{
		if (!State.Data.bHasAssignment)
		{
			Available.Add(State.SettlerId);
		}
	}
	return Available;
}

bool UStoneRosterSubsystem::HasSettler(const FGuid& SettlerId) const
{
	return FindSettlerState(SettlerId) != nullptr;
}

bool UStoneRosterSubsystem::AssignSettlerToTask(const FGuid& SettlerId, const FGuid& TaskActorId, const FGameplayTag& TaskTag, double DurationSeconds)
{
	FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	if (!State || State->Data.bHasAssignment)
	{
		return false;
	}

	FSavedAssignment NewAssignment;
	NewAssignment.AssignmentId = FGuid::NewGuid();
	NewAssignment.TaskTag = TaskTag;
	NewAssignment.TaskActorId = TaskActorId;
	NewAssignment.StartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	NewAssignment.DurationSeconds = DurationSeconds;
	NewAssignment.bIsAwayWorking = true;
	NewAssignment.ElapsedSeconds = 0.0;
	NewAssignment.ReturnTransform = State->SpawnedPawn.IsValid() ? State->SpawnedPawn->GetActorTransform() : State->Data.LastKnownTransform;

	State->Data.CurrentAssignment = NewAssignment;
	State->Data.bHasAssignment = true;

	OnSettlerAssignmentChanged.Broadcast(SettlerId, true);
	return true;
}

void UStoneRosterSubsystem::CompleteAssignment(const FGuid& SettlerId, bool bSuccess)
{
	FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	if (!State || !State->Data.bHasAssignment)
	{
		return;
	}

	if (State->SpawnedPawn.IsValid())
	{
		State->SpawnedPawn->SetActorTransform(State->Data.CurrentAssignment.ReturnTransform);
	}

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
	return (State && State->Data.bHasAssignment) ? State->Data.CurrentAssignment : FSavedAssignment();
}

TArray<FSavedSettler> UStoneRosterSubsystem::GatherRosterState() const
{
	TArray<FSavedSettler> Out;
	Out.Reserve(SettlerStates.Num());

	for (const FSettlerRuntimeState& State : SettlerStates)
	{
		if (State.SpawnedPawn.IsValid())
		{
			// Full gather: transform + live attributes + loose tags via pawn API.
			// Cast away const only for the pawn pointer - GatherSettlerStateFromPawn reads, never writes.
			AStoneBaseChar* PawnPtr = State.SpawnedPawn.Get();
			FSavedSettler Saved = GatherSettlerStateFromPawn(State.SettlerId, PawnPtr);
			Out.Add(Saved);
		}
		else
		{
			// Pawn not spawned: keep last known data as-is (offline settler).
			Out.Add(State.Data);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[StoneRoster] GatherRosterState: gathered %d settlers."), Out.Num());
	return Out;
}

void UStoneRosterSubsystem::ApplyRosterState(const TArray<FSavedSettler>& SavedSettlers)
{
	InitializeRoster(SavedSettlers);
}

FGuid UStoneRosterSubsystem::GetSettlerIdByPawn(const AStoneBaseChar* Pawn) const
{
	if (!Pawn)
	{
		return FGuid();
	}

	for (const FSettlerRuntimeState& State : SettlerStates)
	{
		if (State.SpawnedPawn.IsValid() && State.SpawnedPawn.Get() == Pawn)
		{
			return State.SettlerId;
		}
	}

	return FGuid();
}

FString UStoneRosterSubsystem::GetSettlerDisplayNameByGuid(const FGuid& SettlerId) const
{
	const FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	if (!State)
	{
		return FString();
	}

	return State->Data.DisplayName;
}

FString UStoneRosterSubsystem::GetSettlerDisplayNameByPawn(const AStoneBaseChar* Pawn) const
{
	if (!Pawn)
	{
		return FString();
	}

	for (const FSettlerRuntimeState& State : SettlerStates)
	{
		if (State.SpawnedPawn.IsValid() && State.SpawnedPawn.Get() == Pawn)
		{
			return State.Data.DisplayName;
		}
	}

	return FString();
}

UStoneRosterSubsystem::FSettlerRuntimeState* UStoneRosterSubsystem::FindSettlerState(const FGuid& SettlerId)
{
	return SettlerStates.FindByPredicate([&](const FSettlerRuntimeState& S){ return S.SettlerId == SettlerId; });
}

const UStoneRosterSubsystem::FSettlerRuntimeState* UStoneRosterSubsystem::FindSettlerState(const FGuid& SettlerId) const
{
	return SettlerStates.FindByPredicate([&](const FSettlerRuntimeState& S){ return S.SettlerId == SettlerId; });
}

AStoneBaseChar* UStoneRosterSubsystem::SpawnSettlerPawn(const FSavedSettler& SettlerData)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AStoneGameModeBase* GM = World->GetAuthGameMode<AStoneGameModeBase>();
	if (!GM || !GM->DefaultSettlerClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRoster] Cannot spawn settler: DefaultSettlerClass not set on GameMode."));
		return nullptr;
	}

	const FTransform SpawnXform = SettlerData.LastKnownTransform;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AStoneBaseChar* NewPawn = World->SpawnActor<AStoneBaseChar>(GM->DefaultSettlerClass, SpawnXform, Params);
	if (!NewPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRoster] SpawnActor failed for settler."));
		return nullptr;
	}

	ApplySettlerStateToPawn(NewPawn, SettlerData);
	return NewPawn;
}

void UStoneRosterSubsystem::ApplySettlerStateToPawn(AStoneBaseChar* Pawn, const FSavedSettler& SettlerData)
{
	if (!Pawn)
	{
		return;
	}

	// IMPORTANT:
	// - Do NOT init GAS here.
	// - Pawn owns InitAbilityActorInfo + defaults.
	// - We only apply save-data if it actually contains something.
	AStoneSettlerChar* SettlerPawn = Cast<AStoneSettlerChar>(Pawn);
	if (!SettlerPawn)
	{
		// If someone assigns a non-settler class, we won't try to mutate GAS from here.
		return;
	}

	SettlerPawn->ApplySavedState(SettlerData);
}

FSavedSettler UStoneRosterSubsystem::GatherSettlerStateFromPawn(const FGuid& SettlerId, AStoneBaseChar* Pawn) const
{
	// Start from the current SSOT data so DisplayName, SettlerTags, Assignment etc. are preserved.
	const FSettlerRuntimeState* State = FindSettlerState(SettlerId);
	FSavedSettler Gathered = State ? State->Data : FSavedSettler();
	Gathered.SettlerId = SettlerId;

	if (!Pawn)
	{
		return Gathered;
	}

	// Transform: always read live from pawn.
	Gathered.LastKnownTransform = Pawn->GetActorTransform();

	// Persisted tags: NEVER persist GE/transient tags.
	// IMPORTANT: In this project, "State.*" is BT control-flow granted by state GameplayEffects.
	// Persisting State.* as loose tags breaks load (BT decorators stay true forever).
	// Therefore we only persist stable tags like "Status.*".
	if (UAbilitySystemComponent* ASC = Pawn->GetAbilitySystemComponent())
	{
		static const FGameplayTag StatusRoot = UGameplayTagsManager::Get().RequestGameplayTag(FName("Status"), /*ErrorIfNotFound*/ false);

		FGameplayTagContainer OwnedTags;
		ASC->GetOwnedGameplayTags(OwnedTags);

		FGameplayTagContainer Persisted;
		for (const FGameplayTag& Tag : OwnedTags)
		{
			if (StatusRoot.IsValid() && Tag.MatchesTag(StatusRoot))
			{
				Persisted.AddTag(Tag);
			}
		}

		Gathered.SettlerTags = Persisted;

		UE_LOG(LogTemp, Log, TEXT("[StoneRoster] GatherSettlerStateFromPawn: Owned=%d Persisted(Status)=%d Pawn=%s"),
			OwnedTags.Num(), Persisted.Num(), *GetNameSafe(Pawn));
	}

	// Attributes: use the pawn's own gather function (tag-driven, covers all attribute groups).
	if (AStoneSettlerChar* SettlerPawn = Cast<AStoneSettlerChar>(Pawn))
	{
		Gathered.Attributes = SettlerPawn->GatherCurrentAttributes();

		// ActionComponent is SSOT for action resume (prevents BT/BB cast spam after load)
		if (UStoneSettlerActionComponent* ActionComp = SettlerPawn->GetActionComponent())
		{
			ActionComp->BuildSavedActionState(Gathered.ActionState);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[StoneRoster] GatherSettlerStateFromPawn: SettlerId=%s Tags=%d Attributes=%d Pawn=%s"),
		*SettlerId.ToString(EGuidFormats::DigitsWithHyphensLower),
		Gathered.SettlerTags.Num(),
		Gathered.Attributes.Num(),
		*GetNameSafe(Pawn));

	return Gathered;
}

void UStoneRosterSubsystem::StartSettlerNavigation(const FGuid& SettlerId, const FVector& TargetLocation)
{
}

void UStoneRosterSubsystem::OnSettlerArrival(const FGuid& SettlerId)
{
}
