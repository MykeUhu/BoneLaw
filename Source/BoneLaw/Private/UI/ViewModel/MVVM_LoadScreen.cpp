// Copyright by MykeUhu


#include "UI/ViewModel/MVVM_LoadScreen.h"

#include "Core/GameMode/StoneLoadScreenGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadScreen::InitializeLoadSlots()
{
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_0->SetLoadSlotName(FString ("LoadSlot_0"));
	LoadSlot_0->SlotIndex = 0;
	LoadSlots.Add(0, LoadSlot_0);
	
	
	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_1->SetLoadSlotName(FString ("LoadSlot_1"));
	LoadSlot_1->SlotIndex = 1;
	LoadSlots.Add(1, LoadSlot_1);
	
	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_2->SetLoadSlotName(FString ("LoadSlot_2"));
	LoadSlot_2->SlotIndex = 2;
	LoadSlots.Add(2, LoadSlot_2);
	
	SetNumLoadSlots(LoadSlots.Num());
}

UMVVM_LoadSlot* UMVVM_LoadScreen::GetLoadSlotViewModelByIndex(int32 Index) const
{
	return LoadSlots.FindChecked(Index);
}

void UMVVM_LoadScreen::NewSlotButtonPressed(int32 Slot, FString EnteredName, FString MapName)
{
	AStoneLoadScreenGameMode* StoneLoadScreenGameMode = Cast<AStoneLoadScreenGameMode>(UGameplayStatics::GetGameMode(this));

	LoadSlots[Slot]->SetPlayerName(EnteredName);
	LoadSlots[Slot]->SetMapName(MapName);
	LoadSlots[Slot]->SlotStatus = Taken;

	StoneLoadScreenGameMode->SaveSlotData(LoadSlots[Slot], Slot);
	LoadSlots[Slot]->InitializeSlot();
}

void UMVVM_LoadScreen::NewGameButtonPressed(int32 Slot)
{
	LoadSlots[Slot]->SetWidgetSwitcherIndex.Broadcast(1);
}

void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 Slot)
{
	SlotSelected.Broadcast();
	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		if (LoadSlot.Key == Slot)
		{
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(false);
		}
		else
		{
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(true);
		}
	}
	SelectedSlot = LoadSlots[Slot];
}

void UMVVM_LoadScreen::DeleteButtonPressed()
{
	UE_LOG(LogTemp, Log, TEXT("[MVVM_LoadScreen] DeleteButtonPressed. SelectedSlot=%s"),
		IsValid(SelectedSlot) ? *SelectedSlot->GetLoadSlotName() : TEXT("NULL"));

	if (IsValid(SelectedSlot))
	{
		UE_LOG(LogTemp, Log, TEXT("[MVVM_LoadScreen] Deleting SlotName='%s' SlotIndex=%d"),
			*SelectedSlot->GetLoadSlotName(), SelectedSlot->SlotIndex);

		AStoneLoadScreenGameMode::DeleteSlot(SelectedSlot->GetLoadSlotName(), SelectedSlot->SlotIndex);

		SelectedSlot->SlotStatus = Vacant;
		SelectedSlot->InitializeSlot();
		SelectedSlot->EnableSelectSlotButton.Broadcast(true);
	}
}

void UMVVM_LoadScreen::PlayButtonPressed()
{
	AStoneLoadScreenGameMode* StoneLoadScreenGameMode = Cast<AStoneLoadScreenGameMode>(UGameplayStatics::GetGameMode(this));
	if (IsValid(SelectedSlot))
	{
		StoneLoadScreenGameMode->TravelToMap(SelectedSlot);
	}
}

void UMVVM_LoadScreen::LoadData()
{
	AStoneLoadScreenGameMode* StoneLoadScreenGameMode = Cast<AStoneLoadScreenGameMode>(UGameplayStatics::GetGameMode(this));
	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		ULoadScreenSaveGame* SaveObject = StoneLoadScreenGameMode->GetSaveSlotData(LoadSlot.Value->GetLoadSlotName(), LoadSlot.Key);

		const FString PlayerName = SaveObject->PlayerName;
		TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = SaveObject->SaveSlotStatus;
		
		LoadSlot.Value->SlotStatus = SaveSlotStatus;
		LoadSlot.Value->SetPlayerName(PlayerName);
		LoadSlot.Value->SetMapName(SaveObject->MapName);
		
		LoadSlot.Value->InitializeSlot();
	}
}

void UMVVM_LoadScreen::SetNumLoadSlots(int32 InNumLoadSlots)
{
	UE_MVVM_SET_PROPERTY_VALUE(NumLoadSlots, InNumLoadSlots);
}
