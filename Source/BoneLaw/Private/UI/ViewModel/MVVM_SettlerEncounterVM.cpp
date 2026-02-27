// Copyright by MykeUhu

#include "UI/ViewModel/MVVM_SettlerEncounterVM.h"

// Project
#include "Core/Character/StoneSettlerChar.h"
#include "Core/Components/StoneSettlerActionComponent.h"
#include "Data/StoneEventData.h"
#include "Game/Events/StoneEventResolver.h"
#include "Game/StoneOutcomeExecutor.h"
#include "Runtime/StoneRosterSubsystem.h"

// GAS
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

// Engine
#include "Engine/World.h"

void UMVVM_SettlerEncounterVM::BindToEncounter(AStoneSettlerChar* SettlerActor, const UStoneEventData* EventData)
{
	if (!SettlerActor || !EventData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EncounterVM] BindToEncounter invalid args. Settler=%s Event=%s"),
			*GetNameSafe(SettlerActor), *GetNameSafe(EventData));
		return;
	}

	BoundSettler = SettlerActor;
	BoundEvent   = EventData;

	BoundActionComp = SettlerActor->FindComponentByClass<UStoneSettlerActionComponent>();
	if (BoundActionComp)
	{
		BoundActionComp->OnEncounterClosed.AddDynamic(this, &UMVVM_SettlerEncounterVM::HandleEncounterClosed);
	}

	// Identity (SSOT via RosterSubsystem) – same idea as SlotDetails
	if (UWorld* World = SettlerActor->GetWorld())
	{
		if (UStoneRosterSubsystem* Roster = World->GetSubsystem<UStoneRosterSubsystem>())
		{
			// Wenn du eine direkte GUID am Settler hast -> HIER verwenden.
			// (Ich lasse es neutral, weil ich nicht raten will.)
			// Fallback:
			SetSettlerName(SettlerActor->GetActorLabel());
		}
		else
		{
			SetSettlerName(SettlerActor->GetActorLabel());
		}
	}

	// Push MVVM fields (correct types!)
	SetEventTitle(EventData->Title.ToString());
	SetEventBody(EventData->Body);

	RebuildChoices();
	SetbIsVisible(true);

	UE_LOG(LogTemp, Log, TEXT("[EncounterVM] Bound '%s' Owner=%s NumChoices(UI)=%d"),
		*GetNameSafe(EventData), *GetNameSafe(SettlerActor), NumChoices);
}

void UMVVM_SettlerEncounterVM::BeginDestroy()
{
	Super::BeginDestroy();
}

void UMVVM_SettlerEncounterVM::SelectChoice(int32 ChoiceIndex)
{
	if (!BoundActionComp || !BoundEvent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EncounterVM] SelectChoice ignored (not bound)."));
		return;
	}

	// UI index -> raw index
	const int32 RawIndex = ToRawChoiceIndex(ChoiceIndex);
	if (!BoundEvent->Choices.IsValidIndex(RawIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[EncounterVM] SelectChoice(%d) invalid raw index %d."), ChoiceIndex, RawIndex);
		return;
	}

	// Let BP apply outcomes using the SAME UI index (stable)
	OnChoiceSelected.Broadcast(ChoiceIndex);

	// Then close encounter (your action component only needs aborted flag)
	BoundActionComp->ResolveCurrentEncounter(false);
}

void UMVVM_SettlerEncounterVM::ApplyChoiceOutcomes(int32 ChoiceIndex)
{
	if (!BoundEvent || !IsValid(BoundSettler.Get()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[EncounterVM] ApplyChoiceOutcomes(%d) - not bound (Event=%s Settler=%s)."),
			ChoiceIndex, *GetNameSafe(BoundEvent.Get()), *GetNameSafe(BoundSettler.Get()));
		return;
	}

	const TArray<FStoneOutcome> Outcomes = GetChoiceOutcomesAtIndex(ChoiceIndex);
	if (Outcomes.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("[EncounterVM] ApplyChoiceOutcomes(%d) - no outcomes defined, skipping."), ChoiceIndex);
		return;
	}

	FStoneOutcomeContext Ctx;

	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(BoundSettler.Get()))
	{
		Ctx.ASC = ASI->GetAbilitySystemComponent();
	}

	if (!IsValid(Ctx.ASC))
	{
		UE_LOG(LogTemp, Warning, TEXT("[EncounterVM] ApplyChoiceOutcomes(%d) - Settler ASC invalid for '%s'."),
			ChoiceIndex, *GetNameSafe(BoundSettler.Get()));
		return;
	}

	Ctx.SourceObject = BoundSettler.Get();

	UStoneOutcomeExecutor* Executor = NewObject<UStoneOutcomeExecutor>(this);
	if (Executor)
	{
		Executor->ApplyOutcomes(Outcomes, Ctx);
		UE_LOG(LogTemp, Log, TEXT("[EncounterVM] ApplyChoiceOutcomes(%d) applied %d outcomes to '%s'."),
			ChoiceIndex, Outcomes.Num(), *GetNameSafe(BoundSettler.Get()));
	}
}

