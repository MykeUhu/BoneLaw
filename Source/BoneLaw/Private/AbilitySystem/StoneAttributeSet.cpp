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

	// Primary
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Strength, GetStrengthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Intelligence, GetIntelligenceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Endurance, GetEnduranceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Willpower, GetWillpowerAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Social, GetSocialAttribute);

	// Secondary
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CarryCapacity, GetCarryCapacityAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_TravelSpeed, GetTravelSpeedAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CraftSpeed, GetCraftSpeedAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_GatherEfficiency, GetGatherEfficiencyAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_InjuryResistance, GetInjuryResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_HealthRegeneration, GetHealthRegenerationAttribute);

	// Vital
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_MaxHealth, GetMaxHealthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_Health, GetHealthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_Food, GetFoodAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_Water, GetWaterAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_Warmth, GetWarmthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_Morale, GetMoraleAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Vital_Trust, GetTrustAttribute);

	// Culture
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Empathy, GetCultureEmpathyAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Hierarchy, GetCultureHierarchyAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Violence, GetCultureViolenceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Spirituality, GetCultureSpiritualityAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Innovation, GetCultureInnovationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Collectivism, GetCultureCollectivismAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_Xenophobia, GetCultureXenophobiaAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_TabooStrictness, GetCultureTabooStrictnessAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Culture_DietBalance, GetCultureDietBalanceAttribute);

	// Knowledge
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Medicine, GetKnowledgeMedicineAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Hunting, GetKnowledgeHuntingAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Survival, GetKnowledgeSurvivalAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Craft, GetKnowledgeCraftAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Social, GetKnowledgeSocialAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Courage, GetKnowledgeCourageAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Knowledge_Spiritual, GetKnowledgeSpiritualAttribute);

	// Worldline Axis
	TagsToAttributes.Add(GameplayTags.Attributes_Worldline_MercyRuthless, GetWorldlineMercyRuthlessAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Worldline_TraditionInnovation, GetWorldlineTraditionInnovationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Worldline_CollectiveIndividual, GetWorldlineCollectiveIndividualAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Worldline_SpiritualPractical, GetWorldlineSpiritualPracticalAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Worldline_XenoOpenFear, GetWorldlineXenoOpenFearAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Worldline_TabooLooseStrict, GetWorldlineTabooLooseStrictAttribute);

	// Meta (transient, not saved)
	TagsToAttributes.Add(GameplayTags.Attributes_Meta_IncomingDamage, GetIncomingDamageAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Meta_IncomingHeal, GetIncomingHealAttribute);
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
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);

	// Vital
	DOREPLIFETIME_CONDITION_NOTIFY(UStoneAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
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

static float Clamp01_100(float V) { return FMath::Clamp(V, 0.f, 100.f); }

void UStoneAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
		bTopOffHealth = true;
		return;
	}

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
		return;
	}

	// 0..100 ranges
	if (Attribute == GetFoodAttribute() || Attribute == GetWaterAttribute() || Attribute == GetWarmthAttribute()
		|| Attribute == GetMoraleAttribute() || Attribute == GetTrustAttribute()
		|| Attribute == GetCultureEmpathyAttribute() || Attribute == GetCultureHierarchyAttribute() || Attribute == GetCultureViolenceAttribute()
		|| Attribute == GetCultureSpiritualityAttribute() || Attribute == GetCultureInnovationAttribute() || Attribute == GetCultureCollectivismAttribute()
		|| Attribute == GetCultureXenophobiaAttribute() || Attribute == GetCultureTabooStrictnessAttribute() || Attribute == GetCultureDietBalanceAttribute()
		|| Attribute == GetKnowledgeMedicineAttribute() || Attribute == GetKnowledgeHuntingAttribute() || Attribute == GetKnowledgeSurvivalAttribute()
		|| Attribute == GetKnowledgeCraftAttribute() || Attribute == GetKnowledgeSocialAttribute() || Attribute == GetKnowledgeCourageAttribute()
		|| Attribute == GetKnowledgeSpiritualAttribute()
		|| Attribute == GetWorldlineMercyRuthlessAttribute() || Attribute == GetWorldlineTraditionInnovationAttribute() || Attribute == GetWorldlineCollectiveIndividualAttribute()
		|| Attribute == GetWorldlineSpiritualPracticalAttribute() || Attribute == GetWorldlineXenoOpenFearAttribute() || Attribute == GetWorldlineTabooLooseStrictAttribute())
	{
		NewValue = Clamp01_100(NewValue);
		return;
	}

	// Secondary (keep non-negative where it makes sense)
	if (Attribute == GetCarryCapacityAttribute() || Attribute == GetTravelSpeedAttribute() || Attribute == GetCraftSpeedAttribute()
		|| Attribute == GetGatherEfficiencyAttribute() || Attribute == GetInjuryResistanceAttribute() || Attribute == GetHealthRegenerationAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
		return;
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

	// Meta -> Vital application (BoneLaw = simple)
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float Damage = GetIncomingDamage();
		SetIncomingDamage(0.f);
		if (Damage > 0.f)
		{
			SetHealth(FMath::Clamp(GetHealth() - Damage, 0.f, GetMaxHealth()));
		}
		return;
	}

	if (Data.EvaluatedData.Attribute == GetIncomingHealAttribute())
	{
		const float Heal = GetIncomingHeal();
		SetIncomingHeal(0.f);
		if (Heal > 0.f)
		{
			SetHealth(FMath::Clamp(GetHealth() + Heal, 0.f, GetMaxHealth()));
		}
		return;
	}

	// Safety clamps on any direct writes
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetMaxHealth(FMath::Max(GetMaxHealth(), 1.f));
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetFoodAttribute()) SetFood(Clamp01_100(GetFood()));
	else if (Data.EvaluatedData.Attribute == GetWaterAttribute()) SetWater(Clamp01_100(GetWater()));
	else if (Data.EvaluatedData.Attribute == GetWarmthAttribute()) SetWarmth(Clamp01_100(GetWarmth()));
	else if (Data.EvaluatedData.Attribute == GetMoraleAttribute()) SetMorale(Clamp01_100(GetMorale()));
	else if (Data.EvaluatedData.Attribute == GetTrustAttribute()) SetTrust(Clamp01_100(GetTrust()));
	else if (Data.EvaluatedData.Attribute == GetCultureEmpathyAttribute()) SetCultureEmpathy(Clamp01_100(GetCultureEmpathy()));
	else if (Data.EvaluatedData.Attribute == GetCultureHierarchyAttribute()) SetCultureHierarchy(Clamp01_100(GetCultureHierarchy()));
	else if (Data.EvaluatedData.Attribute == GetCultureViolenceAttribute()) SetCultureViolence(Clamp01_100(GetCultureViolence()));
	else if (Data.EvaluatedData.Attribute == GetCultureSpiritualityAttribute()) SetCultureSpirituality(Clamp01_100(GetCultureSpirituality()));
	else if (Data.EvaluatedData.Attribute == GetCultureInnovationAttribute()) SetCultureInnovation(Clamp01_100(GetCultureInnovation()));
	else if (Data.EvaluatedData.Attribute == GetCultureCollectivismAttribute()) SetCultureCollectivism(Clamp01_100(GetCultureCollectivism()));
	else if (Data.EvaluatedData.Attribute == GetCultureXenophobiaAttribute()) SetCultureXenophobia(Clamp01_100(GetCultureXenophobia()));
	else if (Data.EvaluatedData.Attribute == GetCultureTabooStrictnessAttribute()) SetCultureTabooStrictness(Clamp01_100(GetCultureTabooStrictness()));
	else if (Data.EvaluatedData.Attribute == GetCultureDietBalanceAttribute()) SetCultureDietBalance(Clamp01_100(GetCultureDietBalance()));
	else if (Data.EvaluatedData.Attribute == GetKnowledgeMedicineAttribute()) SetKnowledgeMedicine(Clamp01_100(GetKnowledgeMedicine()));
	else if (Data.EvaluatedData.Attribute == GetKnowledgeHuntingAttribute()) SetKnowledgeHunting(Clamp01_100(GetKnowledgeHunting()));
	else if (Data.EvaluatedData.Attribute == GetKnowledgeSurvivalAttribute()) SetKnowledgeSurvival(Clamp01_100(GetKnowledgeSurvival()));
	else if (Data.EvaluatedData.Attribute == GetKnowledgeCraftAttribute()) SetKnowledgeCraft(Clamp01_100(GetKnowledgeCraft()));
	else if (Data.EvaluatedData.Attribute == GetKnowledgeSocialAttribute()) SetKnowledgeSocial(Clamp01_100(GetKnowledgeSocial()));
	else if (Data.EvaluatedData.Attribute == GetKnowledgeCourageAttribute()) SetKnowledgeCourage(Clamp01_100(GetKnowledgeCourage()));
	else if (Data.EvaluatedData.Attribute == GetKnowledgeSpiritualAttribute()) SetKnowledgeSpiritual(Clamp01_100(GetKnowledgeSpiritual()));
	else if (Data.EvaluatedData.Attribute == GetWorldlineMercyRuthlessAttribute()) SetWorldlineMercyRuthless(Clamp01_100(GetWorldlineMercyRuthless()));
	else if (Data.EvaluatedData.Attribute == GetWorldlineTraditionInnovationAttribute()) SetWorldlineTraditionInnovation(Clamp01_100(GetWorldlineTraditionInnovation()));
	else if (Data.EvaluatedData.Attribute == GetWorldlineCollectiveIndividualAttribute()) SetWorldlineCollectiveIndividual(Clamp01_100(GetWorldlineCollectiveIndividual()));
	else if (Data.EvaluatedData.Attribute == GetWorldlineSpiritualPracticalAttribute()) SetWorldlineSpiritualPractical(Clamp01_100(GetWorldlineSpiritualPractical()));
	else if (Data.EvaluatedData.Attribute == GetWorldlineXenoOpenFearAttribute()) SetWorldlineXenoOpenFear(Clamp01_100(GetWorldlineXenoOpenFear()));
	else if (Data.EvaluatedData.Attribute == GetWorldlineTabooLooseStrictAttribute()) SetWorldlineTabooLooseStrict(Clamp01_100(GetWorldlineTabooLooseStrict()));
}

void UStoneAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		// Ensure health never exceeds MaxHealth; optionally top off when MaxHealth increases.
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		if (bTopOffHealth && NewValue > OldValue)
		{
			SetHealth(GetMaxHealth());
		}
		bTopOffHealth = false;
	}
	else if (Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(NewValue, 0.f, GetMaxHealth()));
	}
}

// =========================
// OnRep
// =========================

// Primary
void UStoneAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Strength, OldValue); }
void UStoneAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Intelligence, OldValue); }
void UStoneAttributeSet::OnRep_Endurance(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Endurance, OldValue); }
void UStoneAttributeSet::OnRep_Willpower(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Willpower, OldValue); }
void UStoneAttributeSet::OnRep_Social(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Social, OldValue); }

// Secondary
void UStoneAttributeSet::OnRep_CarryCapacity(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CarryCapacity, OldValue); }
void UStoneAttributeSet::OnRep_TravelSpeed(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, TravelSpeed, OldValue); }
void UStoneAttributeSet::OnRep_CraftSpeed(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CraftSpeed, OldValue); }
void UStoneAttributeSet::OnRep_GatherEfficiency(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, GatherEfficiency, OldValue); }
void UStoneAttributeSet::OnRep_InjuryResistance(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, InjuryResistance, OldValue); }
void UStoneAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, HealthRegeneration, OldValue); }

