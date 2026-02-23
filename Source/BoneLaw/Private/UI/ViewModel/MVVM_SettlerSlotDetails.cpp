// Copyright by MykeUhu

#include "UI/ViewModel/MVVM_SettlerSlotDetails.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "Core/Character/StoneBaseChar.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystem/StoneAbilitySystemLibrary.h"
#include "Core/Character/StoneSettlerChar.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "Core/StoneGameplayTags.h"

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
	
	// Primary Attributes
	SetStrength(0.f);
	SetIntelligence(0.f);
	SetEndurance(0.f);
	SetWillpower(0.f);
	SetSocial(0.f);
	
	// Secondary Attributes
	SetCarryCapacity(0.f);
	SetTravelSpeed(0.f);
	SetCraftSpeed(0.f);
	SetGatherEfficiency(0.f);
	SetInjuryResistance(0.f);
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
	
	// Primary Attributes
	SetStrength(0.f);
	SetIntelligence(0.f);
	SetEndurance(0.f);
	SetWillpower(0.f);
	SetSocial(0.f);
	
	// Secondary Attributes
	SetCarryCapacity(0.f);
	SetTravelSpeed(0.f);
	SetCraftSpeed(0.f);
	SetGatherEfficiency(0.f);
	SetInjuryResistance(0.f);

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
		
		// Primary Attriubtes
		SetStrength(AS->GetStrength());
		SetIntelligence(AS->GetIntelligence());
		SetEndurance(AS->GetEndurance());
		SetWillpower(AS->GetWillpower());
		SetSocial(AS->GetSocial());
		
		// Secondary Attributes
		SetCarryCapacity(AS->GetCarryCapacity());
		SetTravelSpeed(AS->GetTravelSpeed());
		SetCraftSpeed(AS->GetCraftSpeed());
		SetGatherEfficiency(AS->GetGatherEfficiency());
		SetInjuryResistance(AS->GetInjuryResistance());
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
		
		//Primary Attriubtes
		SetStrength(0.f);
		SetIntelligence(0.f);
		SetEndurance(0.f);
		SetWillpower(0.f);
		SetSocial(0.f);
		
		// Secondary Attributes
		SetCarryCapacity(0.f);
		SetTravelSpeed(0.f);
		SetCraftSpeed(0.f);
		SetGatherEfficiency(0.f);
		SetInjuryResistance(0.f);
	}

	RecomputePct();
	InitializeAttributeIcons();
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
	
	// Primary Attributes
	const FGameplayAttribute A_Strength   = UStoneAttributeSet::GetStrengthAttribute();
	const FGameplayAttribute A_Intelligence   = UStoneAttributeSet::GetIntelligenceAttribute();
	const FGameplayAttribute A_Endurance   = UStoneAttributeSet::GetEnduranceAttribute();
	const FGameplayAttribute A_Willpower   = UStoneAttributeSet::GetWillpowerAttribute();
	const FGameplayAttribute A_Social   = UStoneAttributeSet::GetSocialAttribute();
	
	const FGameplayAttribute A_CarryCapacity = UStoneAttributeSet::GetCarryCapacityAttribute();
	const FGameplayAttribute A_TravelSpeed = UStoneAttributeSet::GetTravelSpeedAttribute();
	const FGameplayAttribute A_CraftSpeed = UStoneAttributeSet::GetCraftSpeedAttribute();
	const FGameplayAttribute A_GatherEfficiency = UStoneAttributeSet::GetGatherEfficiencyAttribute();
	const FGameplayAttribute A_InjuryResistance = UStoneAttributeSet::GetInjuryResistanceAttribute();

	HealthChangedHandle			= BoundASC->GetGameplayAttributeValueChangeDelegate(A_Health).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleHealthChanged);
	MaxHealthChangedHandle		= BoundASC->GetGameplayAttributeValueChangeDelegate(A_MaxHealth).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleMaxHealthChanged);

	FoodChangedHandle			= BoundASC->GetGameplayAttributeValueChangeDelegate(A_Food).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleFoodChanged);
	MaxFoodChangedHandle		= BoundASC->GetGameplayAttributeValueChangeDelegate(A_MaxFood).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleMaxFoodChanged);

	WaterChangedHandle			= BoundASC->GetGameplayAttributeValueChangeDelegate(A_Water).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleWaterChanged);
	MaxWaterChangedHandle		= BoundASC->GetGameplayAttributeValueChangeDelegate(A_MaxWater).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleMaxWaterChanged);

	WarmthChangedHandle			= BoundASC->GetGameplayAttributeValueChangeDelegate(A_Warmth).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleWarmthChanged);

	MoraleChangedHandle			= BoundASC->GetGameplayAttributeValueChangeDelegate(A_Morale).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleMoraleChanged);
	MaxMoraleChangedHandle		= BoundASC->GetGameplayAttributeValueChangeDelegate(A_MaxMorale).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleMaxMoraleChanged);
	
	// Primary Attributes
	StrengthChangedHandle		= BoundASC->GetGameplayAttributeValueChangeDelegate(A_Strength).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleStrengthChanged);
	IntelligenceChangedHandle	= BoundASC->GetGameplayAttributeValueChangeDelegate(A_Intelligence).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleIntelligenceChanged);
	EnduranceChangedHandle		= BoundASC->GetGameplayAttributeValueChangeDelegate(A_Endurance).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleEnduranceChanged);
	WillpowerChangedHandle		= BoundASC->GetGameplayAttributeValueChangeDelegate(A_Willpower).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleWillpowerChanged);
	SocialChangedHandle			= BoundASC->GetGameplayAttributeValueChangeDelegate(A_Social).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleSocialChanged);
	
	// Secondary Attributes
	CarryCapacityChangedHandle	 = BoundASC->GetGameplayAttributeValueChangeDelegate(A_CarryCapacity).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleCarryCapacityChanged);
	TravelSpeedChangedHandle	 = BoundASC->GetGameplayAttributeValueChangeDelegate(A_TravelSpeed).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleTravelSpeedChanged);
	CraftSpeedChangedHandle		 = BoundASC->GetGameplayAttributeValueChangeDelegate(A_CraftSpeed).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleCraftSpeedChanged);
	GatherEfficiencyChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(A_GatherEfficiency).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleGatherEfficiencyChanged);
	InjuryResistanceChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(A_InjuryResistance).AddUObject(this, &UMVVM_SettlerSlotDetails::HandleInjuryResistanceChanged);
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
	
	//Primary Attributes
	Remove(UStoneAttributeSet::GetStrengthAttribute(), StrengthChangedHandle);
	Remove(UStoneAttributeSet::GetIntelligenceAttribute(), IntelligenceChangedHandle);
	Remove(UStoneAttributeSet::GetEnduranceAttribute(), EnduranceChangedHandle);
	Remove(UStoneAttributeSet::GetWillpowerAttribute(), WillpowerChangedHandle);
	Remove(UStoneAttributeSet::GetSocialAttribute(), SocialChangedHandle);
	
	// Secondary Attributes
	Remove(UStoneAttributeSet::GetCarryCapacityAttribute(), CarryCapacityChangedHandle);
	Remove(UStoneAttributeSet::GetTravelSpeedAttribute(), TravelSpeedChangedHandle);
	Remove(UStoneAttributeSet::GetCraftSpeedAttribute(), CraftSpeedChangedHandle);
	Remove(UStoneAttributeSet::GetGatherEfficiencyAttribute(), GatherEfficiencyChangedHandle);
	Remove(UStoneAttributeSet::GetInjuryResistanceAttribute(), InjuryResistanceChangedHandle);
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

