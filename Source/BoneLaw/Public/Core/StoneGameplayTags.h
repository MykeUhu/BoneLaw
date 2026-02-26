// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Central registry of native gameplay tags for BoneLaw / Stone.
 *
 * Guidelines:
 * - Keep groups stable: UI & GAS meta, global state, action system, regions/unlocks,
 *   events (categories + phases), status, worldline, milestones, attributes.
 * - Naming mirrors the actual tag path hierarchy (e.g. State.*, Event.*, Attributes.*).
 * - Do NOT remove or rename fields casually: referenced by BP/DT/Logic.
 */
struct FStoneGameplayTags
{
public:
	static const FStoneGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	// =====================================================================
	// ABILITIES (GAS/UI meta)
	// =====================================================================
	FGameplayTag Abilities_None;

	// Ability acquisition / UI state (for widgets, tooltips, gating)
	FGameplayTag Abilities_Status_Equipped;
	FGameplayTag Abilities_Status_Unlocked;
	FGameplayTag Abilities_Status_Eligible;

	// Ability type (classification for UI/filters, not gameplay state)
	FGameplayTag Abilities_Type_Passive;
	FGameplayTag Abilities_Type_Offensive;

	// Input tags (EnhancedInput -> GAS input mapping, etc.)
	FGameplayTag InputTag;

	// =====================================================================
	// CORE STATE (global run + camera + pawn state)
	// =====================================================================

	// Day/Night cycle
	FGameplayTag State_Day;
	FGameplayTag State_Night;

	// Run gating / global progress flags
	FGameplayTag State_RunStarted;      // Gate "pre-run" events after first choice
	FGameplayTag State_RegionSelected;  // Starting region has been chosen

	// Global exploration / location context
	FGameplayTag State_InCave;
	FGameplayTag State_OnExpedition;

	// Camera / commander modes
	FGameplayTag State_Camera_Free;           // FreeCam mode (default, flying spectator)
	FGameplayTag State_Camera_Follow;         // Following/observing a selected settler
	FGameplayTag State_Camera_BuildPlacement; // Build ghost placement active
	FGameplayTag State_Camera_UI;             // UI interaction (radial menu, etc.)

	// High-level realtime orchestration flags (Stone systems)
	FGameplayTag State_OnAction;
	FGameplayTag State_OnTravel;

	// Injury / found / small state flags
	FGameplayTag State_Injury_Leg;
	FGameplayTag State_Found_Berries;
	FGameplayTag State_Found_SharpStone;

	// ---------------------------------------------------------------------
	// BehaviorTree / Blackboard-used pawn state tags (SSOT via GAS states)
	// ---------------------------------------------------------------------
	FGameplayTag State_Idle;
	FGameplayTag State_Travel_ToActionStart;
	FGameplayTag State_Travel_Returning;
	FGameplayTag State_Action_Running;

	// =====================================================================
	// ACTION TAGS (Action System - separate from Events)
	// =====================================================================

	// High-level action types (used to select logic/pools)
	FGameplayTag Action_Explore_Area;

	// --- Abort / flow control (set by Outcome on settler ASC, polled by ActionComponent) ---
	// Action.Abort            -> stop the action immediately as a failure
	// Action.ReturnImmediately -> skip remaining outbound/arrival, jump straight to Return phase
	FGameplayTag Action_Abort;
	FGameplayTag Action_ReturnImmediately;
	
	// =====================================================================
	// EXIT / TRANSITION TAGS (BP convenience)
	// =====================================================================
	FGameplayTag Exit_Forest;
	FGameplayTag Event_Travel_EnterForest;

	// =====================================================================
	// REGIONS (run-level identity)
	// =====================================================================
	FGameplayTag Region_Core; // Demo region: core area

	// =====================================================================
	// UNLOCKS (progress / discoveries)
	// =====================================================================
	FGameplayTag Unlock_Fire;
	FGameplayTag Unlock_Shelter;
	FGameplayTag Unlock_Traps;
	FGameplayTag Unlock_Herbs;
	FGameplayTag Unlock_Cooking;
	FGameplayTag Unlock_SharpStone;

	// =====================================================================
	// FOCUS (UI hotspots / player intent)
	// =====================================================================
	FGameplayTag Focus_Hunt;
	FGameplayTag Focus_Shelter;
	FGameplayTag Focus_Water;
	FGameplayTag Focus_Fire;
	FGameplayTag Focus_Forage;
	FGameplayTag Focus_Explore;

	// =====================================================================
	// STATUS TAGS (optional, rules/pools/UI)
	// =====================================================================
	FGameplayTag Status_Bleeding;
	FGameplayTag Status_Fever;
	FGameplayTag Status_Exhaustion;
	FGameplayTag Status_Paranoia;
	FGameplayTag Status_Grief;

	// =====================================================================
	// WORLDLINE (hidden narrative state that shapes the run)
	// =====================================================================

	// Worldline poles (coarse classification; usually derived from axis attributes)
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

	// Optional “hard flags” (milestones / irreversible unlocks)
	FGameplayTag Worldline_CannibalismUnlocked;
	FGameplayTag Worldline_RaidersAttracted;
	FGameplayTag Worldline_HealerPath;
	FGameplayTag Worldline_ToolmakerPath;

