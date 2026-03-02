// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/SaveGame.h"
#include "Data/StoneTypes.h"
#include "LoadScreenSaveGame.generated.h"

class UGameplayAbility;
UENUM(BlueprintType)
enum ESaveSlotStatus
{
    Vacant,
    EnterName,
    Taken
};

USTRUCT()
struct FSavedActor
{
    GENERATED_BODY()

    UPROPERTY()
    FName ActorName = FName();

    UPROPERTY()
    FTransform Transform = FTransform();

    // Serialized variables from the Actor - only those marked with SaveGame specifier
    UPROPERTY()
    TArray<uint8> Bytes;
};

inline bool operator==(const FSavedActor& Left, const FSavedActor& Right)
{
    return Left.ActorName == Right.ActorName;
}

USTRUCT()
struct FSavedMap
{
    GENERATED_BODY()

    UPROPERTY()
    FString MapAssetName;

    UPROPERTY()
    TArray<FSavedActor> SavedActors;
};


UCLASS()
class BONELAW_API ULoadScreenSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // --- Slot Meta ---
    UPROPERTY()
    FString SlotName = FString();

    UPROPERTY()
    int32 SlotIndex = 0;

    UPROPERTY()
    FString PlayerName = FString("Default Name");
    
    UPROPERTY()
    TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = Vacant;

    UPROPERTY()
    FString MapName = FString("Default Map Name");

    UPROPERTY()
    FString MapAssetName = FString("Default Map Asset Name");

    UPROPERTY()
    FName PlayerStartTag;

     UPROPERTY()
    bool bFirstTimeLoadIn = true;
    
    // --- Player ---
    UPROPERTY() int32 PlayerLevel = 1;
    UPROPERTY() float Strength = 0;
    UPROPERTY() float Intelligence = 0;
    UPROPERTY() float Endurance = 0;
    UPROPERTY() float Willpower = 0;
    UPROPERTY() float Social = 0;
    
    UPROPERTY() float Food = 0;
    UPROPERTY() float Water = 0;
    UPROPERTY() float Health = 0;
    UPROPERTY() float Morale = 0;
    UPROPERTY() float Warmth = 0;
    UPROPERTY() float Trust = 0;
    
    UPROPERTY() float MaxFood = 0;
    UPROPERTY() float MaxWater = 0;
    UPROPERTY() float MaxHealth = 0;
    UPROPERTY() float MaxMorale = 0;
    UPROPERTY() float MaxTrust = 0;
    
    // --- Abilities ---
    UPROPERTY()
    TArray<FSavedAbility> SavedAbilities;

    UPROPERTY() TArray<FSavedMap> SavedMaps;

    FSavedMap GetSavedMapWithMapName(const FString& InMapName);
    bool HasMap(const FString& InMapName);

    /**
     * Migrate legacy save data to the current schema, if needed.
     * Call immediately after loading the save game object, before gameplay consumes it.
     */
    bool MigrateIfNeeded();
    
    // --- “Run / Meta Progress” (dein Stone-Loop) ---
    UPROPERTY()
    int32 RNGSeed = 1337;

    UPROPERTY()
    int32 DayIndex = 1;

    UPROPERTY()
    int32 TotalChoices = 0;

    UPROPERTY()
    bool bIsNight = false;

    // Aktuelle Region/Run-Kontext (später RegionSelect)
    UPROPERTY()
    FName RegionId = NAME_None;

    // RunTags/Focus wie im RunSubsystem (Aura-style: direkt speichern)
    UPROPERTY()
    FGameplayTagContainer RunTags;

    UPROPERTY()
    FGameplayTag FocusTag;
    
    // ========================================================================
    // PHASE 2: SETTLER ROSTER + ASSIGNMENTS
    // ========================================================================
    
    /** Save version for migration support. Increment when format changes. */
    UPROPERTY()
    int32 SaveVersion = 2;
    
    /** Roster of all settlers (Phase 2: multiple settlers with individual state). */
    UPROPERTY()
    TArray<FSavedSettler> SavedSettlers;

    /** Runtime-built/basebuilding objects (schema-ready; may be empty in Phase 2). */
    UPROPERTY()
    TArray<FSavedBuildable> SavedBuildables;
    
    /** Open event context (if an event was interrupted during save). */
    UPROPERTY()
    FSavedOpenEventContext OpenEventContext;
    
    /** Active packs (for resume). */
    UPROPERTY()
    TArray<FName> ActivePackIds;
    
    /** Seen events (for pool management). */
    UPROPERTY()
    TSet<FName> SeenEventIds;
    
    // ========================================================================
    // LEGACY FIELDS (kept temporarily for migration from SaveVersion 1)
    // These fields are deprecated and will be removed in future versions.
    // DO NOT USE in new code. Use SavedSettlers instead.
    // ========================================================================
    // NOTE: Existing fields above (Strength, Food, SavedAbilities, etc.) 
    // are marked as LEGACY in migration code.
};
