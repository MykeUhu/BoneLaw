// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 *
 */
struct FStoneGameplayTags
{
public:
	static const FStoneGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();
	
	// =========================
	// Abilities (GAS/UI meta)
	// =========================
	FGameplayTag Abilities_None;

	FGameplayTag Abilities_Status_Equipped;
	FGameplayTag Abilities_Status_Unlocked;
	FGameplayTag Abilities_Status_Eligible;

	FGameplayTag Abilities_Type_Passive;
	FGameplayTag Abilities_Type_Offensive;

	// Input
	FGameplayTag InputTag;
	
	// =========================
	// Core State
	// =========================
	FGameplayTag State_Day;
	FGameplayTag State_Night;
	FGameplayTag State_RunStarted; // NEW: Gate "non-start" events after the first choice
	// Run start flow
	FGameplayTag State_RegionSelected; // Run state: starting region has been chosen
	FGameplayTag State_InCave;
	FGameplayTag State_OnExpedition;

	// Action state (real-time actions orchestrated by Stone)
	FGameplayTag State_OnAction;
	FGameplayTag State_OnTravel;
	
	FGameplayTag State_Injury_Leg;
	
	FGameplayTag State_Found_Berries;
	FGameplayTag State_Found_SharpStone;
	
	FGameplayTag Action_Explore_Area;

	// =========================
	// Regions (run-level identity)
	// =========================
	FGameplayTag Region_Core; // Demo region: core area

	// =========================
	// Unlocks (Progress / Discoveries)
	// =========================
	FGameplayTag Unlock_Fire;
	FGameplayTag Unlock_Shelter;
	FGameplayTag Unlock_Traps;
	FGameplayTag Unlock_Herbs;
	FGameplayTag Unlock_Cooking;
	FGameplayTag Unlock_SharpStone;

	// =========================
	// Focus (UI Hotspots)
	// =========================
	FGameplayTag Focus_Hunt;
	FGameplayTag Focus_Shelter;
	FGameplayTag Focus_Water;
	FGameplayTag Focus_Fire;
	FGameplayTag Focus_Forage;
	FGameplayTag Focus_Explore;

	// =========================
	// Event Tags (used for weighting + gating)
	// =========================
	FGameplayTag Event_Hunt;
	FGameplayTag Event_Forage;
	FGameplayTag Event_Shelter;
	FGameplayTag Event_Water;
	FGameplayTag Event_Fire;
	FGameplayTag Event_Social;
	FGameplayTag Event_Illness;
	FGameplayTag Event_Injury;
	FGameplayTag Event_Night;
	FGameplayTag Event_Day;
	FGameplayTag Event_FindStone;
	FGameplayTag Event_Wildlife;
	
	// =========================
	// Event category
	// =========================
	FGameplayTag Event_Explore;
	FGameplayTag Event_ExploreReturn;

	// Travel phase tags (used for action-driven travel)
	FGameplayTag Event_Travel_Outbound;
	FGameplayTag Event_Travel_Arrival;
	FGameplayTag Event_Travel_Return;
	FGameplayTag Event_Travel_ReturnHome;

	// Ambient/idle random events (usually queued, not auto-presented)
	FGameplayTag Event_Ambient;

	// =========================
	// Status Tags (optional, for rules/pools/UI)
	// =========================
	FGameplayTag Status_Bleeding;
	FGameplayTag Status_Fever;
	FGameplayTag Status_Exhaustion;
	FGameplayTag Status_Paranoia;
	FGameplayTag Status_Grief;
	
	// =========================
	// Worldline (hidden narrative state that shapes the run)
	// =========================
	FGameplayTag Worldline_Merciful;
	FGameplayTag Worldline_Ruthless;

	FGameplayTag Worldline_Tradition;
	FGameplayTag Worldline_Innovation;

	FGameplayTag Worldline_Collective;
	FGameplayTag Worldline_Individual;

	FGameplayTag Worldline_Spiritual;
	FGameplayTag Worldline_Practical;

