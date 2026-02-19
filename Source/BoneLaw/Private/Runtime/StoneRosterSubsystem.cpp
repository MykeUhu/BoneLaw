// Copyright by MykeUhu

#include "Runtime/StoneRosterSubsystem.h"

// Engine
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

// Project
#include "Core/Character/StoneBaseChar.h"
#include "Core/Character/StoneSettlerChar.h"
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
			FSavedSettler Saved = State.Data; // SSOT
			Saved.LastKnownTransform = State.SpawnedPawn->GetActorTransform();
			Out.Add(Saved);
		}
		else
		{
			Out.Add(State.Data);
		}
	}

	return Out;
}

void UStoneRosterSubsystem::ApplyRosterState(const TArray<FSavedSettler>& SavedSettlers)
{
	InitializeRoster(SavedSettlers);
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
	FSavedSettler Gathered;
	Gathered.SettlerId = SettlerId;
	if (!Pawn) return Gathered;

	// DisplayName NICHT von Pawn->GetName()!
	Gathered.LastKnownTransform = Pawn->GetActorTransform();
	return Gathered;
}

void UStoneRosterSubsystem::StartSettlerNavigation(const FGuid& SettlerId, const FVector& TargetLocation)
{
}

void UStoneRosterSubsystem::OnSettlerArrival(const FGuid& SettlerId)
{
}
