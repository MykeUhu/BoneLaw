#include "Runtime/StoneActionRuntime.h"

#include "AbilitySystemComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Data/StoneEventData.h"
#include "Game/StoneOutcomeExecutor.h"
#include "Game/StoneScheduler.h"
#include "Game/Events/StoneEventResolver.h"

bool UStoneActionRuntime::Init(UAbilitySystemComponent* InASC, int32 RNGSeed)
{
	ASC = InASC;
	UE_LOG(LogTemp, Warning, TEXT("[StoneActionRuntime] Init TargetASC Owner=%s ASC=%s"),
	*GetNameSafe(InASC ? InASC->GetOwner() : nullptr),
	*GetNameSafe(InASC));
	
	
	RNG.Initialize(RNGSeed);

	if (!ASC.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneActionRuntime] Init failed: ASC is null."));
		return false;
	}

	// Cache PrimaryAsset list once (SSOT = AssetManager)
	if (!EnsureEventIdCache())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneActionRuntime] Init failed: could not build StoneEvent cache."));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[StoneActionRuntime] Init OK. CachedStoneEvents=%d Seed=%d"),
		CachedEventIds.Num(), RNGSeed);

	return true;
}

void UStoneActionRuntime::RebuildEventIdCache()
{
	CachedEventIds.Reset();
	EnsureEventIdCache();
}

bool UStoneActionRuntime::EnsureEventIdCache()
{
	if (CachedEventIds.Num() > 0)
	{
		return true;
	}

	UAssetManager& AM = UAssetManager::Get();
	const FPrimaryAssetType StoneEventType = GetStoneEventAssetType();

	AM.GetPrimaryAssetIdList(StoneEventType, CachedEventIds);

	if (CachedEventIds.Num() == 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[StoneActionRuntime] EnsureEventIdCache: No primary assets registered for type '%s'. ")
			TEXT("Check AssetManager settings / PrimaryAssetId on UStoneEventData."),
			*StoneEventType.ToString());
		return false;
	}

	return true;
}

UStoneEventData* UStoneActionRuntime::PickEventByRequiredTags(const FGameplayTagContainer& RequiredTags)
{
	if (RequiredTags.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneActionRuntime] PickEventByRequiredTags: RequiredTags is empty."));
		return nullptr;
	}

	if (!EnsureEventIdCache())
	{
		return nullptr;
	}

	UAssetManager& AM = UAssetManager::Get();
	FStreamableManager& SM = AM.GetStreamableManager();

	TArray<UStoneEventData*> Matching;
	Matching.Reserve(8);

	// NOTE: This preserves your current behavior: synchronous load + HasAll(required).
	for (const FPrimaryAssetId& Id : CachedEventIds)
	{
		const FSoftObjectPath Path = AM.GetPrimaryAssetPath(Id);
		if (!Path.IsValid())
		{
			continue;
		}

		UObject* Obj = SM.LoadSynchronous(Path, /*bManageActiveHandle*/ false);
		UStoneEventData* Event = Cast<UStoneEventData>(Obj);
		if (!Event)
		{
			continue;
		}

		if (Event->EventTags.HasAll(RequiredTags))
		{
			Matching.Add(Event);
		}
	}

	if (Matching.Num() == 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[StoneActionRuntime] No UStoneEventData found matching RequiredTags='%s'."),
			*RequiredTags.ToStringSimple());
		return nullptr;
	}

	const int32 PickIndex = RNG.RandRange(0, Matching.Num() - 1);
	return Matching[PickIndex];
}

void UStoneActionRuntime::EnsureCoreSystems()
{
	if (!Resolver)
	{
		Resolver = NewObject<UStoneEventResolver>(this);
	}
	if (!OutcomeExecutor)
	{
		OutcomeExecutor = NewObject<UStoneOutcomeExecutor>(this);
	}
	if (!Scheduler)
	{
		Scheduler = NewObject<UStoneScheduler>(this);
	}
}