	FGameplayTag Worldline_Xenophile;
	FGameplayTag Worldline_Xenophobic;

	FGameplayTag Worldline_TabooLoose;
	FGameplayTag Worldline_TabooStrict;

	// Optional “hard flags” (milestones)
	FGameplayTag Worldline_CannibalismUnlocked;
	FGameplayTag Worldline_RaidersAttracted;
	FGameplayTag Worldline_HealerPath;
	FGameplayTag Worldline_ToolmakerPath;

	// =========================
	// Milestone Event Tags (injected events triggered by worldline thresholds)
	// These tags identify which event to queue when a milestone is reached.
	// The WorldlineDirector uses these instead of hardcoded event IDs.
	// =========================
	FGameplayTag MilestoneEvent_Cannibal_FirstTime;
	FGameplayTag MilestoneEvent_Tools_Breakthrough;
	FGameplayTag MilestoneEvent_Healer_Breakthrough;
	FGameplayTag MilestoneEvent_Raiders_FirstContact;
	FGameplayTag MilestoneEvent_Spirits_Awakening;
	FGameplayTag MilestoneEvent_Taboo_Shattered;
	
	// =========================
	// Attributes (GAS)
	// =========================
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Endurance;
	FGameplayTag Attributes_Primary_Willpower;
	FGameplayTag Attributes_Primary_Social;

	FGameplayTag Attributes_Secondary_CarryCapacity;
	FGameplayTag Attributes_Secondary_TravelSpeed;
	FGameplayTag Attributes_Secondary_CraftSpeed;
	FGameplayTag Attributes_Secondary_GatherEfficiency;
	FGameplayTag Attributes_Secondary_InjuryResistance;

	FGameplayTag Attributes_Vital_Health;
	FGameplayTag Attributes_Vital_MaxHealth;
	FGameplayTag Attributes_Vital_HealthRegeneration;
	FGameplayTag Attributes_Vital_Food;
	FGameplayTag Attributes_Vital_Water;
	FGameplayTag Attributes_Vital_Warmth;
	FGameplayTag Attributes_Vital_Morale;
	FGameplayTag Attributes_Vital_Trust;

	FGameplayTag Attributes_Meta_IncomingDamage;
	FGameplayTag Attributes_Meta_IncomingHeal;

	// =========================
	// Culture Attributes (0..100, 50 = neutral where axis-like)
	// =========================
	FGameplayTag Attributes_Culture_Empathy;
	FGameplayTag Attributes_Culture_Hierarchy;
	FGameplayTag Attributes_Culture_Violence;
	FGameplayTag Attributes_Culture_Spirituality;
	FGameplayTag Attributes_Culture_Innovation;
	FGameplayTag Attributes_Culture_Collectivism;
	FGameplayTag Attributes_Culture_Xenophobia;
	FGameplayTag Attributes_Culture_TabooStrictness;
	FGameplayTag Attributes_Culture_DietBalance;

	// =========================
	// Knowledge Attributes (0..100)
	// =========================
	FGameplayTag Attributes_Knowledge_Medicine;
	FGameplayTag Attributes_Knowledge_Hunting;
	FGameplayTag Attributes_Knowledge_Survival;
	FGameplayTag Attributes_Knowledge_Craft;
	FGameplayTag Attributes_Knowledge_Social;
	FGameplayTag Attributes_Knowledge_Courage;
	FGameplayTag Attributes_Knowledge_Spiritual;

	// =========================
	// Worldline Axis Attributes (0..100, 50 = neutral)
	// =========================
	FGameplayTag Attributes_Worldline_MercyRuthless;
	FGameplayTag Attributes_Worldline_TraditionInnovation;
	FGameplayTag Attributes_Worldline_CollectiveIndividual;
	FGameplayTag Attributes_Worldline_SpiritualPractical;
	FGameplayTag Attributes_Worldline_XenoOpenFear;
	FGameplayTag Attributes_Worldline_TabooLooseStrict;


private:
	static FStoneGameplayTags GameplayTags;
};
