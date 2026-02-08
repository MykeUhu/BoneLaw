// StoneCharacterClassInfo.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ScalableFloat.h"
#include "StoneCharacterClassInfo.generated.h"

class UGameplayEffect;
class UGameplayAbility;

UENUM(BlueprintType)
enum class EStoneCharacterClass : uint8
{
    Hunter       UMETA(DisplayName="Hunter"),
    Gatherer     UMETA(DisplayName="Gatherer"),
    Shaman       UMETA(DisplayName="Shaman"),
    Builder      UMETA(DisplayName="Builder"),
    Scout        UMETA(DisplayName="Scout")
};

USTRUCT(BlueprintType)
struct FStoneCharacterClassDefaultInfo
{
    GENERATED_BODY()

    // === Primary Attributes (per class) ===
    UPROPERTY(EditDefaultsOnly, Category="Class Defaults|Attributes")
    TSubclassOf<UGameplayEffect> PrimaryAttributes;

    // === Startup Abilities (per class, optional) ===
    UPROPERTY(EditDefaultsOnly, Category="Class Defaults|Abilities")
    TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

    UPROPERTY(EditDefaultsOnly, Category="Class Defaults|Abilities")
    TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;
};

UCLASS()
class BONELAW_API UStoneCharacterClassInfo : public UDataAsset
{
    GENERATED_BODY()
public:
    // Per class defaults (Aura pattern)
    UPROPERTY(EditDefaultsOnly, Category="Character Class Defaults")
    TMap<EStoneCharacterClass, FStoneCharacterClassDefaultInfo> CharacterClassInformation;

    // === Common Defaults (Aura pattern, expanded for your attribute groups) ===
    UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults|Attributes")
    TSubclassOf<UGameplayEffect> PrimaryAttributes_SetByCaller;

    UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults|Attributes")
    TSubclassOf<UGameplayEffect> SecondaryAttributes;

    UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults|Attributes")
    TSubclassOf<UGameplayEffect> SecondaryAttributes_Infinite;

    UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults|Attributes")
    TSubclassOf<UGameplayEffect> VitalAttributes;

    // Your extra groups (still Aura pattern: common GE blocks)
    UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults|Attributes")
    TSubclassOf<UGameplayEffect> CultureAttributes;

    UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults|Attributes")
    TSubclassOf<UGameplayEffect> KnowledgeAttributes;

    UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults|Attributes")
    TSubclassOf<UGameplayEffect> WorldlineAttributes;

    // Abilities (optional)
    UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults|Abilities")
    TArray<TSubclassOf<UGameplayAbility>> CommonAbilities;

    UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults|Abilities")
    TArray<TSubclassOf<UGameplayAbility>> CommonPassiveAbilities;

    UFUNCTION(BlueprintPure)
    FStoneCharacterClassDefaultInfo GetClassDefaultInfo(EStoneCharacterClass CharacterClass);
};
