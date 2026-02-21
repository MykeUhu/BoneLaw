// Copyright by MykeUhu

#include "UI/ViewModel/MVVM_SettlerSlotDetails.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "Core/Character/StoneBaseChar.h"
#include "GameplayEffectTypes.h"
#include "Core/Character/StoneSettlerChar.h"

// -------------------------
// Details lifecycle
// -------------------------

void UMVVM_SettlerSlotDetails::BindToSettler(const FGuid& SettlerId, AStoneSettlerChar* SettlerActor)
{
	// Always clean previous binding first (important if user clicks quickly between slots)
	Unbind();

	BoundSettlerId = SettlerId;
	BoundSettler = SettlerActor;

	if (!BoundSettler)
	{
		return;
	}

	// Try immediate ASC
	if (UAbilitySystemComponent* ASC = BoundSettler->GetAbilitySystemComponent())
	{
		HandleASCRegistered(ASC);
		return;
	}

	// ASC not ready yet -> wait for registration (Aura-like pattern)
	AscRegisteredHandle = BoundSettler->OnAscRegistered.AddUObject(this, &UMVVM_SettlerSlotDetails::HandleASCRegistered);

	// Optional: reset displayed values while waiting
	SetHealth(0.f);
	SetMaxHealth(0.f);
	SetHealthPct(0.f);

	SetFood(0.f);
	SetMaxFood(0.f);
	SetFoodPct(0.f);

	SetWater(0.f);
	SetMaxWater(0.f);
	SetWaterPct(0.f);

	SetWarmth(0.f);
	SetWarmthPct(0.f);

	SetMorale(0.f);
	SetMaxMorale(0.f);
	SetMoralePct(0.f);
}

void UMVVM_SettlerSlotDetails::Unbind()
{
	UnbindAttributeDelegates();

	// Unbind ASC-registered delegate
	if (BoundSettler && AscRegisteredHandle.IsValid())
	{
		BoundSettler->OnAscRegistered.Remove(AscRegisteredHandle);
		AscRegisteredHandle.Reset();
	}

	BoundASC = nullptr;
	BoundSettler = nullptr;

	// Reset MVVM fields (so UI doesn't keep stale values)
	SetHealth(0.f);
	SetMaxHealth(0.f);
	SetHealthPct(0.f);

	SetFood(0.f);
	SetMaxFood(0.f);
	SetFoodPct(0.f);

	SetWater(0.f);
	SetMaxWater(0.f);
	SetWaterPct(0.f);

	SetWarmth(0.f);
	SetWarmthPct(0.f);

	SetMorale(0.f);
	SetMaxMorale(0.f);
	SetMoralePct(0.f);

	BoundSettlerId.Invalidate();
}

void UMVVM_SettlerSlotDetails::BeginDestroy()
{
	Unbind();
	Super::BeginDestroy();
}

// -------------------------
// GAS binding
// -------------------------

void UMVVM_SettlerSlotDetails::HandleASCRegistered(UAbilitySystemComponent* InASC)
{
	if (!InASC)
	{
		return;
	}

	// Stop waiting
	if (BoundSettler && AscRegisteredHandle.IsValid())
	{
		BoundSettler->OnAscRegistered.Remove(AscRegisteredHandle);
		AscRegisteredHandle.Reset();
	}

	// If we were already bound to something else, make sure delegates are gone.
	UnbindAttributeDelegates();

	BoundASC = InASC;

	// Initial push (read current values)
	const UStoneAttributeSet* AS = BoundASC->GetSet<UStoneAttributeSet>();
	if (AS)
	{
		SetHealth(AS->GetHealth());
		SetMaxHealth(AS->GetMaxHealth());

		SetFood(AS->GetFood());
		SetMaxFood(AS->GetMaxFood());

		SetWater(AS->GetWater());
		SetMaxWater(AS->GetMaxWater());

		SetWarmth(AS->GetWarmth());

		SetMorale(AS->GetMorale());
		SetMaxMorale(AS->GetMaxMorale());
	}
	else
	{
		SetHealth(0.f);
		SetMaxHealth(0.f);

		SetFood(0.f);
		SetMaxFood(0.f);

		SetWater(0.f);
		SetMaxWater(0.f);

		SetWarmth(0.f);

		SetMorale(0.f);
		SetMaxMorale(0.f);
	}

	RecomputePct();
	BindAttributeDelegates();
}

