#include "Runtime/StoneRunSubsystem.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "Core/StoneGameplayTags.h"
#include "Core/StonePlayerState.h"
#include "Data/StoneEventPackData.h"
#include "Kismet/GameplayStatics.h"
#include "Library/StoneEventLibrary.h"
#include "Library/StonePackLibrary.h"
#include "Game/Events/StoneEventResolver.h"

// =============================================================================
// Simulation speed
// =============================================================================

float UStoneRunSubsystem::GetSimulationSpeed() const
{
	// Global scalar that action systems may choose to use.
	return FMath::Clamp(UserSimSpeed * WorldTimeSpeedMult, 0.f, 10.f);
}

void UStoneRunSubsystem::SetSimulationSpeed(float NewSpeed)
{
	UserSimSpeed = FMath::Clamp(NewSpeed, 0.f, 10.f);
}

void UStoneRunSubsystem::SetWorldTimeSpeedMultiplier(float NewWorldTimeSpeed)
{
	WorldTimeSpeedMult = FMath::Clamp(NewWorldTimeSpeed, 0.f, 10.f);
}

// =============================================================================
// Lifecycle
// =============================================================================

void UStoneRunSubsystem::StartNewRun(const FStoneRunConfig& Config)
{
	UE_LOG(LogTemp, Log, TEXT("[StoneRunSubsystem] StartNewRun"));

	if (!EnsurePlayerStateCache())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRunSubsystem] StartNewRun failed: PlayerState cache missing."));
		return;
	}

	Time = FStoneTimeState{};
	RunTags = Config.StartingTags;
	bAutoPackUnlocksEnabled = Config.bEnableAutoPackUnlocks;

	// Trace is purely for diagnostics / UI log.
	Trace = NewObject<UStoneRunTraceBuffer>(this);
	Trace->Init(800);
	Trace->Clear();

	// Day/Night tags start as day by default.
	ApplyDayNightTags(false);

	// Build the event pool from packs. (No event ticking / no global encounter spawning.)
	BuildInitialEventPool(Config);

	RebuildSnapshot();
	BroadcastSnapshot();
}

void UStoneRunSubsystem::Deinitialize()
{
	TemporaryPackIds.Reset();
	CachedPlayerState.Reset();

	Super::Deinitialize();
}

// =============================================================================
// Time (UDS integration)
// =============================================================================

void UStoneRunSubsystem::OnSunrise()
{
	Time.DayIndex += 1;
	Time.bIsNight = false;
	ApplyDayNightTags(false);

	UE_LOG(LogTemp, Log, TEXT("[StoneRunSubsystem] OnSunrise: Day %d"), Time.DayIndex);

	RebuildSnapshot();
	BroadcastSnapshot();
}

void UStoneRunSubsystem::OnSunset()
{
	Time.TotalNightsPassed += 1;
	Time.bIsNight = true;
	ApplyDayNightTags(true);

	UE_LOG(LogTemp, Log, TEXT("[StoneRunSubsystem] OnSunset: Night %d"), Time.TotalNightsPassed);

	RebuildSnapshot();
	BroadcastSnapshot();
}

void UStoneRunSubsystem::OnHourChanged(int32 NewHour)
{
	Time.CurrentHour = NewHour;

	// No random event rolling here. Encounters are action-scoped.
	RebuildSnapshot();
	BroadcastSnapshot();
}

void UStoneRunSubsystem::ApplyDayNightTags(bool bNowNight)
{
	const FStoneGameplayTags& Tags = FStoneGameplayTags::Get();

	RunTags.RemoveTag(Tags.State_Day);
	RunTags.RemoveTag(Tags.State_Night);

	RunTags.AddTag(bNowNight ? Tags.State_Night : Tags.State_Day);
}

// =============================================================================
// Run tags
// =============================================================================

void UStoneRunSubsystem::AddRunTags(const FGameplayTagContainer& TagsToAdd)
{
	if (TagsToAdd.IsEmpty())
	{
		return;
	}

	RunTags.AppendTags(TagsToAdd);
	RebuildSnapshot();
	BroadcastSnapshot();
}

void UStoneRunSubsystem::RemoveRunTags(const FGameplayTagContainer& TagsToRemove)
{
	if (TagsToRemove.IsEmpty())
	{
		return;
	}

	RunTags.RemoveTags(TagsToRemove);
	RebuildSnapshot();
	BroadcastSnapshot();
}

