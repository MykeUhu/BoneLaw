// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "StoneAttributeSet.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties(){}

	FGameplayEffectContextHandle EffectContextHandle;

	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	UPROPERTY()
	AController* SourceController = nullptr;

	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	UPROPERTY()
	AController* TargetController = nullptr;

	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};

// typedef is specific to the FGameplayAttribute() signature, but TStaticFunPtr is generic to any signature chosen
//typedef TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr FAttributeFuncPtr;
template<class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

/**
 * 
 */
UCLASS()
class BONELAW_API UStoneAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	
	UStoneAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;
	
	/*
	 * Primary Attributes
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Strength, Category="Primary Attributes")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, Strength);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Intelligence, Category="Primary Attributes")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, Intelligence);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Endurance, Category="Primary Attributes")
	FGameplayAttributeData Endurance;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, Endurance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Willpower, Category="Primary Attributes")
	FGameplayAttributeData Willpower;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, Willpower);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Social, Category="Primary Attributes")
	FGameplayAttributeData Social;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, Social);
	
	/*
	 * Secondary Attributes
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CarryCapacity, Category="Secondary Attributes")
	FGameplayAttributeData CarryCapacity;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, CarryCapacity);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_TravelSpeed, Category="Secondary Attributes")
	FGameplayAttributeData TravelSpeed;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, TravelSpeed);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CraftSpeed, Category="Secondary Attributes")
	FGameplayAttributeData CraftSpeed;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, CraftSpeed);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_GatherEfficiency, Category="Secondary Attributes")
	FGameplayAttributeData GatherEfficiency;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, GatherEfficiency);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_InjuryResistance, Category="Secondary Attributes")
	FGameplayAttributeData InjuryResistance;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, InjuryResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData HealthRegeneration;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, HealthRegeneration);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, MaxHealth);

	
	/*
	 * Vital Attributes
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Food, Category="Vital Attributes")
	FGameplayAttributeData Food;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, Food);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Water, Category="Vital Attributes")
	FGameplayAttributeData Water;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, Water);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Warmth, Category="Vital Attributes")
	FGameplayAttributeData Warmth;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, Warmth);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Morale, Category="Vital Attributes")
	FGameplayAttributeData Morale;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, Morale);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Trust, Category="Vital Attributes")
	FGameplayAttributeData Trust;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, Trust);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, Health);
	

	/*
	 * Culture Attributes (0..100, axis-like: 50 = neutral)
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CultureEmpathy, Category="Culture Attributes")
	FGameplayAttributeData CultureEmpathy;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, CultureEmpathy);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CultureHierarchy, Category="Culture Attributes")
	FGameplayAttributeData CultureHierarchy;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, CultureHierarchy);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CultureViolence, Category="Culture Attributes")
	FGameplayAttributeData CultureViolence;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, CultureViolence);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CultureSpirituality, Category="Culture Attributes")
	FGameplayAttributeData CultureSpirituality;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, CultureSpirituality);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CultureInnovation, Category="Culture Attributes")
	FGameplayAttributeData CultureInnovation;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, CultureInnovation);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CultureCollectivism, Category="Culture Attributes")
	FGameplayAttributeData CultureCollectivism;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, CultureCollectivism);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CultureXenophobia, Category="Culture Attributes")
	FGameplayAttributeData CultureXenophobia;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, CultureXenophobia);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CultureTabooStrictness, Category="Culture Attributes")
	FGameplayAttributeData CultureTabooStrictness;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, CultureTabooStrictness);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CultureDietBalance, Category="Culture Attributes")
	FGameplayAttributeData CultureDietBalance;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, CultureDietBalance);

	/*
	 * Knowledge Attributes (0..100)
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_KnowledgeMedicine, Category="Knowledge Attributes")
	FGameplayAttributeData KnowledgeMedicine;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, KnowledgeMedicine);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_KnowledgeHunting, Category="Knowledge Attributes")
	FGameplayAttributeData KnowledgeHunting;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, KnowledgeHunting);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_KnowledgeSurvival, Category="Knowledge Attributes")
	FGameplayAttributeData KnowledgeSurvival;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, KnowledgeSurvival);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_KnowledgeCraft, Category="Knowledge Attributes")
	FGameplayAttributeData KnowledgeCraft;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, KnowledgeCraft);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_KnowledgeSocial, Category="Knowledge Attributes")
	FGameplayAttributeData KnowledgeSocial;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, KnowledgeSocial);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_KnowledgeCourage, Category="Knowledge Attributes")
	FGameplayAttributeData KnowledgeCourage;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, KnowledgeCourage);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_KnowledgeSpiritual, Category="Knowledge Attributes")
	FGameplayAttributeData KnowledgeSpiritual;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, KnowledgeSpiritual);

	/*
	 * Worldline Axis Attributes (0..100, 50 = neutral)
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_WorldlineMercyRuthless, Category="Worldline Attributes")
	FGameplayAttributeData WorldlineMercyRuthless;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, WorldlineMercyRuthless);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_WorldlineTraditionInnovation, Category="Worldline Attributes")
	FGameplayAttributeData WorldlineTraditionInnovation;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, WorldlineTraditionInnovation);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_WorldlineCollectiveIndividual, Category="Worldline Attributes")
	FGameplayAttributeData WorldlineCollectiveIndividual;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, WorldlineCollectiveIndividual);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_WorldlineSpiritualPractical, Category="Worldline Attributes")
	FGameplayAttributeData WorldlineSpiritualPractical;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, WorldlineSpiritualPractical);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_WorldlineXenoOpenFear, Category="Worldline Attributes")
	FGameplayAttributeData WorldlineXenoOpenFear;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, WorldlineXenoOpenFear);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_WorldlineTabooLooseStrict, Category="Worldline Attributes")
	FGameplayAttributeData WorldlineTabooLooseStrict;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, WorldlineTabooLooseStrict);


	/*
	 * Meta Attributes
	 */
	UPROPERTY(BlueprintReadOnly, Category="Meta Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, IncomingDamage);

	UPROPERTY(BlueprintReadOnly, Category="Meta Attributes")
	FGameplayAttributeData IncomingHeal;
	ATTRIBUTE_ACCESSORS(UStoneAttributeSet, IncomingHeal);
	
	/*
	 * OnRep (must match attribute order above)
	 */
	// Primary
	UFUNCTION() void OnRep_Strength(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_Intelligence(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_Endurance(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_Willpower(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_Social(const FGameplayAttributeData& OldValue) const;

	// Secondary
	UFUNCTION() void OnRep_CarryCapacity(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_TravelSpeed(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_CraftSpeed(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_GatherEfficiency(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_InjuryResistance(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_HealthRegeneration(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const;

	// Vital
	UFUNCTION() void OnRep_Food(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_Water(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_Warmth(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_Morale(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_Trust(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_Health(const FGameplayAttributeData& OldValue) const;



	// Culture
	UFUNCTION() void OnRep_CultureEmpathy(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_CultureHierarchy(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_CultureViolence(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_CultureSpirituality(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_CultureInnovation(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_CultureCollectivism(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_CultureXenophobia(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_CultureTabooStrictness(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_CultureDietBalance(const FGameplayAttributeData& OldValue) const;

	// Knowledge
	UFUNCTION() void OnRep_KnowledgeMedicine(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_KnowledgeHunting(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_KnowledgeSurvival(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_KnowledgeCraft(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_KnowledgeSocial(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_KnowledgeCourage(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_KnowledgeSpiritual(const FGameplayAttributeData& OldValue) const;

	// Worldline Axis
	UFUNCTION() void OnRep_WorldlineMercyRuthless(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_WorldlineTraditionInnovation(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_WorldlineCollectiveIndividual(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_WorldlineSpiritualPractical(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_WorldlineXenoOpenFear(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_WorldlineTabooLooseStrict(const FGameplayAttributeData& OldValue) const;
	
private:
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const;
	bool bTopOffHealth = false;

	
};
