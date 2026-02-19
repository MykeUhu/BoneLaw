// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_SettlerSlot.generated.h"

class AStoneBaseChar;
class UAbilitySystemComponent;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettlerSlotSwitcherIndex, int32, WidgetSwitcherIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSettlerSlotRequestShowDetails, int32, SlotIndex, AStoneBaseChar*, SettlerActor);

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
	// -------------------------
	// Slot lifecycle
	// -------------------------
	void InitializeSlot();

	void SetOccupied(const FString& InGuid, const FString& InName, AStoneBaseChar* InSettler);
	void ClearSlot();
	
	// -------------------------
	// UI Events
	// -------------------------
	UPROPERTY(BlueprintAssignable)
	FOnSettlerSlotRequestShowDetails OnRequestShowDetails;

	// Called by the slot widget button click
	UFUNCTION(BlueprintCallable)
	void RequestShowDetails();
	
	// -------------------------
	// MVVM Setters
	// -------------------------
	void SetSettlerGUID(const FString& InSettlerGUID);
	void SetSettlerName(const FString& InSettlerName);
	void SetSettlerSlotName(const FString& InSettlerSlotName);
	void SetMoodWidgetIndex(int32 InIndex);

	// -------------------------
	// MVVM Getters
	// -------------------------
	FString GetSettlerGUID() const { return SettlerGUID; }
	FString GetSettlerName() const { return SettlerName; }
	FString GetSettlerSlotName() const { return SettlerSlotName; }
	int32 GetMoodWidgetIndex() const { return MoodWidgetIndex; }

	// -------------------------
	// Public State
	// -------------------------
	UPROPERTY()
	ESettlerSlotStatus SlotStatus = ESettlerSlotStatus::Empty;

	UPROPERTY(BlueprintAssignable)
	FOnSettlerSlotSwitcherIndex OnSettlerSlotSwitcherIndex;

	UPROPERTY()
	int32 SlotIndex = 0;

protected:
	virtual void BeginDestroy() override;

private:
	// -------------------------
	// GAS binding
	// -------------------------
	void BindToSettler(AStoneBaseChar* InSettler);
	void UnbindFromSettler();

	void HandleASCRegistered(UAbilitySystemComponent* InASC);

	void HandleMoraleChanged(const FOnAttributeChangeData& Data);
	void HandleMaxMoraleChanged(const FOnAttributeChangeData& Data);

	void UpdateMoodWidgetIndex();

	UPROPERTY(Transient)
	TWeakObjectPtr<AStoneBaseChar> SettlerActorWeak;

	UPROPERTY(Transient)
	TObjectPtr<AStoneBaseChar> BoundSettler;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> BoundASC;

	FDelegateHandle AscRegisteredHandle;
	FDelegateHandle MoraleChangedHandle;
	FDelegateHandle MaxMoraleChangedHandle;

	float CachedMorale = 0.f;
	float CachedMaxMorale = 0.f;

	// -------------------------
	// MVVM Fields
	// -------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	FString SettlerGUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	FString SettlerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	FString SettlerSlotName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	int32 MoodWidgetIndex = 2;
};
