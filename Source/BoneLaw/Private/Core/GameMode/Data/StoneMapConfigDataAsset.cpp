#include "Core/GameMode/Data/StoneMapConfigDataAsset.h"

bool UStoneMapConfigDataAsset::TryGetMapByKey(FName Key, TSoftObjectPtr<UWorld>& OutMap) const
{
	OutMap = nullptr;

	if (Key.IsNone())
	{
		return false;
	}

	const TSoftObjectPtr<UWorld>* Found = Maps.Find(Key);
	if (!Found)
	{
		return false;
	}

	OutMap = *Found;
	return !OutMap.IsNull();
}

bool UStoneMapConfigDataAsset::TryGetDefaultMap(TSoftObjectPtr<UWorld>& OutMap) const
{
	OutMap = nullptr;

	// 1) Prefer DefaultMapKey
	if (!DefaultMapKey.IsNone())
	{
		if (TryGetMapByKey(DefaultMapKey, OutMap))
		{
			return true;
		}
	}

	// 2) Fallback: DefaultMap soft pointer
	if (!DefaultMap.IsNull())
	{
		OutMap = DefaultMap;
		return true;
	}

	return false;
}

FName UStoneMapConfigDataAsset::FindKeyByMapAssetName(const FString& MapAssetName) const
{
	if (MapAssetName.IsEmpty())
	{
		return NAME_None;
	}

	for (const TPair<FName, TSoftObjectPtr<UWorld>>& Pair : Maps)
	{
		const TSoftObjectPtr<UWorld>& MapPtr = Pair.Value;
		if (MapPtr.IsNull())
		{
			continue;
		}

		const FString AssetName = MapPtr.ToSoftObjectPath().GetAssetName();
		if (AssetName.Equals(MapAssetName, ESearchCase::CaseSensitive))
		{
			return Pair.Key;
		}
	}

	// Also allow DefaultMap when not listed in Maps
	if (!DefaultMap.IsNull())
	{
		const FString DefaultAssetName = DefaultMap.ToSoftObjectPath().GetAssetName();
		if (DefaultAssetName.Equals(MapAssetName, ESearchCase::CaseSensitive))
		{
			return DefaultMapKey;
		}
	}

	return NAME_None;
}

bool UStoneMapConfigDataAsset::ValidateConfig() const
{
	bool bOk = true;

	if (Maps.Num() == 0 && DefaultMap.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneMapConfigDA] No Maps and DefaultMap is null. Config invalid."));
		bOk = false;
	}

	if (!DefaultMapKey.IsNone())
	{
		const TSoftObjectPtr<UWorld>* Found = Maps.Find(DefaultMapKey);
		if (!Found || Found->IsNull())
		{
			// Not fatal if DefaultMap is set, but it's a config smell.
			if (DefaultMap.IsNull())
			{
				UE_LOG(LogTemp, Error, TEXT("[StoneMapConfigDA] DefaultMapKey '%s' not found (or null) AND DefaultMap is null."), *DefaultMapKey.ToString());
				bOk = false;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[StoneMapConfigDA] DefaultMapKey '%s' not found (or null). Falling back to DefaultMap."), *DefaultMapKey.ToString());
			}
		}
	}

	for (const TPair<FName, TSoftObjectPtr<UWorld>>& Pair : Maps)
	{
		if (Pair.Key.IsNone())
		{
			UE_LOG(LogTemp, Error, TEXT("[StoneMapConfigDA] Maps contains an entry with None key."));
			bOk = false;
			continue;
		}

		if (Pair.Value.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("[StoneMapConfigDA] Maps entry '%s' has null map reference."), *Pair.Key.ToString());
			bOk = false;
		}
	}

	return bOk;
}
