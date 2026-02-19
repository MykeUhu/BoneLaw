// Copyright by MykeUhu

#include "UI/ViewModel/MVVM_SettlerSlot.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "Core/Character/StoneBaseChar.h"
#include "GameplayEffectTypes.h"

void UMVVM_SettlerSlot::InitializeSlot()
{
	const int32 Index = (SlotStatus == ESettlerSlotStatus::Occupied) ? 1 : 0;
	OnSettlerSlotSwitcherIndex.Broadcast(Index);
}

// -------------------------
// Slot API
// -------------------------

void UMVVM_SettlerSlot::SetOccupied(const FString& InGuid, const FString& InName, AStoneBaseChar* InSettler)
{
	SlotStatus = ESettlerSlotStatus::Occupied;

	SetSettlerGUID(InGuid);
	SetSettlerName(InName);

	SettlerActorWeak = InSettler;
	BindToSettler(InSettler);

	InitializeSlot();
}

void UMVVM_SettlerSlot::ClearSlot()
{
	SlotStatus = ESettlerSlotStatus::Empty;

	UnbindFromSettler();

	SettlerActorWeak = nullptr;

	SetSettlerGUID(TEXT(""));
	SetSettlerName(TEXT(""));
	SetMoodWidgetIndex(2);

	InitializeSlot();
}

void UMVVM_SettlerSlot::RequestShowDetails()
{
	AStoneBaseChar* Actor = SettlerActorWeak.Get();
	if (SlotStatus == ESettlerSlotStatus::Occupied && Actor)
	{
		OnRequestShowDetails.Broadcast(SlotIndex, Actor);
	}
}

// -------------------------
// MVVM setters
// -------------------------

void UMVVM_SettlerSlot::SetSettlerGUID(const FString& InSettlerGUID)
{
	UE_MVVM_SET_PROPERTY_VALUE(SettlerGUID, InSettlerGUID);
}

void UMVVM_SettlerSlot::SetSettlerName(const FString& InSettlerName)
{
	UE_MVVM_SET_PROPERTY_VALUE(SettlerName, InSettlerName);
}

void UMVVM_SettlerSlot::SetSettlerSlotName(const FString& InSettlerSlotName)
{
	UE_MVVM_SET_PROPERTY_VALUE(SettlerSlotName, InSettlerSlotName);
}

void UMVVM_SettlerSlot::SetMoodWidgetIndex(int32 InIndex)
{
	UE_MVVM_SET_PROPERTY_VALUE(MoodWidgetIndex, InIndex);
}

// -------------------------
// Mood Index
// -------------------------

void UMVVM_SettlerSlot::UpdateMoodWidgetIndex()
{
	// 5 States: 0=":D", 1=":)", 2=":|", 3=":(", 4=":(("
	const float Max = CachedMaxMorale;
	const float Pct = (Max > 0.f) ? (CachedMorale / Max) : 0.f;

	int32 NewIndex = 2; // :|

	if (Pct >= 0.85f)      NewIndex = 0; // :D
	else if (Pct >= 0.65f) NewIndex = 1; // :)
	else if (Pct >= 0.35f) NewIndex = 2; // :|
	else if (Pct >= 0.15f) NewIndex = 3; // :(
	else                   NewIndex = 4; // :((

	SetMoodWidgetIndex(NewIndex);
}

// -------------------------
// GAS binding
// -------------------------

void UMVVM_SettlerSlot::BindToSettler(AStoneBaseChar* InSettler)
{
	UnbindFromSettler();

	BoundSettler = InSettler;
	if (!BoundSettler)
	{
		CachedMorale = 0.f;
		CachedMaxMorale = 0.f;
		SetMoodWidgetIndex(2);
		return;
	}

	// If ASC exists already, bind now, else wait for OnAscRegistered
	if (UAbilitySystemComponent* ASC = BoundSettler->GetAbilitySystemComponent())
	{
		HandleASCRegistered(ASC);
		return;
	}

	AscRegisteredHandle = BoundSettler->OnAscRegistered.AddUObject(this, &UMVVM_SettlerSlot::HandleASCRegistered);
}

void UMVVM_SettlerSlot::HandleASCRegistered(UAbilitySystemComponent* InASC)
{
	if (!InASC)
	{
		return;
	}

	// Stop waiting for ASC
	if (BoundSettler && AscRegisteredHandle.IsValid())
	{
		BoundSettler->OnAscRegistered.Remove(AscRegisteredHandle);
		AscRegisteredHandle.Reset();
	}

	BoundASC = InASC;

	// Initial cache + index
	if (const UStoneAttributeSet* AS = BoundASC->GetSet<UStoneAttributeSet>())
	{
		CachedMorale = AS->GetMorale();
		CachedMaxMorale = AS->GetMaxMorale();
	}
	else
	{
		CachedMorale = 0.f;
		CachedMaxMorale = 0.f;
	}

	UpdateMoodWidgetIndex();

	// Live updates
	const FGameplayAttribute MoraleAttr = UStoneAttributeSet::GetMoraleAttribute();
	const FGameplayAttribute MaxMoraleAttr = UStoneAttributeSet::GetMaxMoraleAttribute();

	MoraleChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(MoraleAttr)
		.AddUObject(this, &UMVVM_SettlerSlot::HandleMoraleChanged);

	MaxMoraleChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(MaxMoraleAttr)
		.AddUObject(this, &UMVVM_SettlerSlot::HandleMaxMoraleChanged);
}

void UMVVM_SettlerSlot::HandleMoraleChanged(const FOnAttributeChangeData& Data)
{
	CachedMorale = Data.NewValue;
	UpdateMoodWidgetIndex();
}

void UMVVM_SettlerSlot::HandleMaxMoraleChanged(const FOnAttributeChangeData& Data)
{
	CachedMaxMorale = Data.NewValue;
	UpdateMoodWidgetIndex();
}

void UMVVM_SettlerSlot::UnbindFromSettler()
{
	// Unbind morale delegates
	if (BoundASC)
	{
		const FGameplayAttribute MoraleAttr = UStoneAttributeSet::GetMoraleAttribute();
		const FGameplayAttribute MaxMoraleAttr = UStoneAttributeSet::GetMaxMoraleAttribute();

		if (MoraleChangedHandle.IsValid())
		{
			BoundASC->GetGameplayAttributeValueChangeDelegate(MoraleAttr).Remove(MoraleChangedHandle);
			MoraleChangedHandle.Reset();
		}

		if (MaxMoraleChangedHandle.IsValid())
		{
			BoundASC->GetGameplayAttributeValueChangeDelegate(MaxMoraleAttr).Remove(MaxMoraleChangedHandle);
			MaxMoraleChangedHandle.Reset();
		}
	}

	// Unbind ASC-registered delegate
	if (BoundSettler && AscRegisteredHandle.IsValid())
	{
		BoundSettler->OnAscRegistered.Remove(AscRegisteredHandle);
		AscRegisteredHandle.Reset();
	}

	BoundASC = nullptr;
	BoundSettler = nullptr;

	CachedMorale = 0.f;
	CachedMaxMorale = 0.f;
}

// -------------------------
// Lifetime
// -------------------------

void UMVVM_SettlerSlot::BeginDestroy()
{
	UnbindFromSettler();
	Super::BeginDestroy();
}
