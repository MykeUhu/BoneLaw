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

void ULoadScreenSaveGame::MigrateIfNeeded()
{
	// SaveVersion is a schema version for this savegame (NOT the project/game version).
	// Phase 2 introduces SavedSettlers (roster-based persistence). Older saves only have legacy single-player fields.

	const bool bNeedsMigration = (SaveVersion < 2) || (SavedSettlers.Num() == 0);
	if (!bNeedsMigration)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[LoadScreenSaveGame] Migrating legacy save data to SaveVersion 2 (roster schema)."));

	SavedSettlers.Empty();

	FSavedSettler DefaultSettler;
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

	UE_LOG(LogTemp, Log, TEXT("[LoadScreenSaveGame] Migration complete. SavedSettlers=%d, SaveVersion=%d"), SavedSettlers.Num(), SaveVersion);
}
