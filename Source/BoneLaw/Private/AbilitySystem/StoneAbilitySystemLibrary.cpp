// Copyright by MykeUhu

#include "AbilitySystem/StoneAbilitySystemLibrary.h"

#include "Game/Save/StoneSaveGame.h"

#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

#include "Core/StoneGameplayTags.h"
#include "UI/WidgetController/StoneOverlayWidgetController.h"
#include "UI/WidgetController/StoneWidgetController.h" // falls FStoneWidgetControllerParams dort liegt

// Optional: wenn du konkrete Attribute setzen willst (direkt), include dein Set:
#include "AbilitySystem/StoneAttributeSet.h"

// Optional: falls AbilityInfo im GameMode hängt
#include "Core/StoneGameMode.h"
#include "Core/StonePlayerState.h"
#include "GameFramework/PlayerState.h"
#include "UI/HUD/StoneHUD.h"

bool UStoneAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject, FStoneWidgetControllerParams& OutParams, AStoneHUD*& OutStoneHUD)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		OutStoneHUD = Cast<AStoneHUD>(PC->GetHUD());
		if (OutStoneHUD)
		{
			AStonePlayerState* PS = PC->GetPlayerState<AStonePlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			UStoneRunSubsystem* RunSS = PC->GetGameInstance()->GetSubsystem<UStoneRunSubsystem>();
			
			// Params befüllen (nur Felder setzen, die du in deinem Struct wirklich hast)
			OutParams.PlayerController = PC;
			OutParams.PlayerState = PC->PlayerState;
			OutParams.AbilitySystemComponent = ASC;
			OutParams.AttributeSet = AS;
			OutParams.RunSubsystem = RunSS;
			
			return true;
		}
	}
	return true;
}

UStoneOverlayWidgetController* UStoneAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FStoneWidgetControllerParams WCParams;
	AStoneHUD* StoneHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, StoneHUD))
	{
		return StoneHUD->GetOverlayWidgetController(WCParams);
	}
	return nullptr;
}

void UStoneAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, UStoneSaveGame* SaveGame)
{
	if (!ASC || !SaveGame) return;

	const UStoneAttributeSet* AttrCDO = GetDefault<UStoneAttributeSet>();
	if (!AttrCDO) return;

	const FStoneGameplayTags& Tags = FStoneGameplayTags::Get();

	auto Clamp01_100 = [](float V) { return FMath::Clamp(V, 0.f, 100.f); };

	// Apply MaxHealth first (health depends on it)
	if (const float* SavedMax = SaveGame->AttributeValues.Find(Tags.Attributes_Vital_MaxHealth.GetTagName()))
	{
		const float MaxValue = FMath::Max(*SavedMax, 1.f);
		ASC->SetNumericAttributeBase(UStoneAttributeSet::GetMaxHealthAttribute(), MaxValue);
	}

	for (const TPair<FName, float>& It : SaveGame->AttributeValues)
	{
		const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(It.Key, /*ErrorIfNotFound*/ false);
		if (!Tag.IsValid()) continue;

		const TStaticFuncPtr<FGameplayAttribute()>* Getter = AttrCDO->TagsToAttributes.Find(Tag);
		if (!Getter) continue;

		FGameplayAttribute Attribute = (*Getter)();

		float Value = It.Value;

		// Clamp known ranges (SetNumericAttributeBase bypasses PreAttributeChange)
		if (Tag == Tags.Attributes_Vital_MaxHealth)
		{
			Value = FMath::Max(Value, 1.f);
		}
		else if (Tag == Tags.Attributes_Vital_Health)
		{
			const float CurrentMax = ASC->GetNumericAttribute(UStoneAttributeSet::GetMaxHealthAttribute());
			Value = FMath::Clamp(Value, 0.f, FMath::Max(CurrentMax, 1.f));
		}
		else
		{
			const FString TagStr = Tag.ToString();
			if (TagStr.StartsWith(TEXT("Attributes.Vital.")) || TagStr.StartsWith(TEXT("Attributes.Culture.")) || TagStr.StartsWith(TEXT("Attributes.Knowledge.")) || TagStr.StartsWith(TEXT("Attributes.Worldline.")))
			{
				Value = Clamp01_100(Value);
			}
		}

		ASC->SetNumericAttributeBase(Attribute, Value);
	}

	// Final safety: health <= max health
	const float FinalMax = FMath::Max(ASC->GetNumericAttribute(UStoneAttributeSet::GetMaxHealthAttribute()), 1.f);
	const float FinalHealth = FMath::Clamp(ASC->GetNumericAttribute(UStoneAttributeSet::GetHealthAttribute()), 0.f, FinalMax);
	ASC->SetNumericAttributeBase(UStoneAttributeSet::GetHealthAttribute(), FinalHealth);
}

UAbilityInfo* UStoneAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	AGameModeBase* GM = UGameplayStatics::GetGameMode(WorldContextObject);
	AStoneGameMode* StoneGM = Cast<AStoneGameMode>(GM);
	return StoneGM ? StoneGM->AbilityInfo : nullptr;
}