// Primary Attributes
void UMVVM_SettlerSlotDetails::HandleStrengthChanged(const FOnAttributeChangeData& Data)      { SetStrength(Data.NewValue); }
void UMVVM_SettlerSlotDetails::HandleIntelligenceChanged(const FOnAttributeChangeData& Data)  { SetIntelligence(Data.NewValue); }
void UMVVM_SettlerSlotDetails::HandleEnduranceChanged(const FOnAttributeChangeData& Data)     { SetEndurance(Data.NewValue); }
void UMVVM_SettlerSlotDetails::HandleWillpowerChanged(const FOnAttributeChangeData& Data)     { SetWillpower(Data.NewValue); }
void UMVVM_SettlerSlotDetails::HandleSocialChanged(const FOnAttributeChangeData& Data)        { SetSocial(Data.NewValue); }

// Secondary Attributes
void UMVVM_SettlerSlotDetails::HandleCarryCapacityChanged(const FOnAttributeChangeData& Data)     { SetCarryCapacity(Data.NewValue); }
void UMVVM_SettlerSlotDetails::HandleTravelSpeedChanged(const FOnAttributeChangeData& Data)       { SetTravelSpeed(Data.NewValue); }
void UMVVM_SettlerSlotDetails::HandleCraftSpeedChanged(const FOnAttributeChangeData& Data)        { SetCraftSpeed(Data.NewValue); }
void UMVVM_SettlerSlotDetails::HandleGatherEfficiencyChanged(const FOnAttributeChangeData& Data)  { SetGatherEfficiency(Data.NewValue); }
void UMVVM_SettlerSlotDetails::HandleInjuryResistanceChanged(const FOnAttributeChangeData& Data)  { SetInjuryResistance(Data.NewValue); }


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

