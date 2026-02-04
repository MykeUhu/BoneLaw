#include "Data/StoneContentValidator.h"

#include "Core/StoneContentSettings.h"
#include "Data/StoneEventData.h"
#include "Data/StoneEventPackData.h"
#include "Data/StoneValidationTypes.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "Modules/ModuleManager.h"

static void AddIssue(TArray<FStoneValidationIssue>& Out, EStoneValidationSeverity Sev, const FString& AssetPath, const FString& Msg)
{
	FStoneValidationIssue I;
	I.Severity = Sev;
	I.AssetPath = AssetPath;
	I.Message = Msg;
	Out.Add(MoveTemp(I));
}

static bool CollectAssetsByClassPath(const FString& RootPath, const FTopLevelAssetPath& ClassPath, TArray<FAssetData>& OutAssets)
{
	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(*RootPath);
	Filter.ClassPaths.Add(ClassPath);

	ARM.Get().GetAssets(Filter, OutAssets);
	return OutAssets.Num() > 0;
}

static void CollectAllStoneEventIds_AssetManager(TSet<FName>& OutEventIds)
{
	OutEventIds.Reset();

	UAssetManager& AM = UAssetManager::Get();
	TArray<FPrimaryAssetId> Ids;
	AM.GetPrimaryAssetIdList(FPrimaryAssetType(TEXT("StoneEvent")), Ids);

	for (const FPrimaryAssetId& Id : Ids)
	{
		// Studio rule: EventId == AssetName
		OutEventIds.Add(Id.PrimaryAssetName);
	}
}

static void CollectAllStonePackIds_AssetManager(TSet<FName>& OutPackIds)
{
	OutPackIds.Reset();

	UAssetManager& AM = UAssetManager::Get();
	TArray<FPrimaryAssetId> Ids;
	AM.GetPrimaryAssetIdList(FPrimaryAssetType(TEXT("StonePack")), Ids);

	for (const FPrimaryAssetId& Id : Ids)
	{
		OutPackIds.Add(Id.PrimaryAssetName);
	}
}

static UObject* LoadPrimaryAssetObjectSync(const FPrimaryAssetId& Id)
{
	UAssetManager& AM = UAssetManager::Get();
	const FSoftObjectPath Path = AM.GetPrimaryAssetPath(Id);
	return Path.IsValid() ? Path.TryLoad() : nullptr;
}

