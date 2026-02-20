// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_SettlerSlotDetails.h"
#include "MVVM_SettlerScreen.generated.h"

class UMVVM_SettlerSlotDetails;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSettlerListRebuilt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSettlerSelected, FGuid, SettlerId);

/** GUID-based detail request (SlotIndex is legacy, GUID is primary). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSettlerRequestShowDetails, FGuid, SettlerGuid, class AStoneBaseChar*, SettlerActor);

class UStoneRosterSubsystem;
class UMVVM_SettlerSlot;
class AStoneBaseChar;

/**
 * Settler Screen ViewModel - DATA PROVIDER ONLY
 *
 * Blueprint creates SlotViewModels and registers them here for GUID-based lookups.
 * This class provides roster data and broadcasts events, but does NOT manage ViewModels.
 */
UCLASS(BlueprintType)
class BONELAW_API UMVVM_SettlerScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:
	// -------------------------
	// Data API for Blueprint
	// -------------------------

	/** Get current number of settlers in roster. */
	UFUNCTION(BlueprintPure, Category="Settlers")
	int32 GetNumSettlers() const { return NumSettlers; }
	
	void SetNumSettlers(int32 InNumSettlers);

	/** Get settler GUID by roster index. */
	UFUNCTION(BlueprintPure, Category="Settlers")
	FGuid GetSettlerGuidByIndex(int32 Index) const;

	/** Get settler display name by GUID. */
	UFUNCTION(BlueprintPure, Category="Settlers")
	FString GetSettlerNameByGuid(const FGuid& SettlerId) const;

	/** Get or spawn settler pawn by GUID. */
	UFUNCTION(BlueprintCallable, Category="Settlers")
	AStoneBaseChar* GetSettlerPawnByGuid(const FGuid& SettlerId);


	// -------------------------
	// ViewModel Registration (Blueprint-driven)
	// -------------------------

	/** Register a slot ViewModel for GUID-based lookups. Call from Blueprint after creating slot VM. */
	UFUNCTION(BlueprintCallable, Category="Settlers")
	void RegisterSlotViewModel(const FGuid& SettlerGuid, UMVVM_SettlerSlot* SlotVM);

	/** Unregister slot ViewModel. */
	UFUNCTION(BlueprintCallable, Category="Settlers")
	void UnregisterSlotViewModel(const FGuid& SettlerGuid);

	/** Get registered slot ViewModel by settler GUID (for cross-widget lookups). */
	UFUNCTION(BlueprintPure, Category="Settlers")
	UMVVM_SettlerSlot* GetSlotViewModelBySettlerGuid(const FGuid& SettlerId) const;

private:
	/** Internal slot-to-GUID map for fast lookups. Blueprint populates this via RegisterSlotViewModel(). */
	UPROPERTY()
	TMap<FGuid, TObjectPtr<UMVVM_SettlerSlot>> SlotViewModelsByGuid;

	/** Cached settler GUIDs from last roster refresh (for index-based lookups). */
	UPROPERTY()
	TArray<FGuid> CachedSettlerGuids;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"))
	int32 NumSettlers;
	// end Data API

	// -------------------------
	// Roster Binding & Events
	// -------------------------
public:
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"), Category="Stone|UI|Settlers")
	void BindToRoster(UObject* WorldContextObject);

	/** Refresh cached settler data from roster. Blueprint rebuilds grid after this. */
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"), Category="Stone|UI|Settlers")
	void RefreshDataFromRoster(UObject* WorldContextObject);

	/** Broadcast when roster changes (Blueprint should rebuild grid). */
	UPROPERTY(BlueprintAssignable, Category="Stone|UI|Settlers")
	FSettlerListRebuilt OnSettlerListRebuilt;

	/** Fired when details should be shown for a settler. */
	UPROPERTY(BlueprintAssignable, Category="Stone|UI|Settlers")
	FOnSettlerRequestShowDetails OnRequestShowDetails;

	/** Broadcast detail request by GUID (called from Blueprint slot widgets). */
	UFUNCTION(BlueprintCallable, Category="Settlers")
	void RequestShowDetails(const FGuid& SettlerGuid, AStoneBaseChar* SettlerActor);

private:
	UFUNCTION()
	void HandleRosterChanged();

	UPROPERTY()
	TWeakObjectPtr<UObject> CachedWorldContextObject;

	UPROPERTY()
	TWeakObjectPtr<UStoneRosterSubsystem> CachedRoster;
	// end Roster
};
