// Copyright by MykeUhu

#include "Core/LoadScreenSaveGame.h"

#include "Core/StoneGameplayTags.h"

FSavedMap ULoadScreenSaveGame::GetSavedMapWithMapName(const FString& InMapName)
{
	for (const FSavedMap& Map : SavedMaps)
	{
		if (Map.MapAssetName == InMapName)
		{
			return Map;
		}
	}
	return FSavedMap();
}

bool ULoadScreenSaveGame::HasMap(const FString& InMapName)
{
	for (const FSavedMap& Map : SavedMaps)
	{
		if (Map.MapAssetName == InMapName)
		{
			return true;
		}
	}
	return false;
}

bool ULoadScreenSaveGame::MigrateIfNeeded()
{
	// SaveVersion is a schema version for this savegame (NOT the project/game version).
	// Phase 2 introduces SavedSettlers (roster-based persistence). Older saves only have legacy single-player fields.
	//
	// IMPORTANT:
	// - SettlerId MUST be valid and unique. GameMode/Roster will skip invalid IDs and nothing will spawn.
	// - We therefore (a) migrate old schema, and (b) repair invalid/duplicate IDs defensively for backward compat.

	bool bDidMutate = false;

	const bool bNeedsSchemaMigration = (SaveVersion < 2) || (SavedSettlers.Num() == 0);
	if (bNeedsSchemaMigration)
	{
		UE_LOG(LogTemp, Log, TEXT("[LoadScreenSaveGame] Migrating legacy save data to SaveVersion 2 (roster schema)."));

		SavedSettlers.Empty();

		FSavedSettler DefaultSettler;
		DefaultSettler.SettlerId = FGuid::NewGuid();
		DefaultSettler.DisplayName = PlayerName.IsEmpty() ? FString(TEXT("Settler")) : PlayerName;
		DefaultSettler.LastKnownTransform = FTransform::Identity; // legacy has no transform; safe default

		// Map legacy floats -> tagged attributes (SSOT via native gameplay tags)
		const FStoneGameplayTags& Tags = FStoneGameplayTags::Get();
		auto AddAttr = [&DefaultSettler](const FGameplayTag& Tag, float Value)
		{
			if (Tag.IsValid())
			{
				DefaultSettler.Attributes.Add(FSavedAttribute(Tag, Value));
			}
		};

		// Primary
		AddAttr(Tags.Attributes_Primary_Strength, Strength);
		AddAttr(Tags.Attributes_Primary_Intelligence, Intelligence);
		AddAttr(Tags.Attributes_Primary_Endurance, Endurance);
		AddAttr(Tags.Attributes_Primary_Willpower, Willpower);
		AddAttr(Tags.Attributes_Primary_Social, Social);

		// Vitals
		AddAttr(Tags.Attributes_Vital_Food, Food);
		AddAttr(Tags.Attributes_Vital_Water, Water);
		AddAttr(Tags.Attributes_Vital_Health, Health);
		AddAttr(Tags.Attributes_Vital_Morale, Morale);
		AddAttr(Tags.Attributes_Vital_Warmth, Warmth);
		AddAttr(Tags.Attributes_Vital_Trust, Trust);

		// Persisted max (legacy had these explicitly)
		AddAttr(Tags.Attributes_Secondary_MaxFood, MaxFood);
		AddAttr(Tags.Attributes_Secondary_MaxWater, MaxWater);
		AddAttr(Tags.Attributes_Secondary_MaxHealth, MaxHealth);
		AddAttr(Tags.Attributes_Secondary_MaxMorale, MaxMorale);
		AddAttr(Tags.Attributes_Secondary_MaxTrust, MaxTrust);

		// Abilities
		DefaultSettler.GrantedAbilities = SavedAbilities;

		SavedSettlers.Add(DefaultSettler);

		SaveVersion = 2;
		bDidMutate = true;

		UE_LOG(LogTemp, Log, TEXT("[LoadScreenSaveGame] Schema migration complete. SavedSettlers=%d, SaveVersion=%d"),
			SavedSettlers.Num(), SaveVersion);
	}

	// Backward-compat repair: invalid / duplicate SettlerIds can exist (older saves / migration bugs).
	// This must be fixed BEFORE the roster is initialized and pawns are spawned.
	TSet<FGuid> Seen;
	Seen.Reserve(SavedSettlers.Num());

	for (FSavedSettler& S : SavedSettlers)
	{
		const bool bInvalid = !S.SettlerId.IsValid();
		const bool bDuplicate = !bInvalid && Seen.Contains(S.SettlerId);

		if (bInvalid || bDuplicate)
		{
			const FGuid Old = S.SettlerId;
			S.SettlerId = FGuid::NewGuid();
			bDidMutate = true;

			UE_LOG(LogTemp, Warning, TEXT("[LoadScreenSaveGame] Repaired %s SettlerId. Old=%s New=%s Name='%s'"),
				bInvalid ? TEXT("INVALID") : TEXT("DUPLICATE"),
				*Old.ToString(EGuidFormats::DigitsWithHyphensLower),
				*S.SettlerId.ToString(EGuidFormats::DigitsWithHyphensLower),
				S.DisplayName.IsEmpty() ? TEXT("<empty>") : *S.DisplayName);
		}

		Seen.Add(S.SettlerId);
	}

	return bDidMutate;
}

