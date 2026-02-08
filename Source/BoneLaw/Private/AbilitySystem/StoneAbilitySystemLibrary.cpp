// Copyright by MykeUhu

#include "AbilitySystem/StoneAbilitySystemLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

#include "Core/StoneGameplayTags.h"
#include "UI/WidgetController/StoneOverlayWidgetController.h"
#include "UI/WidgetController/StoneWidgetController.h" // falls FStoneWidgetControllerParams dort liegt

// Optional: falls AbilityInfo im GameMode hängt
#include "Core/StoneAbilityTypes.h"
#include "Core/StoneGameMode.h"
#include "Core/LoadScreenSaveGame.h"
#include "Core/StonePlayerState.h"
#include "UI/HUD/StoneHUD.h"


bool UStoneAbilitySystemLibrary::MakeWidgetControllerParams(
	const UObject* WorldContextObject,
	FWidgetControllerParams& OutWCParams,
	AStoneHUD*& OutStoneHUD)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		OutStoneHUD = Cast<AStoneHUD>(PC->GetHUD());
		if (OutStoneHUD)
		{
			AStonePlayerState* PS = PC->GetPlayerState<AStonePlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();

			OutWCParams.AttributeSet = AS;
			OutWCParams.AbilitySystemComponent = ASC;
			OutWCParams.PlayerState = PS;
			OutWCParams.PlayerController = PC;
			return true;
		}
	}
	return false;
}

UStoneOverlayWidgetController* UStoneAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AStoneHUD* StoneHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, StoneHUD))
	{
		return StoneHUD->GetOverlayWidgetController(WCParams);
	}
	return nullptr;
}

void UStoneAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, EStoneCharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	AActor* AvatarActor = ASC->GetAvatarActor();

	UStoneCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	FStoneCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	FGameplayEffectContextHandle PrimaryAttributesContextHandle = ASC->MakeEffectContext();
	PrimaryAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());

	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	SecondaryAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	FGameplayEffectContextHandle VitalAttributesContextHandle = ASC->MakeEffectContext();
	VitalAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
	
	FGameplayEffectContextHandle CultureAttributeContextHandle = ASC->MakeEffectContext();
	CultureAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle CultureAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->CultureAttributes, Level, CultureAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*CultureAttributesSpecHandle.Data.Get());
	
	FGameplayEffectContextHandle WorldlineAttributeContextHandle = ASC->MakeEffectContext();
	WorldlineAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle WorldlineAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->WorldlineAttributes, Level, WorldlineAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*WorldlineAttributesSpecHandle.Data.Get());
}

void UStoneAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(const UObject* WorldContextObject,
	UAbilitySystemComponent* ASC, ULoadScreenSaveGame* SaveGame)
{
	UStoneCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return;

	const FStoneGameplayTags& GameplayTags = FStoneGameplayTags::Get();

	const AActor* SourceAvatarActor = ASC->GetAvatarActor();

	FGameplayEffectContextHandle EffectContexthandle = ASC->MakeEffectContext();
	EffectContexthandle.AddSourceObject(SourceAvatarActor);

	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->PrimaryAttributes_SetByCaller, 1.f, EffectContexthandle);

	// Primary Attributes
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Strength, SaveGame->Strength);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Intelligence, SaveGame->Intelligence);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Endurance, SaveGame->Endurance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Willpower, SaveGame->Willpower);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Social, SaveGame->Social);
	
	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	SecondaryAttributesContextHandle.AddSourceObject(SourceAvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes_Infinite, 1.f, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	FGameplayEffectContextHandle VitalAttributesContextHandle = ASC->MakeEffectContext();
	VitalAttributesContextHandle.AddSourceObject(SourceAvatarActor);
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, 1.f, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
	
	FGameplayEffectContextHandle CultureContext = ASC->MakeEffectContext();
	CultureContext.AddSourceObject(SourceAvatarActor);
	const FGameplayEffectSpecHandle CultureSpec =
		ASC->MakeOutgoingSpec(CharacterClassInfo->CultureAttributes, 1.f, CultureContext);
	ASC->ApplyGameplayEffectSpecToSelf(*CultureSpec.Data.Get());

	FGameplayEffectContextHandle WorldlineContext = ASC->MakeEffectContext();
	WorldlineContext.AddSourceObject(SourceAvatarActor);
	const FGameplayEffectSpecHandle WorldlineSpec =
		ASC->MakeOutgoingSpec(CharacterClassInfo->WorldlineAttributes, 1.f, WorldlineContext);
	ASC->ApplyGameplayEffectSpecToSelf(*WorldlineSpec.Data.Get());
}

void UStoneAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC,
	EStoneCharacterClass CharacterClass)
{
	// Stone: no GameplayAbilities currently.
}

UStoneCharacterClassInfo* UStoneAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	const AStoneGameMode* StoneGameMode = Cast<AStoneGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (StoneGameMode == nullptr) return nullptr;
	return StoneGameMode->CharacterClassInfo;
}

UAbilityInfo* UStoneAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	const AStoneGameMode* StoneGameMode = Cast<AStoneGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (StoneGameMode == nullptr) return nullptr;
	return StoneGameMode->AbilityInfo;
}

bool UStoneAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FStoneGameplayEffectContext* StoneEffectContext = static_cast<const FStoneGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return StoneEffectContext->IsSuccessfulDebuff();
	}
	return false;
}

float UStoneAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FStoneGameplayEffectContext* StoneEffectContext = static_cast<const FStoneGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return StoneEffectContext->GetDebuffDamage();
	}
	return 0.f;
}

float UStoneAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FStoneGameplayEffectContext* StoneEffectContext = static_cast<const FStoneGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return StoneEffectContext->GetDebuffDuration();
	}
	return 0.f;
}

float UStoneAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FStoneGameplayEffectContext* StoneEffectContext = static_cast<const FStoneGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return StoneEffectContext->GetDebuffFrequency();
	}
	return 0.f;
}

void UStoneAbilitySystemLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInSuccessfulDebuff)
{
	if (FStoneGameplayEffectContext* StoneEffectContext = static_cast<FStoneGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		StoneEffectContext->SetIsSuccessfulDebuff(bInSuccessfulDebuff);
	}
}

void UStoneAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDamage)
{
	if (FStoneGameplayEffectContext* StoneEffectContext = static_cast<FStoneGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		StoneEffectContext->SetDebuffDamage(InDamage);
	}
}

void UStoneAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle, float InDuration)
{
	if (FStoneGameplayEffectContext* StoneEffectContext = static_cast<FStoneGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		StoneEffectContext->SetDebuffDuration(InDuration);
	}
}

void UStoneAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle,
	float InFrequency)
{
	if (FStoneGameplayEffectContext* StoneEffectContext = static_cast<FStoneGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		StoneEffectContext->SetDebuffFrequency(InFrequency);
	}
}

