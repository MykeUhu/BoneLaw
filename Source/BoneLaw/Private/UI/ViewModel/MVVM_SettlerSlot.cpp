// Copyright by MykeUhu

#include "UI/ViewModel/MVVM_SettlerSlot.h"

void UMVVM_SettlerSlot::InitializeSlot()
{
	const int32 Index = (SlotStatus == ESettlerSlotStatus::Occupied) ? 1 : 0;
	OnSettlerSlotSwitcherIndex.Broadcast(Index);
}

void UMVVM_SettlerSlot::SetSettlerGUID(FString InSettlerGUID)
{
	UE_MVVM_SET_PROPERTY_VALUE(SettlerGUID, InSettlerGUID);
}

void UMVVM_SettlerSlot::SetSettlerName(FString InSettlerName)
{
	UE_MVVM_SET_PROPERTY_VALUE(SettlerName, InSettlerName);
}

void UMVVM_SettlerSlot::SetSettlerSlotName(FString InSettlerSlotName)
{
	UE_MVVM_SET_PROPERTY_VALUE(SettlerSlotName, InSettlerSlotName);
}

void UMVVM_SettlerSlot::SetOccupied(FString InGuid, FString InName)
{
	SlotStatus = ESettlerSlotStatus::Occupied;
	SetSettlerGUID(InGuid);
	SetSettlerName(InName);
	InitializeSlot();
}

void UMVVM_SettlerSlot::ClearSlot()
{
	SlotStatus = ESettlerSlotStatus::Empty;
	SetSettlerGUID("");
	SetSettlerName("");
	InitializeSlot();
}