void UMVVM_SettlerSlotDetails::InitializeAttributeIcons()
{
	if (!AttributeInfo)
	{
		AttributeInfo = UStoneAbilitySystemLibrary::GetAttributeInfo(this);
	}

	if (!AttributeInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMVVM_SettlerSlotDetails::InitializeAttributeIcons - AttributeInfo is not set. Icons will not be displayed."));
		return;
	}

	const FStoneGameplayTags& GameplayTags = FStoneGameplayTags::Get();

	SetHealthIcon(MakeBrushFromAttributeTag(GameplayTags.Attributes_Vital_Health));
	SetFoodIcon(MakeBrushFromAttributeTag(GameplayTags.Attributes_Vital_Food));
	SetWaterIcon(MakeBrushFromAttributeTag(GameplayTags.Attributes_Vital_Water));
	SetWarmthIcon(MakeBrushFromAttributeTag(GameplayTags.Attributes_Vital_Warmth));
	SetMoraleIcon(MakeBrushFromAttributeTag(GameplayTags.Attributes_Vital_Morale));
}

FSlateBrush UMVVM_SettlerSlotDetails::MakeBrushFromAttributeTag(const FGameplayTag& AttributeTag) const
{
	FSlateBrush Brush;
	
	if (!AttributeInfo)
	{
		return Brush;
	}

	const FStoneAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag, false);
	
	if (Info.AttributeIcon)
	{
		Brush.SetResourceObject(Info.AttributeIcon);
		Brush.ImageSize = FVector2D(64.f, 64.f); // Default icon size, adjust as needed
		Brush.DrawAs = ESlateBrushDrawType::Image;
	}

	return Brush;
}

// -------------------------
// MVVM setters
// -------------------------

void UMVVM_SettlerSlotDetails::SetHealth(float InValue)					{ UE_MVVM_SET_PROPERTY_VALUE(Health, InValue); }
void UMVVM_SettlerSlotDetails::SetMaxHealth(float InValue)				{ UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, InValue); }
void UMVVM_SettlerSlotDetails::SetHealthPct(float InValue)				{ UE_MVVM_SET_PROPERTY_VALUE(HealthPct, InValue); }
void UMVVM_SettlerSlotDetails::SetHealthIcon(const FSlateBrush& InIcon) { UE_MVVM_SET_PROPERTY_VALUE(HealthIcon, InIcon); }

void UMVVM_SettlerSlotDetails::SetFood(float InValue)					{ UE_MVVM_SET_PROPERTY_VALUE(Food, InValue); }
void UMVVM_SettlerSlotDetails::SetMaxFood(float InValue)				{ UE_MVVM_SET_PROPERTY_VALUE(MaxFood, InValue); }
void UMVVM_SettlerSlotDetails::SetFoodPct(float InValue)				{ UE_MVVM_SET_PROPERTY_VALUE(FoodPct, InValue); }
void UMVVM_SettlerSlotDetails::SetFoodIcon(const FSlateBrush& InIcon) { UE_MVVM_SET_PROPERTY_VALUE(FoodIcon, InIcon); }