void UStoneActionRuntime::GetResolvedChoices(const UStoneEventData* Event, TArray<FStoneChoiceResolved>& OutResolved) const
{
	OutResolved.Reset();

	// Ensure systems exist (const -> EnsureCoreSystems müsste mutable/const-safe sein oder du machst non-const)
	const_cast<UStoneActionRuntime*>(this)->EnsureCoreSystems();

	if (!Event || !Resolver)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneActionRuntime] GetResolvedChoices: missing Event/Resolver."));
		return;
	}

	UAbilitySystemComponent* InASC = ASC.Get();
	if (!InASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneActionRuntime] GetResolvedChoices: ASC invalid."));
		return;
	}

	Resolver->ResolveChoices(Event, InASC, RuntimeTags, OutResolved);
}

void UStoneActionRuntime::ExecuteChoiceOutcomes(const FStoneChoiceData& Choice, bool bSoftFailPath)
{
	if (!OutcomeExecutor)
	{
		return;
	}

	UAbilitySystemComponent* InASC = ASC.Get();
	if (!InASC)
	{
		return;
	}

	FStoneOutcomeContext Ctx;
	Ctx.ASC = InASC;
	

	// IMPORTANT: tags are action-scoped now
	Ctx.Tags = &RuntimeTags;

	// Optional: if your executor uses these, keep them. Otherwise remove.
	Ctx.Scheduler = Scheduler;
	Ctx.Time = &Time;
	Ctx.FocusTag = &FocusTag;

	const TArray<FStoneOutcome>& Outcomes = bSoftFailPath ? Choice.FailOutcomes : Choice.Outcomes;
	OutcomeExecutor->ApplyOutcomes(Outcomes, Ctx);

	// Optional schedules directly on choice (if your data supports it)
	if (Scheduler)
	{
		for (const FStoneScheduledEvent& Sch : Choice.Schedules)
		{
			Scheduler->Enqueue(Sch, Time);
		}
	}
}

bool UStoneActionRuntime::ApplyChoice(const UStoneEventData* Event, int32 ChoiceIndex)
{
	EnsureCoreSystems();

	if (!Event || !Resolver)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneActionRuntime] ApplyChoice failed: Event or Resolver null."));
		return false;
	}

	UAbilitySystemComponent* InASC = ASC.Get();
	if (!InASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneActionRuntime] ApplyChoice failed: ASC null."));
		return false;
	}

	if (!Event->Choices.IsValidIndex(ChoiceIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneActionRuntime] ApplyChoice failed: invalid ChoiceIndex=%d for Event=%s"),
			ChoiceIndex, *GetNameSafe(Event));
		return false;
	}

	const FStoneChoiceData& Choice = Event->Choices[ChoiceIndex];

	// Evaluate requirement: SoftFail remains callable
	const bool bReqOk = Resolver->EvaluateRequirement(Choice.Requirement, InASC, RuntimeTags);
	const bool bSoftFail = (!bReqOk && Choice.LockMode == EStoneChoiceLockMode::SoftFail);

	if (!bReqOk && Choice.LockMode != EStoneChoiceLockMode::SoftFail)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[StoneActionRuntime] ApplyChoice blocked: requirement failed and not SoftFail. Event=%s Choice=%d"),
			*GetNameSafe(Event), ChoiceIndex);
		return false;
	}

	ExecuteChoiceOutcomes(Choice, bSoftFail);

	IncrementChoiceCounter();
	UE_LOG(LogTemp, Warning, TEXT("[StoneActionRuntime] ApplyChoice TargetASC Owner=%s ASC=%s Event=%s Choice=%d"),
		*GetNameSafe(InASC ? InASC->GetOwner() : nullptr),
		*GetNameSafe(InASC),
		*GetNameSafe(Event),
		ChoiceIndex);	return true;
}

void UStoneActionRuntime::IncrementChoiceCounter()
{
	Time.TotalChoices += 1;

	if (Scheduler)
	{
		// If your scheduler expects NotifyChoiceAdvanced signature, keep it.
		// Otherwise remove.
		Scheduler->NotifyChoiceAdvanced(Time, 1, /*bIsNewDay*/ false, Time.bIsNight);
	}
}