// =============================================================================
// GAS access (PlayerState owned ASC)
// =============================================================================

bool UStoneRunSubsystem::EnsurePlayerStateCache()
{
	if (CachedPlayerState.IsValid())
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneRunSubsystem] EnsurePlayerStateCache: No PlayerController."));
		return false;
	}

	AStonePlayerState* PS = PC->GetPlayerState<AStonePlayerState>();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneRunSubsystem] EnsurePlayerStateCache: PlayerState is not AStonePlayerState."));
		return false;
	}

	CachedPlayerState = PS;
	return true;
}

AStonePlayerState* UStoneRunSubsystem::GetPlayerState() const
{
	return CachedPlayerState.Get();
}

UAbilitySystemComponent* UStoneRunSubsystem::GetASC() const
{
	if (CachedPlayerState.IsValid())
	{
		return CachedPlayerState->GetAbilitySystemComponent();
	}
	return nullptr;
}

// =============================================================================
// Snapshot (UI convenience)
// =============================================================================

void UStoneRunSubsystem::RebuildSnapshot()
{
	Snapshot.Time = Time;
	Snapshot.RunTags = RunTags;

	if (UAbilitySystemComponent* ASC = GetASC())
	{
		Snapshot.Food   = ASC->GetNumericAttribute(UStoneAttributeSet::GetFoodAttribute());
		Snapshot.Water  = ASC->GetNumericAttribute(UStoneAttributeSet::GetWaterAttribute());
		Snapshot.Health = ASC->GetNumericAttribute(UStoneAttributeSet::GetHealthAttribute());
		Snapshot.Morale = ASC->GetNumericAttribute(UStoneAttributeSet::GetMoraleAttribute());
		Snapshot.Warmth = ASC->GetNumericAttribute(UStoneAttributeSet::GetWarmthAttribute());
		Snapshot.Trust  = ASC->GetNumericAttribute(UStoneAttributeSet::GetTrustAttribute());
	}
}

void UStoneRunSubsystem::BroadcastSnapshot()
{
	OnSnapshotChanged.Broadcast(Snapshot);
}

// =============================================================================
// Libraries / packs
// =============================================================================

void UStoneRunSubsystem::EnsureEventLibrary(bool bPreloadAllSync)
{
	if (EventLibrary)
	{
		return;
	}

	EventLibrary = NewObject<UStoneEventLibrary>(this);
	EventLibrary->Initialize();

	// Optional preload (UI-driven game -> can be desired; content size dependent)
	EventLibrary->PreloadAll(bPreloadAllSync);
}

void UStoneRunSubsystem::EnsurePackLibrary(bool bPreloadAllSync)
{
	if (PackLibrary)
	{
		return;
	}

	PackLibrary = NewObject<UStonePackLibrary>(this);
	PackLibrary->Initialize();
	PackLibrary->PreloadAll(bPreloadAllSync);

	KnownPackIds.Reset();
	PackLibrary->GetAllKnownPackIds(KnownPackIds);
}

void UStoneRunSubsystem::AddEventsFromPackId(FName PackId, bool bPreloadIfPackRequests)
{
	if (PackId.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRunSubsystem] AddEventsFromPackId: PackId is None"));
		return;
	}

	EnsurePackLibrary(true);
	EnsureEventLibrary(true);

	if (!PackLibrary)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRunSubsystem] AddEventsFromPackId: PackLibrary is null"));
		return;
	}

	UStoneEventPackData* Pack = PackLibrary->GetPack(PackId);
	if (!Pack)
	{
		TArray<FName> One;
		One.Add(PackId);
		PackLibrary->PreloadByIds(One, true);
		Pack = PackLibrary->GetPack(PackId);
	}

	if (!Pack)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[StoneRunSubsystem] AddEventsFromPackId: Pack '%s' NOT FOUND. Check AssetManager scan for 'StonePack'."),
			*PackId.ToString());
		return;
	}

	ActivePackIds.AddUnique(PackId);

	// We intentionally do NOT build or manage a global EventPool here anymore.
	// Packs are still tracked so action systems can query active packs if needed.

	if (bPreloadIfPackRequests && Pack->bPreloadOnUnlock && EventLibrary)
	{
		TArray<FName> PreloadEvents;
		for (const FStonePackEntry& Entry : Pack->Events)
		{
			if (!Entry.EventId.IsNone())
			{
				PreloadEvents.AddUnique(Entry.EventId);
			}
		}
		if (PreloadEvents.Num() > 0)
		{
			EventLibrary->PreloadByIds(PreloadEvents, true);
		}
	}
}

