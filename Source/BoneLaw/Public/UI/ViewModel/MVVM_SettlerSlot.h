// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_SettlerSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettlerSlotSwitcherIndex, int32, WidgetSwitcherIndex);

UENUM(BlueprintType)
enum class ESettlerSlotStatus : uint8
{
	Empty,
	Occupied
};

UCLASS()
class BONELAW_API UMVVM_SettlerSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
	ESettlerSlotStatus SlotStatus = ESettlerSlotStatus::Empty;
	
	UPROPERTY(BlueprintAssignable)
	FOnSettlerSlotSwitcherIndex OnSettlerSlotSwitcherIndex;

	void InitializeSlot();
	
	// Setter
	void SetSettlerGUID(FString InSettlerGUID);
	void SetSettlerName(FString InSettlerName);
	void SetSettlerSlotName(FString InSettlerSlotName);
		
	// Getter
	FString GetSettlerGUID() const { return SettlerGUID; }
	FString GetSettlerName() const { return SettlerName; }
	FString GetSettlerSlotName() const { return SettlerSlotName; }
	
	// Helper
	void SetOccupied(FString InGuid, FString InName);
	void ClearSlot();

	UPROPERTY()
	int32 SlotIndex = 0;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, meta=(AllowPrivateAccess="true"))
	FString SettlerGUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, meta=(AllowPrivateAccess="true"))
	FString SettlerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, meta=(AllowPrivateAccess="true"))
	FString SettlerSlotName;
};
