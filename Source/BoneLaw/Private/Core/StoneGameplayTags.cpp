// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/StoneGameplayTags.h"

#include "GameplayTagsManager.h"

FStoneGameplayTags FStoneGameplayTags::GameplayTags;

void FStoneGameplayTags::InitializeNativeGameplayTags()
{
	// =========================
	// Abilities (GAS/UI meta)
	// =========================
	GameplayTags.Abilities_None = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.None"),
		FString("GAS: No ability selected / empty ability tag")
	);

	GameplayTags.Abilities_Status_Equipped = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Status.Equipped"),
		FString("GAS: Ability is equipped (active in slot/loadout)")
	);

	GameplayTags.Abilities_Status_Unlocked = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Status.Unlocked"),
		FString("GAS: Ability is unlocked but not equipped")
	);

	GameplayTags.Abilities_Status_Eligible = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Status.Eligible"),
		FString("GAS: Ability is eligible to unlock (meets requirements)")
	);

	GameplayTags.Abilities_Type_Passive = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Type.Passive"),
		FString("GAS: Passive ability")
	);

	GameplayTags.Abilities_Type_Offensive = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Type.Offensive"),
		FString("GAS: Offensive/active ability")
	);

	// =========================
	// Input
	// =========================
	GameplayTags.InputTag = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag"),
		FString("GAS: Root for input tags (InputTag.*)")
	);
	
	// =========================
	// Core State
	// =========================
	GameplayTags.State_Day = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Day"),
		FString("Run state: Day")
	);

	GameplayTags.State_Night = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Night"),
		FString("Run state: Night")
	);

	GameplayTags.State_RunStarted = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.RunStarted"),
		FString("Run state: after the start event has been resolved (first choice made)")
	);

	GameplayTags.State_RegionSelected = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.RegionSelected"),
		FString("Run state: starting region has been chosen")
	);
	
	GameplayTags.State_InCave = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.InCave"),
		FString("Run state: player is in cave / idle between events")
	);

	GameplayTags.State_OnExpedition = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.OnExpedition"),
		FString("Run state: player is currently on an expedition")
	);

	GameplayTags.State_OnAction = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.OnAction"),
		FString("Run state: player is currently performing a real-time action")
	);

	GameplayTags.State_OnTravel = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.OnTravel"),
		FString("Run state: player is currently travelling (a travel action is active)")
	);
	
	
	GameplayTags.State_Injury_Leg = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Injury.Leg"),
		FString("Injury: Leg")
		);
	
	GameplayTags.State_Found_Berries = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Found.Berries"),
		FString("Found: Berries")
		);
	
	GameplayTags.State_Found_SharpStone = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Found.SharpStone"),
		FString("Found: Sharp Stone")
		);
	
	GameplayTags.Action_Explore_Area= UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Action.Explore.Area"),
		FString("Action: Explore an area")
		);

	// =========================
	// Regions
	// =========================
	GameplayTags.Region_Core = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Region.Core"),
		FString("Region: Demo start region (core area)")
	);

	// =========================
	// Unlocks (Progress / Discoveries)
	// =========================
	GameplayTags.Unlock_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Unlock.Fire"),
		FString("Discovery: Fire unlocked / usable")
	);

	GameplayTags.Unlock_Shelter = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Unlock.Shelter"),
		FString("Discovery: Shelter improvements unlocked")
	);

	GameplayTags.Unlock_Traps = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Unlock.Traps"),
		FString("Discovery: Traps unlocked")
	);

	GameplayTags.Unlock_Herbs = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Unlock.Herbs"),
		FString("Discovery: Herbs / medicine unlocked")
	);

	GameplayTags.Unlock_Cooking = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Unlock.Cooking"),
		FString("Discovery: Cooking unlocked")
	);
	
	GameplayTags.Unlock_SharpStone = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Unlock.SharpStone"),
		FString("Discovery: Sharp stone unlocked")
	);
	
	// =========================
	// Focus (UI Hotspots)
	// =========================
	GameplayTags.Focus_Hunt = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Focus.Hunt"),
		FString("Focus: Weight next event selection towards hunting")
	);

	GameplayTags.Focus_Shelter = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Focus.Shelter"),
		FString("Focus: Weight next event selection towards shelter/rest")
	);

	GameplayTags.Focus_Water = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Focus.Water"),
		FString("Focus: Weight next event selection towards water")
	);

	GameplayTags.Focus_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Focus.Fire"),
		FString("Focus: Weight next event selection towards fire/cooking")
	);

	GameplayTags.Focus_Forage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Focus.Forage"),
		FString("Focus: Weight next event selection towards foraging")
	);
	
	GameplayTags.Focus_Explore = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Focus.Explore"),
		FString("Focus: exploration / expedition")
	);

	// =========================
	// Event Tags
	// =========================
	GameplayTags.Event_Hunt = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Hunt"),
		FString("Event category: Hunt")
	);

	GameplayTags.Event_Forage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Forage"),
		FString("Event category: Forage")
	);

	GameplayTags.Event_Shelter = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Shelter"),
		FString("Event category: Shelter")
	);

	GameplayTags.Event_Water = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Water"),
		FString("Event category: Water")
	);

	GameplayTags.Event_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Fire"),
		FString("Event category: Fire / cooking")
	);

	GameplayTags.Event_Social = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Social"),
		FString("Event category: Social")
	);

	GameplayTags.Event_Illness = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Illness"),
		FString("Event category: Illness")
	);

	GameplayTags.Event_Injury = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Injury"),
		FString("Event category: Injury")
	);

	GameplayTags.Event_Night = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Night"),
		FString("Event timing: Night")
	);

	GameplayTags.Event_Day = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Day"),
		FString("Event timing: Day")
	);
	
	GameplayTags.Event_FindStone = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.FindStone"),
		FString("Event: Find a stone")
		);
	
	GameplayTags.Event_Wildlife = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Wildlife"),
		FString("Event: Wildlife encounter")
		);
		
	// =========================
	// Event category
	// =========================
	GameplayTags.Event_Explore = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Explore"),
		FString("Event category: Exploration encounter (only during expedition)")
	);

	GameplayTags.Event_ExploreReturn = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.ExploreReturn"),
		FString("Event category: Expedition return event")
	);

	GameplayTags.Event_Travel_Outbound = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Travel.Outbound"),
		FString("Event category: Travel event that can occur on the outbound leg")
	);

	GameplayTags.Event_Travel_Arrival = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Travel.Arrival"),
		FString("Event category: Travel arrival event (must happen at destination)")
	);

	GameplayTags.Event_Travel_Return = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Travel.Return"),
		FString("Event category: Travel event that can occur on the return leg")
	);

	GameplayTags.Event_Travel_ReturnHome = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Travel.ReturnHome"),
		FString("Event category: Travel return-home event (end of travel action)")
	);

	GameplayTags.Event_Ambient = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Ambient"),
		FString("Event category: Ambient/idle random events (usually queued, not auto-presented)")
	);

	// =========================
	// Status Tags (optional)
	// =========================
	GameplayTags.Status_Bleeding = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Status.Bleeding"),
		FString("Status: Bleeding")
	);

	GameplayTags.Status_Fever = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Status.Fever"),
		FString("Status: Fever")
	);

	GameplayTags.Status_Exhaustion = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Status.Exhaustion"),
		FString("Status: Exhaustion")
	);

	GameplayTags.Status_Paranoia = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Status.Paranoia"),
		FString("Status: Paranoia")
	);

	GameplayTags.Status_Grief = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Status.Grief"),
		FString("Status: Grief")
	);
	
	// =========================
	// Worldline
	// =========================
	GameplayTags.Worldline_Merciful = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.Merciful"),
		FString("Worldline: mercy oriented choices dominate")
		);
	
	GameplayTags.Worldline_Ruthless = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.Ruthless"), 
		FString("Worldline: ruthless/survival-at-any-cost dominates")
		);

	GameplayTags.Worldline_Tradition = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.Tradition"),
		FString("Worldline: tradition/ritual dominates")
		);
	
	GameplayTags.Worldline_Innovation = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.Innovation"),
		FString("Worldline: experimentation/innovation dominates")
		);

	GameplayTags.Worldline_Collective = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.Collective"),
		FString("Worldline: collective-first dominates")
		);
	
	GameplayTags.Worldline_Individual = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.Individual"),
		FString("Worldline: individual-first dominates")
		);

	GameplayTags.Worldline_Spiritual = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.Spiritual"), 
		FString("Worldline: spiritual worldview dominates")
		);
	
	GameplayTags.Worldline_Practical = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.Practical"), 
		FString("Worldline: practical/material worldview dominates")
		);

	GameplayTags.Worldline_Xenophile = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.Xenophile"), 
		FString("Worldline: open to outsiders dominates")
		);
	
	GameplayTags.Worldline_Xenophobic = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.Xenophobic"),
		FString("Worldline: fear/hatred of outsiders dominates")
		);

	GameplayTags.Worldline_TabooLoose = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.TabooLoose"), 
		FString("Worldline: taboos loosened")
		);
	
	GameplayTags.Worldline_TabooStrict = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.TabooStrict"),
		FString("Worldline: taboos strict")
		);

	GameplayTags.Worldline_CannibalismUnlocked = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.CannibalismUnlocked"), 
		FString("Worldline milestone: cannibalism mechanics unlocked")
		);
	
	GameplayTags.Worldline_RaidersAttracted = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.RaidersAttracted"),
		FString("Worldline milestone: raiders attracted by your reputation")
		);
	
	GameplayTags.Worldline_HealerPath = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.HealerPath"), 
		FString("Worldline milestone: healer path taken")
		);
	
	GameplayTags.Worldline_ToolmakerPath = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Worldline.ToolmakerPath"), 
		FString("Worldline milestone: toolmaker path taken")
		);

	// =========================
	// Milestone Event Tags (injected events triggered by worldline thresholds)
	// =========================
	GameplayTags.MilestoneEvent_Cannibal_FirstTime = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("MilestoneEvent.Cannibal.FirstTime"),
		FString("Milestone event: First cannibalism opportunity")
		);
	
	GameplayTags.MilestoneEvent_Tools_Breakthrough = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("MilestoneEvent.Tools.Breakthrough"),
		FString("Milestone event: Tool-making breakthrough")
		);
	
	GameplayTags.MilestoneEvent_Healer_Breakthrough = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("MilestoneEvent.Healer.Breakthrough"),
		FString("Milestone event: Healing breakthrough")
		);
	
	GameplayTags.MilestoneEvent_Raiders_FirstContact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("MilestoneEvent.Raiders.FirstContact"),
		FString("Milestone event: First raider contact")
		);
	
	GameplayTags.MilestoneEvent_Spirits_Awakening = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("MilestoneEvent.Spirits.Awakening"),
		FString("Milestone event: Spiritual awakening")
		);
	
	GameplayTags.MilestoneEvent_Taboo_Shattered = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("MilestoneEvent.Taboo.Shattered"),
		FString("Milestone event: A major taboo has been shattered")
		);

	// =========================
	// Attributes (GAS)
	// =========================
	GameplayTags.Attributes_Primary_Strength = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Primary.Strength"), 
	    FString("Primary attribute: Strength")
	    );

	GameplayTags.Attributes_Primary_Intelligence = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Primary.Intelligence"), 
	    FString("Primary attribute: Intelligence")
	    );

	GameplayTags.Attributes_Primary_Endurance = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Primary.Endurance"),
	    FString("Primary attribute: Endurance")
	    );

	GameplayTags.Attributes_Primary_Willpower = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Primary.Willpower"),
	    FString("Primary attribute: Willpower")
	    );

	GameplayTags.Attributes_Primary_Social = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Primary.Social"), 
	    FString("Primary attribute: Social / Trust handling")
	    );

	GameplayTags.Attributes_Secondary_CarryCapacity = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Secondary.CarryCapacity"), 
	    FString("Secondary attribute: Carry capacity")
	    );

	GameplayTags.Attributes_Secondary_TravelSpeed = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Secondary.TravelSpeed"),
	    FString("Secondary attribute: Travel speed")
	    );

	GameplayTags.Attributes_Secondary_CraftSpeed = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Secondary.CraftSpeed"), 
	    FString("Secondary attribute: Craft speed")
	    );

	GameplayTags.Attributes_Secondary_GatherEfficiency = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Secondary.GatherEfficiency"), 
	    FString("Secondary attribute: Gather efficiency")
	    );

	GameplayTags.Attributes_Secondary_InjuryResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Secondary.InjuryResistance"), 
	    FString("Secondary attribute: Injury resistance")
	    );

	GameplayTags.Attributes_Vital_Health = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Vital.Health"), 
	    FString("Vital attribute: Health")
	    );

	GameplayTags.Attributes_Vital_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Vital.MaxHealth"), 
	    FString("Vital attribute: MaxHealth")
	    );

	GameplayTags.Attributes_Vital_HealthRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Vital.HealthRegeneration"), 
	    FString("Vital attribute: Health regeneration")
	    );

	GameplayTags.Attributes_Vital_Food = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Vital.Food"), 
	    FString("Vital attribute: Food")
	    );
	
	GameplayTags.Attributes_Vital_MaxFood = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.MaxFood"),
		FString("Vital attribute: MaxFood")
		);

	GameplayTags.Attributes_Vital_Water = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Vital.Water"),
	    FString("Vital attribute: Water")
	    );
	
	GameplayTags.Attributes_Vital_MaxWater = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.MaxWater"),
		FString("Vital attribute: MaxWater")
		);

	GameplayTags.Attributes_Vital_Warmth = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Vital.Warmth"), 
	    FString("Vital attribute: Warmth")
	    );
	
	GameplayTags.Attributes_Vital_MaxWarmth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.MaxWarmth"),
		FString("Vital attribute: MaxWarmth")
		);

	GameplayTags.Attributes_Vital_Morale = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Vital.Morale"),
	    FString("Vital attribute: Morale")
	    );
	
	GameplayTags.Attributes_Vital_MaxMorale = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.MaxMorale"),
		FString("Vital attribute: MaxMorale")
		);

	GameplayTags.Attributes_Vital_Trust = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Vital.Trust"), 
	    FString("Vital attribute: Trust")
	    );
	
	GameplayTags.Attributes_Vital_MaxTrust = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.MaxTrust"),
		FString("Vital attribute: MaxTrust")
		);

	GameplayTags.Attributes_Meta_IncomingDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Meta.IncomingDamage"), 
	    FString("Meta attribute: Incoming damage")
	    );

	GameplayTags.Attributes_Meta_IncomingHeal = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Meta.IncomingHeal"),
	    FString("Meta attribute: Incoming heal")
	    );

	// =========================
	// Culture Attributes (GAS)
	// =========================
	GameplayTags.Attributes_Culture_Empathy = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Culture.Empathy"),
	    FString("Culture attribute: Empathy (0..100)")
	    );
	GameplayTags.Attributes_Culture_Hierarchy = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Culture.Hierarchy"),
	    FString("Culture attribute: Hierarchy (0..100)")
	    );
	GameplayTags.Attributes_Culture_Violence = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Culture.Violence"),
	    FString("Culture attribute: Violence (0..100)")
	    );
	GameplayTags.Attributes_Culture_Spirituality = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Culture.Spirituality"),
	    FString("Culture attribute: Spirituality (0..100)")
	    );
	GameplayTags.Attributes_Culture_Innovation = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Culture.Innovation"),
	    FString("Culture attribute: Innovation (0..100)")
	    );
	GameplayTags.Attributes_Culture_Collectivism = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Culture.Collectivism"),
	    FString("Culture attribute: Collectivism (0..100)")
	    );
	GameplayTags.Attributes_Culture_Xenophobia = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Culture.Xenophobia"),
	    FString("Culture attribute: Xenophobia (0..100)")
	    );
	GameplayTags.Attributes_Culture_TabooStrictness = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Culture.TabooStrictness"),
	    FString("Culture attribute: TabooStrictness (0..100)")
	    );
	GameplayTags.Attributes_Culture_DietBalance = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Culture.DietBalance"),
	    FString("Culture attribute: DietBalance (0..100, 0=plants, 100=meat)")
	    );

	// =========================
	// Knowledge Attributes (GAS)
	// =========================
	GameplayTags.Attributes_Knowledge_Medicine = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Knowledge.Medicine"),
	    FString("Knowledge attribute: Medicine (0..100)")
	    );
	GameplayTags.Attributes_Knowledge_Hunting = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Knowledge.Hunting"),
	    FString("Knowledge attribute: Hunting (0..100)")
	    );
	GameplayTags.Attributes_Knowledge_Survival = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Knowledge.Survival"),
	    FString("Knowledge attribute: Survival (0..100)")
	    );
	GameplayTags.Attributes_Knowledge_Craft = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Knowledge.Craft"),
	    FString("Knowledge attribute: Craft (0..100)")
	    );
	GameplayTags.Attributes_Knowledge_Social = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Knowledge.Social"),
	    FString("Knowledge attribute: Social (0..100)")
	    );
	GameplayTags.Attributes_Knowledge_Courage = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Knowledge.Courage"),
	    FString("Knowledge attribute: Courage (0..100)")
	    );
	GameplayTags.Attributes_Knowledge_Spiritual = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Knowledge.Spiritual"),
	    FString("Knowledge attribute: Spiritual (0..100)")
	    );

	// =========================
	// Worldline Axis Attributes (GAS)
	// =========================
	GameplayTags.Attributes_Worldline_MercyRuthless = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Worldline.MercyRuthless"),
	    FString("Worldline axis: Mercy(0) .. Ruthless(100), 50=neutral")
	    );
	GameplayTags.Attributes_Worldline_TraditionInnovation = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Worldline.TraditionInnovation"),
	    FString("Worldline axis: Tradition(0) .. Innovation(100), 50=neutral")
	    );
	GameplayTags.Attributes_Worldline_CollectiveIndividual = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Worldline.CollectiveIndividual"),
	    FString("Worldline axis: Collective(0) .. Individual(100), 50=neutral")
	    );
	GameplayTags.Attributes_Worldline_SpiritualPractical = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Worldline.SpiritualPractical"),
	    FString("Worldline axis: Spiritual(0) .. Practical(100), 50=neutral")
	    );
	GameplayTags.Attributes_Worldline_XenoOpenFear = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Worldline.XenoOpenFear"),
	    FString("Worldline axis: Open(0) .. Fear(100), 50=neutral")
	    );
	GameplayTags.Attributes_Worldline_TabooLooseStrict = UGameplayTagsManager::Get().AddNativeGameplayTag(
	    FName("Attributes.Worldline.TabooLooseStrict"),
	    FString("Worldline axis: Loose(0) .. Strict(100), 50=neutral")
	    );
}
