#include "Library/StoneEventLibrary.h"

#include "Data/StoneEventData.h"

static FPrimaryAssetType StoneEventType()
{
	return FPrimaryAssetType(TEXT("StoneEvent"));
}

void UStoneEventLibrary::Initialize()
{
	if (bInitialized) return;
	bInitialized = true;

	BuildPathCache();
}

void UStoneEventLibrary::BuildPathCache()
{
	EventIdToPrimaryId.Reset();

	UAssetManager& AM = UAssetManager::Get();

	TArray<FPrimaryAssetId> Ids;
	AM.GetPrimaryAssetIdList(StoneEventType(), Ids);

	for (const FPrimaryAssetId& Id : Ids)
	{
		// We enforce Studio rule: EventId == AssetName
		const FName EventId = Id.PrimaryAssetName;
		EventIdToPrimaryId.Add(EventId, Id);
	}
}

bool UStoneEventLibrary::HasEvent(FName EventId) const
{
	return EventId != NAME_None && EventIdToPrimaryId.Contains(EventId);
}

void UStoneEventLibrary::GetAllKnownEventIds(TArray<FName>& OutEventIds) const
{
	OutEventIds.Reset();
	OutEventIds.Reserve(EventIdToPrimaryId.Num());
	for (const auto& It : EventIdToPrimaryId)
	{
		OutEventIds.Add(It.Key);
	}
}

UStoneEventData* UStoneEventLibrary::GetEvent(FName EventId) const
{
	if (const TObjectPtr<UStoneEventData>* Found = LoadedEvents.Find(EventId))
	{
		return Found->Get();
	}
	return nullptr;
}

bool UStoneEventLibrary::PreloadPrimaryAssetIds(const TArray<FPrimaryAssetId>& Ids, bool bSynchronous)
{
	if (Ids.Num() == 0)
	{
		OnEventsPreloaded.Broadcast(true);
		return true;
	}

	UAssetManager& AM = UAssetManager::Get();

	// LoadPrimaryAssets() gives a streamable handle; we can Wait if synchronous.
	ActiveHandle = AM.LoadPrimaryAssets(Ids);

	if (!ActiveHandle.IsValid())
	{
		OnEventsPreloaded.Broadcast(false);
		return false;
	}

	if (bSynchronous)
	{
		ActiveHandle->WaitUntilComplete();
	}

	// Populate LoadedEvents for all assets that are now loaded (sync) or partially loaded (async)
	bool bAllOk = true;

	for (const FPrimaryAssetId& Id : Ids)
	{
		const FName EventId = Id.PrimaryAssetName;

		const FSoftObjectPath Path = AM.GetPrimaryAssetPath(Id);
		if (!Path.IsValid())
		{
			bAllOk = false;
			continue;
		}

		UObject* Obj = Path.ResolveObject();
		if (!Obj && bSynchronous)
		{
			// In sync path, it should be loaded. If not, attempt a strict load.
			Obj = Path.TryLoad();
		}

		UStoneEventData* Ev = Cast<UStoneEventData>(Obj);
		if (!Ev)
		{
			bAllOk = false;
			continue;
		}

		LoadedEvents.Add(EventId, Ev);
	}

	// If async, broadcast completion once it really completes
	if (!bSynchronous)
	{
		ActiveHandle->BindCompleteDelegate(FStreamableDelegate::CreateLambda([this]()
		{
			// Build a final sweep for any missing objects by resolving now
			UAssetManager& AM2 = UAssetManager::Get();
			for (const auto& Pair : EventIdToPrimaryId)
			{
				const FName EventId = Pair.Key;
				if (LoadedEvents.Contains(EventId)) continue;

				const FSoftObjectPath Path = AM2.GetPrimaryAssetPath(Pair.Value);
				UObject* Obj = Path.ResolveObject();
				if (UStoneEventData* Ev = Cast<UStoneEventData>(Obj))
				{
					LoadedEvents.Add(EventId, Ev);
				}
			}

			OnEventsPreloaded.Broadcast(true);
		}));
	}
	else
	{
		OnEventsPreloaded.Broadcast(bAllOk);
	}

	return bAllOk;
}

bool UStoneEventLibrary::PreloadByIds(const TArray<FName>& EventIds, bool bSynchronous)
{
	Initialize();

	TArray<FPrimaryAssetId> Ids;
	Ids.Reserve(EventIds.Num());

	for (const FName& EId : EventIds)
	{
		if (const FPrimaryAssetId* Found = EventIdToPrimaryId.Find(EId))
		{
			Ids.Add(*Found);
		}
	}

	return PreloadPrimaryAssetIds(Ids, bSynchronous);
}

bool UStoneEventLibrary::PreloadAll(bool bSynchronous)
{
	Initialize();

	TArray<FPrimaryAssetId> Ids;
	Ids.Reserve(EventIdToPrimaryId.Num());

	for (const auto& Pair : EventIdToPrimaryId)
	{
		Ids.Add(Pair.Value);
	}

	return PreloadPrimaryAssetIds(Ids, bSynchronous);
}