void UMVVM_SettlerEncounterVM::HandleEncounterClosed(bool bAborted)
{
	OnEncounterClosed.Broadcast(bAborted);
}

int32 UMVVM_SettlerEncounterVM::ToRawChoiceIndex(int32 UiIndex) const
{
	return VisibleChoiceIndices.IsValidIndex(UiIndex) ? VisibleChoiceIndices[UiIndex] : INDEX_NONE;
}

void UMVVM_SettlerEncounterVM::RebuildChoices()
{
	ResolvedChoices.Reset();
	VisibleChoiceIndices.Reset();
	SetNumChoices(0);

	if (!BoundEvent || BoundEvent->Choices.Num() == 0)
	{
		return;
	}

	UAbilitySystemComponent* SettlerASC = nullptr;
	if (BoundSettler)
	{
		if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(BoundSettler.Get()))
		{
			SettlerASC = ASI->GetAbilitySystemComponent();
		}
	}

	FGameplayTagContainer CurrentTags;
	if (SettlerASC)
	{
		SettlerASC->GetOwnedGameplayTags(CurrentTags);
	}

	// IMPORTANT: UStoneEventResolver is a UObject -> use default object (no allocations)
	const UStoneEventResolver* Resolver = GetDefault<UStoneEventResolver>();
	if (!Resolver)
	{
		return;
	}

	Resolver->ResolveChoices(BoundEvent, SettlerASC, CurrentTags, ResolvedChoices);

	// Build UI mapping: only visible entries become UI indices 0..N-1
	for (int32 RawIdx = 0; RawIdx < ResolvedChoices.Num(); ++RawIdx)
	{
		if (ResolvedChoices[RawIdx].bVisible)
		{
			VisibleChoiceIndices.Add(RawIdx);
		}
	}

	SetNumChoices(VisibleChoiceIndices.Num());
}

// Choice helpers (UI index!)
FText UMVVM_SettlerEncounterVM::GetChoiceTextAtIndex(int32 Index) const
{
	const int32 Raw = ToRawChoiceIndex(Index);
	return (BoundEvent && BoundEvent->Choices.IsValidIndex(Raw)) ? BoundEvent->Choices[Raw].ChoiceText : FText::GetEmpty();
}

TArray<FStoneOutcome> UMVVM_SettlerEncounterVM::GetChoiceOutcomesAtIndex(int32 Index) const
{
	const int32 Raw = ToRawChoiceIndex(Index);
	return (BoundEvent && BoundEvent->Choices.IsValidIndex(Raw)) ? BoundEvent->Choices[Raw].Outcomes : TArray<FStoneOutcome>();
}

bool UMVVM_SettlerEncounterVM::IsChoiceLockedAtIndex(int32 Index) const
{
	const int32 Raw = ToRawChoiceIndex(Index);
	return !ResolvedChoices.IsValidIndex(Raw) ? true : !ResolvedChoices[Raw].bEnabled;
}

bool UMVVM_SettlerEncounterVM::IsChoiceSoftFailAtIndex(int32 Index) const
{
	const int32 Raw = ToRawChoiceIndex(Index);
	return ResolvedChoices.IsValidIndex(Raw) ? ResolvedChoices[Raw].bSoftFail : false;
}

FText UMVVM_SettlerEncounterVM::GetChoiceDisabledReasonAtIndex(int32 Index) const
{
	const int32 Raw = ToRawChoiceIndex(Index);
	return ResolvedChoices.IsValidIndex(Raw) ? ResolvedChoices[Raw].DisabledReason : FText::GetEmpty();
}

// MVVM setters (types match fields!)
void UMVVM_SettlerEncounterVM::SetEventTitle(const FString InTitle)      { UE_MVVM_SET_PROPERTY_VALUE(EventTitle, InTitle); }
void UMVVM_SettlerEncounterVM::SetEventBody(const FText InBody)        { UE_MVVM_SET_PROPERTY_VALUE(EventBody, InBody); }
void UMVVM_SettlerEncounterVM::SetSettlerName(const FString InName)    { UE_MVVM_SET_PROPERTY_VALUE(SettlerName, InName); }
void UMVVM_SettlerEncounterVM::SetNumChoices(int32 InNum)               { UE_MVVM_SET_PROPERTY_VALUE(NumChoices, InNum); }
void UMVVM_SettlerEncounterVM::SetbIsVisible(bool bInVisible)            { UE_MVVM_SET_PROPERTY_VALUE(bIsVisible, bInVisible); }