	// ---------------------------------------------------------------------
	// Milestone Event Tags (injected events triggered by worldline thresholds)
	// Identify which event to queue when a milestone is reached.
	// ---------------------------------------------------------------------
	FGameplayTag MilestoneEvent_Cannibal_FirstTime;
	FGameplayTag MilestoneEvent_Tools_Breakthrough;
	FGameplayTag MilestoneEvent_Healer_Breakthrough;
	FGameplayTag MilestoneEvent_Raiders_FirstContact;
	FGameplayTag MilestoneEvent_Spirits_Awakening;
	FGameplayTag MilestoneEvent_Taboo_Shattered;

	// =====================================================================
	// ATTRIBUTES (GAS)
	// =====================================================================

	// -------------------------
	// Primary Attributes
	// -------------------------
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Endurance;
	FGameplayTag Attributes_Primary_Willpower;
	FGameplayTag Attributes_Primary_Social;

	// -------------------------
	// Secondary Attributes
	// -------------------------
	FGameplayTag Attributes_Secondary_CarryCapacity;
	FGameplayTag Attributes_Secondary_TravelSpeed;
	FGameplayTag Attributes_Secondary_CraftSpeed;
	FGameplayTag Attributes_Secondary_GatherEfficiency;
	FGameplayTag Attributes_Secondary_InjuryResistance;

	// Max pools (derived caps)
	FGameplayTag Attributes_Secondary_MaxFood;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxWater;
	// (comment kept from you) no neee of maxwarmth
	FGameplayTag Attributes_Secondary_MaxTrust;
	FGameplayTag Attributes_Secondary_MaxMorale;

	// -------------------------
	// Vital Attributes (live values)
	// -------------------------
	FGameplayTag Attributes_Vital_Health;
	FGameplayTag Attributes_Vital_Food;
	FGameplayTag Attributes_Vital_Water;
	FGameplayTag Attributes_Vital_Warmth;
	FGameplayTag Attributes_Vital_Morale;
	FGameplayTag Attributes_Vital_Trust;

	// -------------------------
	// Meta Attributes (incoming)
	// -------------------------
	FGameplayTag Attributes_Meta_IncomingDamage;
	FGameplayTag Attributes_Meta_IncomingHeal;

	// =====================================================================
	// CULTURE ATTRIBUTES (0..100, 50 = neutral where axis-like)
	// =====================================================================
	FGameplayTag Attributes_Culture_Empathy;
	FGameplayTag Attributes_Culture_Hierarchy;
	FGameplayTag Attributes_Culture_Violence;
	FGameplayTag Attributes_Culture_Spirituality;
	FGameplayTag Attributes_Culture_Innovation;
	FGameplayTag Attributes_Culture_Collectivism;
	FGameplayTag Attributes_Culture_Xenophobia;
	FGameplayTag Attributes_Culture_TabooStrictness;
	FGameplayTag Attributes_Culture_DietBalance;

	// =====================================================================
	// KNOWLEDGE ATTRIBUTES (0..100)
	// =====================================================================
	FGameplayTag Attributes_Knowledge_Medicine;
	FGameplayTag Attributes_Knowledge_Hunting;
	FGameplayTag Attributes_Knowledge_Survival;
	FGameplayTag Attributes_Knowledge_Craft;
	FGameplayTag Attributes_Knowledge_Social;
	FGameplayTag Attributes_Knowledge_Courage;
	FGameplayTag Attributes_Knowledge_Spiritual;

	// =====================================================================
	// WORLDLINE AXIS ATTRIBUTES (0..100, 50 = neutral)
	// Axis meaning:
	// - MercyRuthless:      Barmherzig  <-> Gnadenlos
	// - TraditionInnovation:Tradition   <-> Innovation
	// - CollectiveIndividual:Kollektiv  <-> Individualismus
	// - SpiritualPractical: Spiritualität <-> Pragmatismus
	// - XenoOpenFear:      Weltoffen    <-> Fremdenfurcht
	// - TabooLooseStrict:  Locker       <-> Streng
	// =====================================================================
	FGameplayTag Attributes_Worldline_MercyRuthless;
	FGameplayTag Attributes_Worldline_TraditionInnovation;
	FGameplayTag Attributes_Worldline_CollectiveIndividual;
	FGameplayTag Attributes_Worldline_SpiritualPractical;
	FGameplayTag Attributes_Worldline_XenoOpenFear;
	FGameplayTag Attributes_Worldline_TabooLooseStrict;
	
	
	// =====================================================================
	// EVENTS (LEGACY COMPATIBILITY)
	// Keep these tags even if the new SettlerAction/MVVM system replaces RunSubsystem events.
	// Reason: old subsystems (RunSubsystem/Resolver/WeightPolicy) still reference them.
	// =====================================================================

	// Generic / ambient
	FGameplayTag Event_Ambient;
	FGameplayTag Event_Night;

	// Core categories used by resolver/weight policy (legacy weighting rules)
	FGameplayTag Event_Hunt;
	FGameplayTag Event_Forage;
	FGameplayTag Event_Shelter;
	FGameplayTag Event_Fire;
	FGameplayTag Event_Water;
	FGameplayTag Event_Illness;
	FGameplayTag Event_Injury;
	FGameplayTag Event_Social;

	// Travel phased events (legacy time-based travel)
	FGameplayTag Event_Travel_Outbound;
	FGameplayTag Event_Travel_Return;
	FGameplayTag Event_Travel_Arrival;
	FGameplayTag Event_Travel_ReturnHome;

	// Explore return (legacy expedition)
	FGameplayTag Event_ExploreReturn;

private:
	static FStoneGameplayTags GameplayTags;
};