void UMVVM_SettlerSlotDetails::SetWater(float InValue)					{ UE_MVVM_SET_PROPERTY_VALUE(Water, InValue); }
void UMVVM_SettlerSlotDetails::SetMaxWater(float InValue)				{ UE_MVVM_SET_PROPERTY_VALUE(MaxWater, InValue); }
void UMVVM_SettlerSlotDetails::SetWaterPct(float InValue)				{ UE_MVVM_SET_PROPERTY_VALUE(WaterPct, InValue); }
void UMVVM_SettlerSlotDetails::SetWaterIcon(const FSlateBrush& InIcon) { UE_MVVM_SET_PROPERTY_VALUE(WaterIcon, InIcon); }

void UMVVM_SettlerSlotDetails::SetWarmth(float InValue)					{ UE_MVVM_SET_PROPERTY_VALUE(Warmth, InValue); }
void UMVVM_SettlerSlotDetails::SetWarmthPct(float InValue)				{ UE_MVVM_SET_PROPERTY_VALUE(WarmthPct, InValue); }
void UMVVM_SettlerSlotDetails::SetWarmthIcon(const FSlateBrush& InIcon) { UE_MVVM_SET_PROPERTY_VALUE(WarmthIcon, InIcon); }

void UMVVM_SettlerSlotDetails::SetMorale(float InValue)					{ UE_MVVM_SET_PROPERTY_VALUE(Morale, InValue); }
void UMVVM_SettlerSlotDetails::SetMaxMorale(float InValue)				{ UE_MVVM_SET_PROPERTY_VALUE(MaxMorale, InValue); }
void UMVVM_SettlerSlotDetails::SetMoralePct(float InValue)				{ UE_MVVM_SET_PROPERTY_VALUE(MoralePct, InValue); }
void UMVVM_SettlerSlotDetails::SetMoraleIcon(const FSlateBrush& InIcon) { UE_MVVM_SET_PROPERTY_VALUE(MoraleIcon, InIcon); }

// Primary Attributes
void UMVVM_SettlerSlotDetails::SetStrength(float InStrength)            { UE_MVVM_SET_PROPERTY_VALUE(Strength, InStrength); }
void UMVVM_SettlerSlotDetails::SetIntelligence(float InIntelligence)    { UE_MVVM_SET_PROPERTY_VALUE(Intelligence, InIntelligence); }
void UMVVM_SettlerSlotDetails::SetEndurance(float InEndurance)          { UE_MVVM_SET_PROPERTY_VALUE(Endurance, InEndurance); }
void UMVVM_SettlerSlotDetails::SetWillpower(float InWillpower)          { UE_MVVM_SET_PROPERTY_VALUE(Willpower, InWillpower); }
void UMVVM_SettlerSlotDetails::SetSocial(float InSocial)                { UE_MVVM_SET_PROPERTY_VALUE(Social, InSocial); }

// Secondary Attributes
void UMVVM_SettlerSlotDetails::SetCarryCapacity(float InCarryCapacity)          { UE_MVVM_SET_PROPERTY_VALUE(CarryCapacity, InCarryCapacity); }
void UMVVM_SettlerSlotDetails::SetTravelSpeed(float InTravelSpeed)              { UE_MVVM_SET_PROPERTY_VALUE(TravelSpeed, InTravelSpeed); }
void UMVVM_SettlerSlotDetails::SetCraftSpeed(float InCraftSpeed)                { UE_MVVM_SET_PROPERTY_VALUE(CraftSpeed, InCraftSpeed); }
void UMVVM_SettlerSlotDetails::SetGatherEfficiency(float InGatherEfficiency)    { UE_MVVM_SET_PROPERTY_VALUE(GatherEfficiency, InGatherEfficiency); }
void UMVVM_SettlerSlotDetails::SetInjuryResistance(float InInjuryResistance)	{ UE_MVVM_SET_PROPERTY_VALUE(InjuryResistance, InInjuryResistance); }