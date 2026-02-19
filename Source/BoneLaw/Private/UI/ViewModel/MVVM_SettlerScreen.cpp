// Copyright by MykeUhu

#include "UI/ViewModel/MVVM_SettlerScreen.h"
#include "UI/ViewModel/MVVM_SettlerSlot.h"
#include "Runtime/StoneRosterSubsystem.h"
#include "Engine/World.h"

static UWorld* ResolveWorldFromContext(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	// Try GetWorld() first
	if (UWorld* W = WorldContextObject->GetWorld())
	{
		return W;
	}

	// Fallback: Engine context resolve (safe in game)
	if (GEngine)
	{
		return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	}

	return nullptr;
}

// MVVM
void UMVVM_SettlerScreen::InitializeSettlerSlots()
{
	// EXACT COPY of UMVVM_LoadScreen::InitializeLoadSlots() pattern
	// Create 20 fixed slots (like LoadSlot_0, LoadSlot_1, ...)
	
	SettlerSlot_0 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_0->SetSettlerSlotName(FString ("SettlerSlot_0"));
	SettlerSlot_0->SlotIndex = 0;
	SettlerSlots.Add(0, SettlerSlot_0);
	
	SettlerSlot_1 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_1->SlotIndex = 1;
	SettlerSlots.Add(1, SettlerSlot_1);
	
	SettlerSlot_2 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_2->SlotIndex = 2;
	SettlerSlots.Add(2, SettlerSlot_2);
	
	SettlerSlot_3 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_3->SlotIndex = 3;
	SettlerSlots.Add(3, SettlerSlot_3);
	
	SettlerSlot_4 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_4->SlotIndex = 4;
	SettlerSlots.Add(4, SettlerSlot_4);
	
	SettlerSlot_5 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_5->SlotIndex = 5;
	SettlerSlots.Add(5, SettlerSlot_5);
	
	SettlerSlot_6 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_6->SlotIndex = 6;
	SettlerSlots.Add(6, SettlerSlot_6);
	
	SettlerSlot_7 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_7->SlotIndex = 7;
	SettlerSlots.Add(7, SettlerSlot_7);
	
	SettlerSlot_8 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_8->SlotIndex = 8;
	SettlerSlots.Add(8, SettlerSlot_8);
	
	SettlerSlot_9 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_9->SlotIndex = 9;
	SettlerSlots.Add(9, SettlerSlot_9);
	
	SettlerSlot_10 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_10->SlotIndex = 10;
	SettlerSlots.Add(10, SettlerSlot_10);
	
	SettlerSlot_11 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_11->SlotIndex = 11;
	SettlerSlots.Add(11, SettlerSlot_11);
	
	SettlerSlot_12 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_12->SlotIndex = 12;
	SettlerSlots.Add(12, SettlerSlot_12);
	
	SettlerSlot_13 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_13->SlotIndex = 13;
	SettlerSlots.Add(13, SettlerSlot_13);
	
	SettlerSlot_14 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_14->SlotIndex = 14;
	SettlerSlots.Add(14, SettlerSlot_14);
	
	SettlerSlot_15 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_15->SlotIndex = 15;
	SettlerSlots.Add(15, SettlerSlot_15);
	
	SettlerSlot_16 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_16->SlotIndex = 16;
	SettlerSlots.Add(16, SettlerSlot_16);
	
	SettlerSlot_17 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_17->SlotIndex = 17;
	SettlerSlots.Add(17, SettlerSlot_17);
	
	SettlerSlot_18 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_18->SlotIndex = 18;
	SettlerSlots.Add(18, SettlerSlot_18);
	
	SettlerSlot_19 = NewObject<UMVVM_SettlerSlot>(this, SettlerSlotViewModelClass);
	SettlerSlot_19->SlotIndex = 19;
	SettlerSlots.Add(19, SettlerSlot_19);
	
	SetNumSettlerSlots(SettlerSlots.Num());
	
	UE_LOG(LogTemp, Log, TEXT("[SettlerScreen] Initialized 20 fixed settler slots"));
}

UMVVM_SettlerSlot* UMVVM_SettlerScreen::GetSettlerSlotViewModelByIndex(int32 Index) const
{
	if (UMVVM_SettlerSlot* const* Found = SettlerSlots.Find(Index))
	{
		return *Found;
	}

	return nullptr;
}

void UMVVM_SettlerScreen::SetNumSettlerSlots(int32 InNumSettlerSlots)
{
	UE_MVVM_SET_PROPERTY_VALUE(NumSettlerSlots, InNumSettlerSlots);
}



// end MVVM

// Roster
void UMVVM_SettlerScreen::BindToRoster(UObject* WorldContextObject)
{
	CachedWorldContextObject = WorldContextObject;

	UWorld* World = ResolveWorldFromContext(WorldContextObject);
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerScreen] BindToRoster failed: invalid WorldContext."));
		return;
	}

	UStoneRosterSubsystem* Roster = World->GetSubsystem<UStoneRosterSubsystem>();
	if (!Roster)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerScreen] BindToRoster failed: StoneRosterSubsystem missing."));
		return;
	}

	// Unbind old if any
	if (CachedRoster.IsValid())
	{
		CachedRoster->OnRosterChanged.RemoveDynamic(this, &UMVVM_SettlerScreen::HandleRosterChanged);
	}

	CachedRoster = Roster;
	Roster->OnRosterChanged.AddDynamic(this, &UMVVM_SettlerScreen::HandleRosterChanged);

	// Initial fill
	RefreshSlotsFromRoster(WorldContextObject);

	UE_LOG(LogTemp, Log, TEXT("[SettlerScreen] Bound to roster changes + initial refresh done."));
}

void UMVVM_SettlerScreen::RefreshSlotsFromRoster(UObject* WorldContextObject)
{
	UWorld* World = ResolveWorldFromContext(WorldContextObject);
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerScreen] RefreshSlotsFromRoster failed: invalid WorldContext."));
		return;
	}

	UStoneRosterSubsystem* Roster = World->GetSubsystem<UStoneRosterSubsystem>();
	if (!Roster)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerScreen] RefreshSlotsFromRoster failed: StoneRosterSubsystem missing."));
		return;
	}

	// 1) Clear all slots
	for (auto& KV : SettlerSlots)
	{
		if (UMVVM_SettlerSlot* SlotVM = KV.Value)
		{
			SlotVM->ClearSlot();
		}
	}

	// 2) Fill from roster (all settlers; wenn ihr nur "available" wollt: GetAvailableSettlerIds())
	const TArray<FGuid> SettlerIds = Roster->GetAllSettlerIds();

	const int32 MaxSlots = SettlerSlots.Num();
	const int32 FillCount = FMath::Min(MaxSlots, SettlerIds.Num());

	for (int32 i = 0; i < FillCount; ++i)
	{
		const FGuid& Id = SettlerIds[i];
		const FSavedSettler Info = Roster->GetSettlerInfo(Id);

		if (UMVVM_SettlerSlot* SlotVM = GetSettlerSlotViewModelByIndex(i))
		{
			SlotVM->SetOccupied(Id.ToString(EGuidFormats::DigitsWithHyphens), Info.DisplayName);
		}
	}

	OnSettlerListRebuilt.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[SettlerScreen] Refreshed slots from roster. Settlers=%d Slots=%d"),
		SettlerIds.Num(), SettlerSlots.Num());
}

void UMVVM_SettlerScreen::HandleRosterChanged()
{
	UObject* Ctx = CachedWorldContextObject.Get();
	RefreshSlotsFromRoster(Ctx);
}
// end Roster
