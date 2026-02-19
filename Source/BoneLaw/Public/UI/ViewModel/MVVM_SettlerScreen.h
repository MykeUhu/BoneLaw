// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_SettlerScreen.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSettlerListRebuilt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSettlerSelected, FGuid, SettlerId);


class UStoneRosterSubsystem;
class UMVVM_SettlerSlot;

/**
 * Settler Screen ViewModel - FIXED 20 SLOTS (LoadScreen pattern 1:1)
 * 
 * Usage (exactly like LoadScreen):
 * 1. HUD creates ViewModel
 * 2. Call InitializeSettlerSlots() ONCE (creates 20 fixed slots)
 * 3. Call RefreshSlotsFromRoster() to populate slots from Roster
 * 4. Blueprint binds to SettlerSlot_0..19 and calls InitializeSlot() on each
 */
UCLASS()
class BONELAW_API UMVVM_SettlerScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:
	// MVVM 
	void InitializeSettlerSlots();
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_SettlerSlot> SettlerSlotViewModelClass;
	
	UFUNCTION(BlueprintPure)
	UMVVM_SettlerSlot* GetSettlerSlotViewModelByIndex(int32 Index) const;
	
	void SetNumSettlerSlots(int32 InNumSettlerSlots);
	
	int32 GetNumSettlerSlots() const { return NumSettlerSlots; }
	
private:
	UPROPERTY()
	TMap<int32, UMVVM_SettlerSlot*> SettlerSlots;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_0;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_1;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_2;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_3;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_4;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_5;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_6;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_7;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_8;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_9;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_10;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_11;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_12;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_13;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_14;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_15;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_16;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_17;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_18;
	
	UPROPERTY()
	UMVVM_SettlerSlot* SettlerSlot_19;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"))
	int32 NumSettlerSlots;
	// end MVVM
	
	// Roster
public:
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"), Category="Stone|UI|Settlers")
	void BindToRoster(UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"), Category="Stone|UI|Settlers")
	void RefreshSlotsFromRoster(UObject* WorldContextObject);
	
	UPROPERTY(BlueprintAssignable, Category="Stone|UI|Settlers")
	FSettlerListRebuilt OnSettlerListRebuilt;
	
private:
	UFUNCTION()
	void HandleRosterChanged();

	UPROPERTY()
	TWeakObjectPtr<UObject> CachedWorldContextObject;

	UPROPERTY()
	TWeakObjectPtr<UStoneRosterSubsystem> CachedRoster;
	
	// end Roster
};
