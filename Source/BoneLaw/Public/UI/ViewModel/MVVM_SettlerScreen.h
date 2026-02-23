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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettlerRequestShowDetails, FGuid, SettlerGuid);

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

private:
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
	void RequestShowDetails(const FGuid& SettlerGuid);

private:
	UFUNCTION()
	void HandleRosterChanged();

	UPROPERTY()
	TWeakObjectPtr<UObject> CachedWorldContextObject;
	
	UPROPERTY()
	TWeakObjectPtr<UStoneRosterSubsystem> CachedRoster;
	// end Roster
	
	// -------------------------
	// Selected Settler (SSOT = GUID)
	// -------------------------
public:
	/** Blueprint API: set selected settler */
	UFUNCTION(BlueprintCallable, Category="Settlers")
	void SetSelectedSettlerGuid(const FGuid& InGuid);

	/** Blueprint API: get selected settler */
	UFUNCTION(BlueprintPure, Category="Settlers")
	FGuid GetSelectedSettlerGuid_BP() const { return SelectedSettlerGuid; }

private:
	// MVVM Field
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Getter, Setter, meta=(AllowPrivateAccess="true"))
	FGuid SelectedSettlerGuid;

	// MVVM Getter/Setter required by UPROPERTY(Getter/Setter)
	FGuid GetSelectedSettlerGuid() const { return SelectedSettlerGuid; }
	void SetSelectedSettlerGuid_Internal(const FGuid& InGuid);
};