void UMVVM_SettlerSlotDetails::BindAttributeDelegates()
{
	if (!BoundASC)
	{
		return;
	}

	const FGameplayAttribute A_Health     = UStoneAttributeSet::GetHealthAttribute();
	const FGameplayAttribute A_MaxHealth  = UStoneAttributeSet::GetMaxHealthAttribute();

	const FGameplayAttribute A_Food       = UStoneAttributeSet::GetFoodAttribute();
	const FGameplayAttribute A_MaxFood    = UStoneAttributeSet::GetMaxFoodAttribute();

	const FGameplayAttribute A_Water      = UStoneAttributeSet::GetWaterAttribute();
	const FGameplayAttribute A_MaxWater   = UStoneAttributeSet::GetMaxWaterAttribute();

	const FGameplayAttribute A_Warmth     = UStoneAttributeSet::GetWarmthAttribute();

	const FGameplayAttribute A_Morale     = UStoneAttributeSet::GetMoraleAttribute();
	const FGameplayAttribute A_MaxMorale  = UStoneAttributeSet::GetMaxMoraleAttribute();

	HealthChangedHandle     = BoundASC->GetGameplayAttributeValueChangeDelegate(A_Health).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleHealthChanged);
	MaxHealthChangedHandle  = BoundASC->GetGameplayAttributeValueChangeDelegate(A_MaxHealth).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleMaxHealthChanged);

	FoodChangedHandle       = BoundASC->GetGameplayAttributeValueChangeDelegate(A_Food).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleFoodChanged);
	MaxFoodChangedHandle    = BoundASC->GetGameplayAttributeValueChangeDelegate(A_MaxFood).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleMaxFoodChanged);

	WaterChangedHandle      = BoundASC->GetGameplayAttributeValueChangeDelegate(A_Water).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleWaterChanged);
	MaxWaterChangedHandle   = BoundASC->GetGameplayAttributeValueChangeDelegate(A_MaxWater).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleMaxWaterChanged);

	WarmthChangedHandle     = BoundASC->GetGameplayAttributeValueChangeDelegate(A_Warmth).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleWarmthChanged);

	MoraleChangedHandle     = BoundASC->GetGameplayAttributeValueChangeDelegate(A_Morale).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleMoraleChanged);
	MaxMoraleChangedHandle  = BoundASC->GetGameplayAttributeValueChangeDelegate(A_MaxMorale).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleMaxMoraleChanged);
}

void UMVVM_SettlerSlotDetails::UnbindAttributeDelegates()
{
	if (!BoundASC)
	{
		return;
	}

	auto Remove = [&](const FGameplayAttribute& Attr, FDelegateHandle& Handle)
	{
		if (Handle.IsValid())
		{
			BoundASC->GetGameplayAttributeValueChangeDelegate(Attr).Remove(Handle);
			Handle.Reset();
		}
	};

	Remove(UStoneAttributeSet::GetHealthAttribute(), HealthChangedHandle);
	Remove(UStoneAttributeSet::GetMaxHealthAttribute(), MaxHealthChangedHandle);

	Remove(UStoneAttributeSet::GetFoodAttribute(), FoodChangedHandle);
	Remove(UStoneAttributeSet::GetMaxFoodAttribute(), MaxFoodChangedHandle);

	Remove(UStoneAttributeSet::GetWaterAttribute(), WaterChangedHandle);
	Remove(UStoneAttributeSet::GetMaxWaterAttribute(), MaxWaterChangedHandle);

	Remove(UStoneAttributeSet::GetWarmthAttribute(), WarmthChangedHandle);

	Remove(UStoneAttributeSet::GetMoraleAttribute(), MoraleChangedHandle);
	Remove(UStoneAttributeSet::GetMaxMoraleAttribute(), MaxMoraleChangedHandle);
}

// -------------------------
// Handlers
// -------------------------

void UMVVM_SettlerSlotDetails::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	SetHealth(Data.NewValue);
	RecomputePct();
}

void UMVVM_SettlerSlotDetails::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	SetMaxHealth(Data.NewValue);
	RecomputePct();
}

void UMVVM_SettlerSlotDetails::HandleFoodChanged(const FOnAttributeChangeData& Data)
{
	SetFood(Data.NewValue);
	RecomputePct();
}