// Vital
void UStoneAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, MaxHealth, OldValue); }
void UStoneAttributeSet::OnRep_Food(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Food, OldValue); }
void UStoneAttributeSet::OnRep_Water(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Water, OldValue); }
void UStoneAttributeSet::OnRep_Warmth(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Warmth, OldValue); }
void UStoneAttributeSet::OnRep_Morale(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Morale, OldValue); }
void UStoneAttributeSet::OnRep_Trust(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Trust, OldValue); }
void UStoneAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, Health, OldValue); }

// Culture
void UStoneAttributeSet::OnRep_CultureEmpathy(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureEmpathy, OldValue); }
void UStoneAttributeSet::OnRep_CultureHierarchy(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureHierarchy, OldValue); }
void UStoneAttributeSet::OnRep_CultureViolence(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureViolence, OldValue); }
void UStoneAttributeSet::OnRep_CultureSpirituality(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureSpirituality, OldValue); }
void UStoneAttributeSet::OnRep_CultureInnovation(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureInnovation, OldValue); }
void UStoneAttributeSet::OnRep_CultureCollectivism(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureCollectivism, OldValue); }
void UStoneAttributeSet::OnRep_CultureXenophobia(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureXenophobia, OldValue); }
void UStoneAttributeSet::OnRep_CultureTabooStrictness(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureTabooStrictness, OldValue); }
void UStoneAttributeSet::OnRep_CultureDietBalance(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, CultureDietBalance, OldValue); }

// Knowledge
void UStoneAttributeSet::OnRep_KnowledgeMedicine(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeMedicine, OldValue); }
void UStoneAttributeSet::OnRep_KnowledgeHunting(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeHunting, OldValue); }
void UStoneAttributeSet::OnRep_KnowledgeSurvival(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeSurvival, OldValue); }
void UStoneAttributeSet::OnRep_KnowledgeCraft(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeCraft, OldValue); }
void UStoneAttributeSet::OnRep_KnowledgeSocial(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeSocial, OldValue); }
void UStoneAttributeSet::OnRep_KnowledgeCourage(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeCourage, OldValue); }
void UStoneAttributeSet::OnRep_KnowledgeSpiritual(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, KnowledgeSpiritual, OldValue); }

// Worldline Axis
void UStoneAttributeSet::OnRep_WorldlineMercyRuthless(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, WorldlineMercyRuthless, OldValue); }
void UStoneAttributeSet::OnRep_WorldlineTraditionInnovation(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, WorldlineTraditionInnovation, OldValue); }
void UStoneAttributeSet::OnRep_WorldlineCollectiveIndividual(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, WorldlineCollectiveIndividual, OldValue); }
void UStoneAttributeSet::OnRep_WorldlineSpiritualPractical(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, WorldlineSpiritualPractical, OldValue); }
void UStoneAttributeSet::OnRep_WorldlineXenoOpenFear(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, WorldlineXenoOpenFear, OldValue); }
void UStoneAttributeSet::OnRep_WorldlineTabooLooseStrict(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(UStoneAttributeSet, WorldlineTabooLooseStrict, OldValue); }
