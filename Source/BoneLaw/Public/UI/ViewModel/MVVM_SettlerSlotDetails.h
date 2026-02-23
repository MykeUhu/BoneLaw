// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MVVMViewModelBase.h"
#include "MVVM_SettlerSlotDetails.generated.h"

class AStoneSettlerChar;
class AStoneBaseChar;
class UAbilitySystemComponent;
class UAttributeInfo;
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
	
	// Primary Attributes
	void HandleStrengthChanged(const FOnAttributeChangeData& Data);
	void HandleIntelligenceChanged (const FOnAttributeChangeData& Data);
	void HandleEnduranceChanged(const FOnAttributeChangeData& Data);
	void HandleWillpowerChanged(const FOnAttributeChangeData& Data);
	void HandleSocialChanged(const FOnAttributeChangeData& Data);
	
	// Secondary Attributes
	void HandleCarryCapacityChanged(const FOnAttributeChangeData& Data);
	void HandleTravelSpeedChanged(const FOnAttributeChangeData& Data);
	void HandleCraftSpeedChanged(const FOnAttributeChangeData& Data);
	void HandleGatherEfficiencyChanged(const FOnAttributeChangeData& Data);
	void HandleInjuryResistanceChanged(const FOnAttributeChangeData& Data);

	// Helpers
	void RecomputePct();
	void InitializeAttributeIcons();
	FSlateBrush MakeBrushFromAttributeTag(const FGameplayTag& AttributeTag) const;

	// -------------------------
	// MVVM Setters
	// -------------------------
	void SetHealth(float InValue);
	void SetMaxHealth(float InValue);
	void SetHealthPct(float InValue);
	void SetHealthIcon(const FSlateBrush& InIcon);

	void SetFood(float InValue);
	void SetMaxFood(float InValue);
	void SetFoodPct(float InValue);
	void SetFoodIcon(const FSlateBrush& InIcon);

	void SetWater(float InValue);
	void SetMaxWater(float InValue);
	void SetWaterPct(float InValue);
	void SetWaterIcon(const FSlateBrush& InIcon);

	void SetWarmth(float InValue);
	void SetWarmthPct(float InValue);
	void SetWarmthIcon(const FSlateBrush& InIcon);

	void SetMorale(float InValue);
	void SetMaxMorale(float InValue);
	void SetMoralePct(float InValue);
	void SetMoraleIcon(const FSlateBrush& InIcon);
	
	// Primary Attributes
	void SetStrength(float InStrength);
	void SetIntelligence(float InIntelligence);
	void SetEndurance(float InEndurance);
	void SetWillpower(float InWillpower);
	void SetSocial(float InSocial);
	
	// Secondary Attributes
	void SetCarryCapacity(float InCarryCapacity);
	void SetTravelSpeed(float InTravelSpeed);
	void SetCraftSpeed(float InCraftSpeed);
	void SetGatherEfficiency(float InGatherEfficiency);
	void SetInjuryResistance(float InInjuryResistance);

	// -------------------------
	// MVVM Getters
	// -------------------------
	float GetHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }
	float GetHealthPct() const { return HealthPct; }
	FSlateBrush GetHealthIcon() const { return HealthIcon; }

	float GetFood() const { return Food; }
	float GetMaxFood() const { return MaxFood; }
	float GetFoodPct() const { return FoodPct; }
	FSlateBrush GetFoodIcon() const { return FoodIcon; }

	float GetWater() const { return Water; }
	float GetMaxWater() const { return MaxWater; }
	float GetWaterPct() const { return WaterPct; }
	FSlateBrush GetWaterIcon() const { return WaterIcon; }

	float GetWarmth() const { return Warmth; }
	float GetWarmthPct() const { return WarmthPct; }
	FSlateBrush GetWarmthIcon() const { return WarmthIcon; }

	float GetMorale() const { return Morale; }
	float GetMaxMorale() const { return MaxMorale; }
	float GetMoralePct() const { return MoralePct; }
	FSlateBrush GetMoraleIcon() const { return MoraleIcon; }

    // Primary Attributes
	float GetStrength() const { return Strength; }
	float GetIntelligence() const { return Intelligence; }
	float GetEndurance() const { return Endurance; }
	float GetWillpower() const { return Willpower; }
	float GetSocial() const { return Social; }
	
	// Secondary Attributes
	float GetCarryCapacity() const { return CarryCapacity; }
	float GetTravelSpeed() const { return TravelSpeed; }
	float GetCraftSpeed() const { return CraftSpeed; }
	float GetGatherEfficiency() const { return GatherEfficiency; }
	float GetInjuryResistance() const { return InjuryResistance; }

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
	
	// Primary Attributes
	FDelegateHandle StrengthChangedHandle;
	FDelegateHandle IntelligenceChangedHandle;
	FDelegateHandle EnduranceChangedHandle;
	FDelegateHandle WillpowerChangedHandle;
	FDelegateHandle SocialChangedHandle;
	
	// Secondary Attributes
	FDelegateHandle CarryCapacityChangedHandle;
	FDelegateHandle TravelSpeedChangedHandle;
	FDelegateHandle CraftSpeedChangedHandle;
	FDelegateHandle GatherEfficiencyChangedHandle;
	FDelegateHandle InjuryResistanceChangedHandle;

	// -------------------------
	// AttributeInfo DataAsset
	// -------------------------
	UPROPERTY(EditDefaultsOnly, Category="Attribute Info")
	TObjectPtr<UAttributeInfo> AttributeInfo;

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
	FSlateBrush HealthIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Food = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float MaxFood = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float FoodPct = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	FSlateBrush FoodIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Water = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float MaxWater = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float WaterPct = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	FSlateBrush WaterIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Warmth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float WarmthPct = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	FSlateBrush WarmthIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Morale = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float MaxMorale = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float MoralePct = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	FSlateBrush MoraleIcon;
	
	// Primary Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Strength;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Intelligence;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Endurance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Willpower;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float Social;
	
	// Secondary Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float CarryCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float TravelSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float CraftSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float GatherEfficiency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, FieldNotify, meta=(AllowPrivateAccess="true"))
	float InjuryResistance;
};
