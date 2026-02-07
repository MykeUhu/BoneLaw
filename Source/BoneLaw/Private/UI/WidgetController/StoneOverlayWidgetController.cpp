// Copyright by MykeUhu
// Following Stone pattern - typed getters inherited from base

#include "UI/WidgetController/StoneOverlayWidgetController.h"

#include "AbilitySystem/StoneAttributeSet.h"
#include "Data/StoneEventData.h"

void UStoneOverlayWidgetController::BroadcastInitialValues()
{
	UStoneRunSubsystem* RunSS = GetRunSubsystem();
	if (!RunSS) return;

	HandleSnapshotChanged(RunSS->GetSnapshot());
	HandleEventChanged(RunSS->GetCurrentEvent());
	
	OnHealthChanged.Broadcast(GetStoneAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetStoneAS()->GetMaxHealth());

	OnFoodChanged.Broadcast(GetStoneAS()->GetFood());
	OnMaxFoodChanged.Broadcast(GetStoneAS()->GetMaxFood());

	OnWaterChanged.Broadcast(GetStoneAS()->GetWater());
	OnMaxWaterChanged.Broadcast(GetStoneAS()->GetMaxWater());

	OnWarmthChanged.Broadcast(GetStoneAS()->GetWarmth());
	OnMaxWarmthChanged.Broadcast(GetStoneAS()->GetMaxWarmth());

	OnMoraleChanged.Broadcast(GetStoneAS()->GetMorale());
	OnMaxMoraleChanged.Broadcast(GetStoneAS()->GetMaxMorale());

	OnTrustChanged.Broadcast(GetStoneAS()->GetTrust());
	OnMaxTrustChanged.Broadcast(GetStoneAS()->GetMaxTrust());
}

void UStoneOverlayWidgetController::BindCallbacksToDependencies()
{
	UStoneRunSubsystem* RunSS = GetRunSubsystem();
	if (!RunSS) return;

	RunSS->OnSnapshotChanged.RemoveDynamic(this, &UStoneOverlayWidgetController::HandleSnapshotChanged);
	RunSS->OnSnapshotChanged.AddDynamic(this, &UStoneOverlayWidgetController::HandleSnapshotChanged);

	RunSS->OnEventChanged.RemoveDynamic(this, &UStoneOverlayWidgetController::HandleEventChanged);
	RunSS->OnEventChanged.AddDynamic(this, &UStoneOverlayWidgetController::HandleEventChanged);
	
	// Attributes
	check(AbilitySystemComponent);
	check(GetStoneAS());

	// --- Health ---
	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(GetStoneAS()->GetHealthAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		});

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(GetStoneAS()->GetMaxHealthAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		});

	// --- Food ---
	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(GetStoneAS()->GetFoodAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnFoodChanged.Broadcast(Data.NewValue);
		});

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(GetStoneAS()->GetMaxFoodAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnMaxFoodChanged.Broadcast(Data.NewValue);
		});

	// --- Water ---
	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(GetStoneAS()->GetWaterAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnWaterChanged.Broadcast(Data.NewValue);
		});

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(GetStoneAS()->GetMaxWaterAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnMaxWaterChanged.Broadcast(Data.NewValue);
		});

	// --- Warmth ---
	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(GetStoneAS()->GetWarmthAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnWarmthChanged.Broadcast(Data.NewValue);
		});

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(GetStoneAS()->GetMaxWarmthAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnMaxWarmthChanged.Broadcast(Data.NewValue);
		});

	// --- Morale ---
	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(GetStoneAS()->GetMoraleAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnMoraleChanged.Broadcast(Data.NewValue);
		});

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(GetStoneAS()->GetMaxMoraleAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnMaxMoraleChanged.Broadcast(Data.NewValue);
		});

	// --- Trust ---
	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(GetStoneAS()->GetTrustAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnTrustChanged.Broadcast(Data.NewValue);
		});

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(GetStoneAS()->GetMaxTrustAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnMaxTrustChanged.Broadcast(Data.NewValue);
		});
}

UStoneRunSubsystem* UStoneOverlayWidgetController::GetRunSubsystem() const
{
	if (CachedRunSubsystem == nullptr)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(PlayerController, EGetWorldErrorMode::LogAndReturnNull))
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				CachedRunSubsystem = GI->GetSubsystem<UStoneRunSubsystem>();
			}
		}
	}
	return CachedRunSubsystem;
}

void UStoneOverlayWidgetController::HandleSnapshotChanged(const FStoneSnapshot& Snapshot)
{
	OnOverlaySnapshotChanged.Broadcast(Snapshot);
}

void UStoneOverlayWidgetController::HandleEventChanged(const UStoneEventData* Event)
{
	OnOverlayEventChanged.Broadcast(Event);
}

void UStoneOverlayWidgetController::GetResolvedChoices(TArray<FStoneChoiceResolved>& OutChoices) const
{
	OutChoices.Reset();
	UStoneRunSubsystem* RunSS = GetRunSubsystem();
	if (!RunSS) return;

	RunSS->GetResolvedChoices(OutChoices);
}