void UMVVM_SettlerSlotDetails::HandleMaxFoodChanged(const FOnAttributeChangeData& Data)
{
	SetMaxFood(Data.NewValue);
	RecomputePct();
}

void UMVVM_SettlerSlotDetails::HandleWaterChanged(const FOnAttributeChangeData& Data)
{
	SetWater(Data.NewValue);
	RecomputePct();
}

void UMVVM_SettlerSlotDetails::HandleMaxWaterChanged(const FOnAttributeChangeData& Data)
{
	SetMaxWater(Data.NewValue);
	RecomputePct();
}

void UMVVM_SettlerSlotDetails::HandleWarmthChanged(const FOnAttributeChangeData& Data)
{
	SetWarmth(Data.NewValue);
	RecomputePct();
}

void UMVVM_SettlerSlotDetails::HandleMoraleChanged(const FOnAttributeChangeData& Data)
{
	SetMorale(Data.NewValue);
	RecomputePct();
}

void UMVVM_SettlerSlotDetails::HandleMaxMoraleChanged(const FOnAttributeChangeData& Data)
{
	SetMaxMorale(Data.NewValue);
	RecomputePct();
}

// -------------------------
// Helpers
// -------------------------

void UMVVM_SettlerSlotDetails::RecomputePct()
{
	SetHealthPct((MaxHealth > 0.f) ? (Health / MaxHealth) : 0.f);
	SetFoodPct((MaxFood > 0.f) ? (Food / MaxFood) : 0.f);
	SetWaterPct((MaxWater > 0.f) ? (Water / MaxWater) : 0.f);

	// Warmth has no max in your header -> keep it as "value only" pct
	// If you later add MaxWarmth -> just mirror pattern.
	SetWarmthPct(0.f);

	SetMoralePct((MaxMorale > 0.f) ? (Morale / MaxMorale) : 0.f);
}

// -------------------------
// MVVM setters
// -------------------------

void UMVVM_SettlerSlotDetails::SetHealth(float InValue)         { UE_MVVM_SET_PROPERTY_VALUE(Health, InValue); }
void UMVVM_SettlerSlotDetails::SetMaxHealth(float InValue)      { UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, InValue); }
void UMVVM_SettlerSlotDetails::SetHealthPct(float InValue)      { UE_MVVM_SET_PROPERTY_VALUE(HealthPct, InValue); }

void UMVVM_SettlerSlotDetails::SetFood(float InValue)           { UE_MVVM_SET_PROPERTY_VALUE(Food, InValue); }
void UMVVM_SettlerSlotDetails::SetMaxFood(float InValue)        { UE_MVVM_SET_PROPERTY_VALUE(MaxFood, InValue); }
void UMVVM_SettlerSlotDetails::SetFoodPct(float InValue)        { UE_MVVM_SET_PROPERTY_VALUE(FoodPct, InValue); }

void UMVVM_SettlerSlotDetails::SetWater(float InValue)          { UE_MVVM_SET_PROPERTY_VALUE(Water, InValue); }
void UMVVM_SettlerSlotDetails::SetMaxWater(float InValue)       { UE_MVVM_SET_PROPERTY_VALUE(MaxWater, InValue); }
void UMVVM_SettlerSlotDetails::SetWaterPct(float InValue)       { UE_MVVM_SET_PROPERTY_VALUE(WaterPct, InValue); }

void UMVVM_SettlerSlotDetails::SetWarmth(float InValue)         { UE_MVVM_SET_PROPERTY_VALUE(Warmth, InValue); }
void UMVVM_SettlerSlotDetails::SetWarmthPct(float InValue)      { UE_MVVM_SET_PROPERTY_VALUE(WarmthPct, InValue); }

void UMVVM_SettlerSlotDetails::SetMorale(float InValue)         { UE_MVVM_SET_PROPERTY_VALUE(Morale, InValue); }
void UMVVM_SettlerSlotDetails::SetMaxMorale(float InValue)      { UE_MVVM_SET_PROPERTY_VALUE(MaxMorale, InValue); }
void UMVVM_SettlerSlotDetails::SetMoralePct(float InValue)      { UE_MVVM_SET_PROPERTY_VALUE(MoralePct, InValue); }