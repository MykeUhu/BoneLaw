// Copyright by MykeUhu

#include "AbilitySystem/StoneAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h" // FGameplayEffectModCallbackData / EvaluatedData.Attribute comparisons
#include "Core/StoneGameplayTags.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UStoneAttributeSet::UStoneAttributeSet()
{
	const FStoneGameplayTags& GameplayTags = FStoneGameplayTags::Get();

	/* Primary Attributes */
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Strength, GetStrengthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Intelligence, GetIntelligenceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Endurance, GetEnduranceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Willpower, GetWillpowerAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Social, GetSocialAttribute);

	/* Secondary Attributes */
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CarryCapacity, GetCarryCapacityAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_TravelSpeed, GetTravelSpeedAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CraftSpeed, GetCraftSpeedAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_GatherEfficiency, GetGatherEfficiencyAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_InjuryResistance, GetInjuryResistanceAttribute);
	
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxHealth, GetMaxHealthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxFood, GetMaxFoodAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxWater, GetMaxWaterAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxMorale, GetMaxMoraleAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxTrust, GetMaxTrustAttribute);

	/* Vital Attributes */
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_Health, GetHealthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_Food, GetFoodAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_Water, GetWaterAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_Warmth, GetWarmthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_Morale, GetMoraleAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_Trust, GetTrustAttribute);

	/* Culture Attributes */
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Empathy, GetCultureEmpathyAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Hierarchy, GetCultureHierarchyAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Violence, GetCultureViolenceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Spirituality, GetCultureSpiritualityAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Innovation, GetCultureInnovationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Collectivism, GetCultureCollectivismAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Xenophobia, GetCultureXenophobiaAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_TabooStrictness, GetCultureTabooStrictnessAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_DietBalance, GetCultureDietBalanceAttribute);

	/* Knowledge Attributes */
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Medicine, GetKnowledgeMedicineAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Hunting, GetKnowledgeHuntingAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Survival, GetKnowledgeSurvivalAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Craft, GetKnowledgeCraftAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Social, GetKnowledgeSocialAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Courage, GetKnowledgeCourageAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Spiritual, GetKnowledgeSpiritualAttribute);

	/* Worldline Axis Attributes */
	TagsToAttributes.Add(GameplayTags.Attributes_Worldline_MercyRuthless, GetWorldlineMercyRuthlessAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Worldline_TraditionInnovation, GetWorldlineTraditionInnovationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Worldline_CollectiveIndividual, GetWorldlineCollectiveIndividualAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Worldline_SpiritualPractical, GetWorldlineSpiritualPracticalAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Worldline_XenoOpenFear, GetWorldlineXenoOpenFearAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Worldline_TabooLooseStrict, GetWorldlineTabooLooseStrictAttribute);
}

void UStoneAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Primary
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, Endurance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, Willpower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, Social, COND_None, REPNOTIFY_Always);

	// Secondary
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, CarryCapacity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, TravelSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, CraftSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, GatherEfficiency, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, InjuryResistance, COND_None, REPNOTIFY_Always);
			
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, MaxFood,   COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, MaxWater,  COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, MaxMorale, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, MaxTrust,  COND_None, REPNOTIFY_Always);

	// Vital
	
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, Food, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, Water, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, Warmth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, Morale, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, Trust, COND_None, REPNOTIFY_Always);

	// Culture
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, CultureEmpathy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, CultureHierarchy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, CultureViolence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, CultureSpirituality, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, CultureInnovation, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, CultureCollectivism, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, CultureXenophobia, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, CultureTabooStrictness, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, CultureDietBalance, COND_None, REPNOTIFY_Always);

	// Knowledge
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, KnowledgeMedicine, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, KnowledgeHunting, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, KnowledgeSurvival, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, KnowledgeCraft, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, KnowledgeSocial, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, KnowledgeCourage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, KnowledgeSpiritual, COND_None, REPNOTIFY_Always);

	// Worldline Axis
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, WorldlineMercyRuthless, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, WorldlineTraditionInnovation, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, WorldlineCollectiveIndividual, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, WorldlineSpiritualPractical, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, WorldlineXenoOpenFear, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, WorldlineTabooLooseStrict, COND_None, REPNOTIFY_Always);
}

void UStoneAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp current vitals to their max cap
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetFoodAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxFood());
	}
	else if (Attribute == GetWaterAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxWater());
	}
	else if (Attribute == GetMoraleAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMorale());
	}
	else if (Attribute == GetTrustAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxTrust());
	} 
}


void UStoneAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
		if (Props.SourceController == nullptr && Props.SourceAvatarActor)
		{
			if (const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}
		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}

void UStoneAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	
	// Clamp after any direct modifications
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetFoodAttribute())
	{
		SetFood(FMath::Clamp(GetFood(), 0.f, GetMaxFood()));
	}
	else if (Data.EvaluatedData.Attribute == GetWaterAttribute())
	{
		SetWater(FMath::Clamp(GetWater(), 0.f, GetMaxWater()));
	}
	else if (Data.EvaluatedData.Attribute == GetMoraleAttribute())
	{
		SetMorale(FMath::Clamp(GetMorale(), 0.f, GetMaxMorale()));
	}
	else if (Data.EvaluatedData.Attribute == GetTrustAttribute())
	{
		SetTrust(FMath::Clamp(GetTrust(), 0.f, GetMaxTrust()));
	}
	

	// Meta: IncomingDamage
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalIncomingDamage = GetIncomingDamage();
		SetIncomingDamage(0.f);
		if (LocalIncomingDamage > 0.f)
		{
			const float NewHealth = GetHealth() - LocalIncomingDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
		}
	}

	// Meta: IncomingHeal
	if (Data.EvaluatedData.Attribute == GetIncomingHealAttribute())
	{
		const float LocalIncomingHeal = GetIncomingHeal();
		SetIncomingHeal(0.f);
		if (LocalIncomingHeal > 0.f)
		{
			const float NewHealth = GetHealth() + LocalIncomingHeal;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
		}
	}
}

void UStoneAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// Aura pattern: top off health when MaxHealth increases (e.g. after level up)
	if (Attribute == GetMaxHealthAttribute() && bTopOffHealth)
	{
		SetHealth(GetMaxHealth());
		bTopOffHealth = false;
	}
}

// =========================
// OnRep - parameter named after attribute
// =========================

// Primary
void UStoneAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Strength, OldStrength); }
void UStoneAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Intelligence, OldIntelligence); }
void UStoneAttributeSet::OnRep_Endurance(const FGameplayAttributeData& OldEndurance) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Endurance, OldEndurance); }
void UStoneAttributeSet::OnRep_Willpower(const FGameplayAttributeData& OldWillpower) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Willpower, OldWillpower); }
void UStoneAttributeSet::OnRep_Social(const FGameplayAttributeData& OldSocial) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Social, OldSocial); }

// Secondary
void UStoneAttributeSet::OnRep_CarryCapacity(const FGameplayAttributeData& OldCarryCapacity) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CarryCapacity, OldCarryCapacity); }
void UStoneAttributeSet::OnRep_TravelSpeed(const FGameplayAttributeData& OldTravelSpeed) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, TravelSpeed, OldTravelSpeed); }
void UStoneAttributeSet::OnRep_CraftSpeed(const FGameplayAttributeData& OldCraftSpeed) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CraftSpeed, OldCraftSpeed); }
void UStoneAttributeSet::OnRep_GatherEfficiency(const FGameplayAttributeData& OldGatherEfficiency) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, GatherEfficiency, OldGatherEfficiency); }
void UStoneAttributeSet::OnRep_InjuryResistance(const FGameplayAttributeData& OldInjuryResistance) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, InjuryResistance, OldInjuryResistance); }

void UStoneAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, MaxHealth, OldMaxHealth); }
void UStoneAttributeSet::OnRep_MaxFood(const FGameplayAttributeData& OldMaxFood) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, MaxFood, OldMaxFood); }
void UStoneAttributeSet::OnRep_MaxWater(const FGameplayAttributeData& OldMaxWater) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, MaxWater, OldMaxWater); }
void UStoneAttributeSet::OnRep_MaxMorale(const FGameplayAttributeData& OldMaxMorale) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, MaxMorale, OldMaxMorale); }
void UStoneAttributeSet::OnRep_MaxTrust(const FGameplayAttributeData& OldMaxTrust) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, MaxTrust, OldMaxTrust); }

// Vital
void UStoneAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Health, OldHealth); }
void UStoneAttributeSet::OnRep_Food(const FGameplayAttributeData& OldFood) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Food, OldFood); }
void UStoneAttributeSet::OnRep_Water(const FGameplayAttributeData& OldWater) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Water, OldWater); }
void UStoneAttributeSet::OnRep_Warmth(const FGameplayAttributeData& OldWarmth) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Warmth, OldWarmth); }
void UStoneAttributeSet::OnRep_Morale(const FGameplayAttributeData& OldMorale) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Morale, OldMorale); }
void UStoneAttributeSet::OnRep_Trust(const FGameplayAttributeData& OldTrust) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Trust, OldTrust); }