TArray<FStoneValidationIssue> UStoneContentValidator::ValidateAllStoneEvents(const FString& RootPath)
{
	TArray<FStoneValidationIssue> Issues;

	// SSOT path: validate exactly what AssetManager scans as StoneEvent
	if (RootPath.IsEmpty())
	{
		UAssetManager& AM = UAssetManager::Get();
		TArray<FPrimaryAssetId> EventIds;
		AM.GetPrimaryAssetIdList(FPrimaryAssetType(TEXT("StoneEvent")), EventIds);

		if (EventIds.Num() == 0)
		{
			AddIssue(Issues, EStoneValidationSeverity::Warning, TEXT("<AssetManager>"),
				TEXT("No StoneEvent PrimaryAssets found. Check DefaultGame.ini [/Script/Engine.AssetManagerSettings] PrimaryAssetTypesToScan for StoneEvent."));
			return Issues;
		}

		// Known ids for reference checks
		TSet<FName> KnownEventIds;
		for (const FPrimaryAssetId& Id : EventIds) { KnownEventIds.Add(Id.PrimaryAssetName); }

		TSet<FName> SeenAssetNames;

		for (const FPrimaryAssetId& Id : EventIds)
		{
			UObject* Obj = LoadPrimaryAssetObjectSync(Id);
			UStoneEventData* Ev = Cast<UStoneEventData>(Obj);

			const FString AssetPath = Obj ? Obj->GetPathName() : FString::Printf(TEXT("<%s>"), *Id.ToString());

			if (!Ev)
			{
				AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
					TEXT("PrimaryAsset is not UStoneEventData (or failed to load)."));
				continue;
			}

			const FName AssetName = Id.PrimaryAssetName;

			if (Ev->EventId.IsNone())
			{
				AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
					TEXT("EventId is None. Must be set and must match AssetName."));
			}
			else if (Ev->EventId != AssetName)
			{
				AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
					FString::Printf(TEXT("EventId '%s' does not match AssetName '%s' (studio rule)."),
						*Ev->EventId.ToString(), *AssetName.ToString()));
			}

			if (SeenAssetNames.Contains(AssetName))
			{
				AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
					FString::Printf(TEXT("Duplicate StoneEvent AssetName '%s'."), *AssetName.ToString()));
			}
			SeenAssetNames.Add(AssetName);

			if (Ev->BaseWeight <= 0)
			{
				AddIssue(Issues, EStoneValidationSeverity::Warning, AssetPath,
					TEXT("BaseWeight <= 0. Event will rarely/never be picked unless forced."));
			}

			if (Ev->Choices.Num() < 1)
			{
				AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
					TEXT("Event has 0 choices. Must have at least 1 choice."));
			}

			// Schedule reference checks (only existence by id)
			for (int32 i = 0; i < Ev->Choices.Num(); ++i)
			{
				const FStoneChoiceData& C = Ev->Choices[i];
				for (int32 s = 0; s < C.Schedules.Num(); ++s)
				{
					const FStoneScheduledEvent& Sch = C.Schedules[s];
					if (!Sch.EventId.IsNone() && !KnownEventIds.Contains(Sch.EventId))
					{
						AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
							FString::Printf(TEXT("Choice[%d] schedules missing EventId '%s'."), i, *Sch.EventId.ToString()));
					}
				}
			}
		}

		return Issues;
	}

	// Debug override: validate by path using AssetRegistry
	TArray<FAssetData> EventAssets;
	if (!CollectAssetsByClassPath(RootPath, UStoneEventData::StaticClass()->GetClassPathName(), EventAssets))
	{
		AddIssue(Issues, EStoneValidationSeverity::Warning, RootPath,
			TEXT("No StoneEvent assets found under RootPath. Check path or use empty RootPath to validate via AssetManager scan."));
		return Issues;
	}

	TSet<FName> SeenIds;
	TSet<FName> KnownEventIds;
	for (const FAssetData& AD : EventAssets) { KnownEventIds.Add(AD.AssetName); }

	for (const FAssetData& AD : EventAssets)
	{
		UStoneEventData* Ev = Cast<UStoneEventData>(AD.GetAsset());
		if (!Ev)
		{
			AddIssue(Issues, EStoneValidationSeverity::Error, AD.GetObjectPathString(),
				TEXT("Asset is not UStoneEventData."));
			continue;
		}

		const FString AssetPath = Ev->GetPathName();
		const FName AssetName = AD.AssetName;

		if (Ev->EventId.IsNone())
		{
			AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
				TEXT("EventId is None. Must be set and must match AssetName."));
		}
		else if (Ev->EventId != AssetName)
		{
			AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
				FString::Printf(TEXT("EventId '%s' does not match AssetName '%s' (studio rule)."),
					*Ev->EventId.ToString(), *AssetName.ToString()));
		}

		if (SeenIds.Contains(AssetName))
		{
			AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
				FString::Printf(TEXT("Duplicate StoneEvent AssetName '%s'."), *AssetName.ToString()));
		}
		SeenIds.Add(AssetName);
	}

	return Issues;
}