void UStoneRunSubsystem::BuildInitialEventPool(const FStoneRunConfig& Config)
{
	EnsurePackLibrary(true);
	EnsureEventLibrary(true);

	ActivePackIds.Reset();
	TemporaryPackIds.Reset();

	if (Config.StartingPackIds.Num() == 0)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[StoneRunSubsystem] BuildInitialEventPool: Config.StartingPackIds is empty. Caller must provide starting packs."));
		return;
	}

	for (const FName& PackId : Config.StartingPackIds)
	{
		AddEventsFromPackId(PackId, /*bPreloadIfPackRequests*/ true);
	}

	TryAutoUnlockPacks();

	UE_LOG(LogTemp, Log, TEXT("[StoneRunSubsystem] Start packs: ActivePacks=%d"), ActivePackIds.Num());
}

void UStoneRunSubsystem::TryAutoUnlockPacks()
{
	if (!bAutoPackUnlocksEnabled)
	{
		return;
	}

	EnsurePackLibrary(true);
	EnsureEventLibrary(true);

	if (!PackLibrary)
	{
		return;
	}

	for (const FName& PackId : KnownPackIds)
	{
		if (PackId.IsNone() || ActivePackIds.Contains(PackId))
		{
			continue;
		}

		UStoneEventPackData* Pack = PackLibrary->GetPack(PackId);
		if (!Pack || !Pack->bAutoUnlockWhenRequirementsMet)
		{
			continue;
		}

		if (!RunTags.HasAll(Pack->RequiredTagsAll))
		{
			continue;
		}
		if (!Pack->BlockedTagsAny.IsEmpty() && RunTags.HasAny(Pack->BlockedTagsAny))
		{
			continue;
		}

		ActivePackIds.AddUnique(PackId);

		UE_LOG(LogTemp, Log, TEXT("[StoneRunSubsystem] Auto-unlocked pack %s"), *PackId.ToString());

		if (Pack->bPreloadOnUnlock && EventLibrary)
		{
			TArray<FName> PreloadEvents;
			for (const FStonePackEntry& Entry : Pack->Events)
			{
				if (!Entry.EventId.IsNone())
				{
					PreloadEvents.AddUnique(Entry.EventId);
				}
			}
			if (PreloadEvents.Num() > 0)
			{
				EventLibrary->PreloadByIds(PreloadEvents, true);
			}
		}
	}
}

void UStoneRunSubsystem::ActivatePackTemporary(FName PackId)
{
	if (PackId.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneRunSubsystem] ActivatePackTemporary: PackId is None"));
		return;
	}

	const bool bWasActive = ActivePackIds.Contains(PackId);
	AddEventsFromPackId(PackId, /*bPreloadIfPackRequests*/ true);

	if (!bWasActive)
	{
		TemporaryPackIds.AddUnique(PackId);
	}
}

void UStoneRunSubsystem::DeactivatePackInternal(FName PackId)
{
	if (PackId.IsNone())
	{
		return;
	}
	ActivePackIds.Remove(PackId);
}

void UStoneRunSubsystem::RebuildEventPoolFromActivePacks(bool /*bPreloadIfPackRequests*/)
{
	// Intentionally empty: the global event pool system was removed.
}

void UStoneRunSubsystem::DeactivateTemporaryPacks()
{
	if (TemporaryPackIds.Num() == 0)
	{
		return;
	}

	for (const FName& PackId : TemporaryPackIds)
	{
		DeactivatePackInternal(PackId);
	}

	TemporaryPackIds.Reset();
	RebuildEventPoolFromActivePacks(false);
}

void UStoneRunSubsystem::DeactivateTemporaryPacksByIds(const TArray<FName>& PackIds)
{
	if (PackIds.Num() == 0 || TemporaryPackIds.Num() == 0)
	{
		return;
	}

	bool bChanged = false;
	for (const FName& PackId : PackIds)
	{
		if (PackId.IsNone())
		{
			continue;
		}

		const int32 Index = TemporaryPackIds.Find(PackId);
		if (Index != INDEX_NONE)
		{
			TemporaryPackIds.RemoveAt(Index);
			DeactivatePackInternal(PackId);
			bChanged = true;
		}
	}

	if (bChanged)
	{
		RebuildEventPoolFromActivePacks(false);
	}
}
