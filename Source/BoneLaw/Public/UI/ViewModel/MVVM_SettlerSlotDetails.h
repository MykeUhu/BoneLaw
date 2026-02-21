// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_SettlerSlotDetails.generated.h"

class AStoneSettlerChar;
class AStoneBaseChar;
class UAbilitySystemComponent;
struct FOnAttributeChangeData;

UCLASS()
class BONELAW_API UMVVM_SettlerSlotDetails : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	// -------------------------
	// Details lifecycle
	// -------------------------
	UFUNCTION(BlueprintCallable)
	void BindToSettler(const FGuid& SettlerId, AStoneSettlerChar* SettlerActor);

	UFUNCTION(BlueprintCallable)
	void Unbind();

protected:
	virtual void BeginDestroy() override;

private:
	// -------------------------
	// GAS binding
	// -------------------------
	void HandleASCRegistered(UAbilitySystemComponent* InASC);

	void BindAttributeDelegates();
	void UnbindAttributeDelegates();

	// Handlers
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleFoodChanged(const FOnAttributeChangeData& Data);
	void HandleWaterChanged(const FOnAttributeChangeData& Data);
	void HandleWarmthChanged(const FOnAttributeChangeData& Data);
	void HandleMoraleChanged(const FOnAttributeChangeData& Data);

	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxFoodChanged(const FOnAttributeChangeData& Data);
	void HandleMaxWaterChanged(const FOnAttributeChangeData& Data);
	void HandleMaxMoraleChanged(const FOnAttributeChangeData& Data);

	// Helpers
	void RecomputePct();

	// -------------------------
	// MVVM Setters
	// -------------------------
	void SetHealth(float InValue);
	void SetMaxHealth(float InValue);
	void SetHealthPct(float InValue);

	void SetFood(float InValue);
	void SetMaxFood(float InValue);
	void SetFoodPct(float InValue);

	void SetWater(float InValue);
	void SetMaxWater(float InValue);
	void SetWaterPct(float InValue);

	void SetWarmth(float InValue);
	void SetWarmthPct(float InValue);

	void SetMorale(float InValue);
	void SetMaxMorale(float InValue);
	void SetMoralePct(float InValue);

	// -------------------------
	// MVVM Getters
	// -------------------------
	float GetHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }
	float GetHealthPct() const { return HealthPct; }

	float GetFood() const { return Food; }
	float GetMaxFood() const { return MaxFood; }
	float GetFoodPct() const { return FoodPct; }

	float GetWater() const { return Water; }
	float GetMaxWater() const { return MaxWater; }
	float GetWaterPct() const { return WaterPct; }

	float GetWarmth() const { return Warmth; }
	float GetWarmthPct() const { return WarmthPct; }

	float GetMorale() const { return Morale; }
	float GetMaxMorale() const { return MaxMorale; }
	float GetMoralePct() const { return MoralePct; }

private:
	// -------------------------
	// Bound identity
	// -------------------------
	FGuid BoundSettlerId;

	// -------------------------
	// GAS State
	// -------------------------
	UPROPERTY(Transient)
	TObjectPtr<AStoneBaseChar> BoundSettler;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> BoundASC;

	FDelegateHandle AscRegisteredHandle;

	FDelegateHandle HealthChangedHandle;
	FDelegateHandle FoodChangedHandle;
	FDelegateHandle WaterChangedHandle;
	FDelegateHandle WarmthChangedHandle;
	FDelegateHandle MoraleChangedHandle;

	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle MaxFoodChangedHandle;
	FDelegateHandle MaxWaterChangedHandle;
	FDelegateHandle MaxMoraleChangedHandle;

private:
	// -------------------------
	// MVVM Fields
	// -------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Health = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float MaxHealth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float HealthPct = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Food = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float MaxFood = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float FoodPct = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Water = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float MaxWater = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float WaterPct = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Warmth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float WarmthPct = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Morale = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float MaxMorale = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float MoralePct = 0.f;
};