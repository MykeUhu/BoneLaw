// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/SaveGame.h"
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

USTRUCT(BlueprintType)
struct FSavedAbility
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassDefaults")
    TSubclassOf<UGameplayAbility> GameplayAbility;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FGameplayTag AbilityTag = FGameplayTag();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FGameplayTag AbilityStatus = FGameplayTag();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FGameplayTag AbilitySlot = FGameplayTag();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FGameplayTag AbilityType = FGameplayTag();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    int32 AbilityLevel = 1;
};

inline bool operator==(const FSavedAbility& Left, const FSavedAbility& Right)
{
    return Left.AbilityTag.MatchesTagExact(Right.AbilityTag);
}

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
    FString MapName = FString("Default Map Name");

    UPROPERTY()
    FString MapAssetName = FString("Default Map Asset Name");

    UPROPERTY()
    FName PlayerStartTag;

    UPROPERTY()
    TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = Vacant;

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
};