// Culture
void UStoneAttributeSet::OnRep_CultureEmpathy(const FGameplayAttributeData& OldCultureEmpathy) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureEmpathy, OldCultureEmpathy); }
void UStoneAttributeSet::OnRep_CultureHierarchy(const FGameplayAttributeData& OldCultureHierarchy) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureHierarchy, OldCultureHierarchy); }
void UStoneAttributeSet::OnRep_CultureViolence(const FGameplayAttributeData& OldCultureViolence) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureViolence, OldCultureViolence); }
void UStoneAttributeSet::OnRep_CultureSpirituality(const FGameplayAttributeData& OldCultureSpirituality) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureSpirituality, OldCultureSpirituality); }
void UStoneAttributeSet::OnRep_CultureInnovation(const FGameplayAttributeData& OldCultureInnovation) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureInnovation, OldCultureInnovation); }
void UStoneAttributeSet::OnRep_CultureCollectivism(const FGameplayAttributeData& OldCultureCollectivism) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureCollectivism, OldCultureCollectivism); }
void UStoneAttributeSet::OnRep_CultureXenophobia(const FGameplayAttributeData& OldCultureXenophobia) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureXenophobia, OldCultureXenophobia); }
void UStoneAttributeSet::OnRep_CultureTabooStrictness(const FGameplayAttributeData& OldCultureTabooStrictness) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureTabooStrictness, OldCultureTabooStrictness); }
void UStoneAttributeSet::OnRep_CultureDietBalance(const FGameplayAttributeData& OldCultureDietBalance) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureDietBalance, OldCultureDietBalance); }

// Knowledge
void UStoneAttributeSet::OnRep_KnowledgeMedicine(const FGameplayAttributeData& OldKnowledgeMedicine) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeMedicine, OldKnowledgeMedicine); }
void UStoneAttributeSet::OnRep_KnowledgeHunting(const FGameplayAttributeData& OldKnowledgeHunting) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeHunting, OldKnowledgeHunting); }
void UStoneAttributeSet::OnRep_KnowledgeSurvival(const FGameplayAttributeData& OldKnowledgeSurvival) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeSurvival, OldKnowledgeSurvival); }
void UStoneAttributeSet::OnRep_KnowledgeCraft(const FGameplayAttributeData& OldKnowledgeCraft) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeCraft, OldKnowledgeCraft); }
void UStoneAttributeSet::OnRep_KnowledgeSocial(const FGameplayAttributeData& OldKnowledgeSocial) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeSocial, OldKnowledgeSocial); }
void UStoneAttributeSet::OnRep_KnowledgeCourage(const FGameplayAttributeData& OldKnowledgeCourage) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeCourage, OldKnowledgeCourage); }
void UStoneAttributeSet::OnRep_KnowledgeSpiritual(const FGameplayAttributeData& OldKnowledgeSpiritual) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeSpiritual, OldKnowledgeSpiritual); }

// Worldline Axis
void UStoneAttributeSet::OnRep_WorldlineMercyRuthless(const FGameplayAttributeData& OldWorldlineMercyRuthless) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, WorldlineMercyRuthless, OldWorldlineMercyRuthless); }
void UStoneAttributeSet::OnRep_WorldlineTraditionInnovation(const FGameplayAttributeData& OldWorldlineTraditionInnovation) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, WorldlineTraditionInnovation, OldWorldlineTraditionInnovation); }
void UStoneAttributeSet::OnRep_WorldlineCollectiveIndividual(const FGameplayAttributeData& OldWorldlineCollectiveIndividual) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, WorldlineCollectiveIndividual, OldWorldlineCollectiveIndividual); }
void UStoneAttributeSet::OnRep_WorldlineSpiritualPractical(const FGameplayAttributeData& OldWorldlineSpiritualPractical) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, WorldlineSpiritualPractical, OldWorldlineSpiritualPractical); }
void UStoneAttributeSet::OnRep_WorldlineXenoOpenFear(const FGameplayAttributeData& OldWorldlineXenoOpenFear) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, WorldlineXenoOpenFear, OldWorldlineXenoOpenFear); }
void UStoneAttributeSet::OnRep_WorldlineTabooLooseStrict(const FGameplayAttributeData& OldWorldlineTabooLooseStrict) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, WorldlineTabooLooseStrict, OldWorldlineTabooLooseStrict); }