TArray<FStoneValidationIssue> UStoneContentValidator::ValidateAllStonePacks(const FString& RootPath)
{
	TArray<FStoneValidationIssue> Issues;

	// Known events (SSOT)
	TSet<FName> KnownEventIds;
	CollectAllStoneEventIds_AssetManager(KnownEventIds);

	if (RootPath.IsEmpty())
	{
		UAssetManager& AM = UAssetManager::Get();
		TArray<FPrimaryAssetId> PackIds;
		AM.GetPrimaryAssetIdList(FPrimaryAssetType(TEXT("StonePack")), PackIds);

		if (PackIds.Num() == 0)
		{
			AddIssue(Issues, EStoneValidationSeverity::Warning, TEXT("<AssetManager>"),
				TEXT("No StonePack PrimaryAssets found. Check DefaultGame.ini PrimaryAssetTypesToScan for StonePack."));
			return Issues;
		}

		TSet<FName> SeenPackAssetNames;

		for (const FPrimaryAssetId& Id : PackIds)
		{
			UObject* Obj = LoadPrimaryAssetObjectSync(Id);
			UStoneEventPackData* Pack = Cast<UStoneEventPackData>(Obj);

			const FString AssetPath = Obj ? Obj->GetPathName() : FString::Printf(TEXT("<%s>"), *Id.ToString());

			if (!Pack)
			{
				AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
					TEXT("PrimaryAsset is not UStoneEventPackData (or failed to load)."));
				continue;
			}

			const FName AssetName = Id.PrimaryAssetName;

			if (Pack->PackId.IsNone())
			{
				AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
					TEXT("PackId is None. Must be set and must match AssetName."));
			}
			else if (Pack->PackId != AssetName)
			{
				AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
					FString::Printf(TEXT("PackId '%s' does not match AssetName '%s' (studio rule)."),
						*Pack->PackId.ToString(), *AssetName.ToString()));
			}

			if (SeenPackAssetNames.Contains(AssetName))
			{
				AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
					FString::Printf(TEXT("Duplicate pack asset name '%s'."), *AssetName.ToString()));
			}
			SeenPackAssetNames.Add(AssetName);

			if (Pack->Events.Num() == 0)
			{
				AddIssue(Issues, EStoneValidationSeverity::Warning, AssetPath,
					TEXT("Pack has 0 events. It will never add anything to the pool."));
			}

			for (int32 i = 0; i < Pack->Events.Num(); ++i)
			{
				const FStonePackEntry& Entry = Pack->Events[i];

				if (Entry.EventId.IsNone())
				{
					AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
						FString::Printf(TEXT("Pack entry [%d] has EventId None."), i));
					continue;
				}

				if (!KnownEventIds.Contains(Entry.EventId))
				{
					AddIssue(Issues, EStoneValidationSeverity::Error, AssetPath,
						FString::Printf(TEXT("Pack entry [%d] references missing EventId '%s' (expected StoneEvent PrimaryAsset with that name)."),
							i, *Entry.EventId.ToString()));
				}
			}
		}

		return Issues;
	}

	// Debug override by path
	TArray<FAssetData> PackAssets;
	if (!CollectAssetsByClassPath(RootPath, UStoneEventPackData::StaticClass()->GetClassPathName(), PackAssets))
	{
		AddIssue(Issues, EStoneValidationSeverity::Warning, RootPath,
			TEXT("No StonePack assets found under RootPath. Check path or use empty RootPath to validate via AssetManager scan."));
		return Issues;
	}

	for (const FAssetData& AD : PackAssets)
	{
		UStoneEventPackData* Pack = Cast<UStoneEventPackData>(AD.GetAsset());
		if (!Pack)
		{
			AddIssue(Issues, EStoneValidationSeverity::Error, AD.GetObjectPathString(),
				TEXT("Asset is not UStoneEventPackData."));
			continue;
		}
	}

	return Issues;
}

TArray<FStoneValidationIssue> UStoneContentValidator::ValidateAllStoneContent(const FString& EventsRoot, const FString& PacksRoot)
{
	TArray<FStoneValidationIssue> Out;

	Out.Append(ValidateAllStoneEvents(EventsRoot));
	Out.Append(ValidateAllStonePacks(PacksRoot));

	// Required worldline events are SSOT in UStoneContentSettings (no hardcode here)
	const UStoneContentSettings* S = GetDefault<UStoneContentSettings>();
	const TArray<FName>& Required = S ? S->RequiredWorldlineEventIds : TArray<FName>();

	if (Required.Num() > 0)
	{
		TSet<FName> KnownEventIds;
		CollectAllStoneEventIds_AssetManager(KnownEventIds);

		for (const FName& Id : Required)
		{
			if (!KnownEventIds.Contains(Id))
			{
				AddIssue(Out, EStoneValidationSeverity::Error, TEXT("<Worldline>"),
					FString::Printf(TEXT("Missing required Worldline event asset '%s' (create StoneEvent PrimaryAsset with AssetName/EventId)."), *Id.ToString()));
			}
		}
	}

	return Out;
}
