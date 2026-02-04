#include "Library/StonePackLibrary.h"

#include "Data/StoneEventPackData.h"

static FPrimaryAssetType StonePackType()
{
	return FPrimaryAssetType(TEXT("StonePack"));
}

void UStonePackLibrary::Initialize()
{
	if (bInitialized) return;
	bInitialized = true;
	BuildPathCache();
}

void UStonePackLibrary::BuildPathCache()
{
	PackIdToPrimaryId.Reset();

	UAssetManager& AM = UAssetManager::Get();

	TArray<FPrimaryAssetId> Ids;
	AM.GetPrimaryAssetIdList(StonePackType(), Ids);

	UE_LOG(LogTemp, Warning, TEXT("[StonePackLibrary] Found %d PrimaryAssets for type '%s'"),
		Ids.Num(), *StonePackType().ToString());

	if (Ids.Num() == 0)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[StonePackLibrary] ZERO packs found. Fix AssetManager settings:\n")
			TEXT("Project Settings -> Asset Manager -> Primary Asset Types to Scan -> StonePack\n")
			TEXT("Directories must include your real folder: /Game/Blueprints/Game/Stone/Packs"));
	}

	for (const FPrimaryAssetId& Id : Ids)
	{
		const FName PackId = Id.PrimaryAssetName;
		PackIdToPrimaryId.Add(PackId, Id);

		const FSoftObjectPath Path = AM.GetPrimaryAssetPath(Id);
		UE_LOG(LogTemp, Warning, TEXT("[StonePackLibrary] Id=%s  Type=%s  Name=%s"),
			*Id.ToString(),
			*Id.PrimaryAssetType.ToString(),
			*Id.PrimaryAssetName.ToString());
	}
}

bool UStonePackLibrary::HasPack(FName PackId) const
{
	return PackId != NAME_None && PackIdToPrimaryId.Contains(PackId);
}

UStoneEventPackData* UStonePackLibrary::GetPack(FName PackId) const
{
	if (const TObjectPtr<UStoneEventPackData>* Found = LoadedPacks.Find(PackId))
	{
		return Found->Get();
	}
	return nullptr;
}

void UStonePackLibrary::GetAllKnownPackIds(TArray<FName>& Out) const
{
	Out.Reset();
	for (const auto& It : PackIdToPrimaryId)
	{
		Out.Add(It.Key);
	}
}

bool UStonePackLibrary::PreloadPrimaryAssetIds(const TArray<FPrimaryAssetId>& Ids, bool bSynchronous)
{
	if (Ids.Num() == 0) return true;

	UAssetManager& AM = UAssetManager::Get();
	ActiveHandle = AM.LoadPrimaryAssets(Ids);
	if (!ActiveHandle.IsValid()) return false;

	if (bSynchronous)
	{
		ActiveHandle->WaitUntilComplete();
	}

	bool bOk = true;

	for (const FPrimaryAssetId& Id : Ids)
	{
		const FName PackId = Id.PrimaryAssetName;
		const FSoftObjectPath Path = AM.GetPrimaryAssetPath(Id);
		UObject* Obj = Path.ResolveObject();
		if (!Obj && bSynchronous)
		{
			Obj = Path.TryLoad();
		}

		UStoneEventPackData* Pack = Cast<UStoneEventPackData>(Obj);
		if (!Pack) { bOk = false; continue; }

		// Studio rule: PackId == AssetName
		LoadedPacks.Add(PackId, Pack);
	}

	return bOk;
}

bool UStonePackLibrary::PreloadByIds(const TArray<FName>& PackIds, bool bSynchronous)
{
	Initialize();

	TArray<FPrimaryAssetId> Ids;
	for (const FName& Id : PackIds)
	{
		if (const FPrimaryAssetId* Found = PackIdToPrimaryId.Find(Id))
		{
			Ids.Add(*Found);
		}
	}
	return PreloadPrimaryAssetIds(Ids, bSynchronous);
}

bool UStonePackLibrary::PreloadAll(bool bSynchronous)
{
	Initialize();

	TArray<FPrimaryAssetId> Ids;
	for (const auto& It : PackIdToPrimaryId)
	{
		Ids.Add(It.Value);
	}
	return PreloadPrimaryAssetIds(Ids, bSynchronous);
}
