// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Core/LoadScreenSaveGame.h"
#include "MVVM_LoadSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitcherIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnableSelectSlotButton, bool, bEnable);

/**
 * 
 */
UCLASS()
class BONELAW_API UMVVM_LoadSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FSetWidgetSwitcherIndex SetWidgetSwitcherIndex;
	
	UPROPERTY(BlueprintAssignable)
	FEnableSelectSlotButton EnableSelectSlotButton;
	
	void InitializeSlot();
	
	void SetPlayerName(FString InPlayerName);
	void SetLoadSlotName(FString InLoadSlotName);
	
	FString GetPlayerName() const { return PlayerName; }
	FString GetLoadSlotName() const { return LoadSlotName; }
	
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SlotStatus;
	
private:
	
	UPROPERTY()
	FString SlotIndex;
	
	// -- Field Notifies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"))
	FString PlayerName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"))
	FString LoadSlotName;
};
