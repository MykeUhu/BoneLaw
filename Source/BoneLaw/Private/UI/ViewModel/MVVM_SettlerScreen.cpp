// Copyright by MykeUhu

#include "UI/ViewModel/MVVM_SettlerScreen.h"
#include "UI/ViewModel/MVVM_SettlerSlot.h"
#include "Runtime/StoneRosterSubsystem.h"
#include "Core/Character/StoneBaseChar.h"
#include "Engine/World.h"

static UWorld* ResolveWorldFromContext(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	// Try GetWorld() first
	if (UWorld* W = WorldContextObject->GetWorld())
	{
		return W;
	}

	// Fallback: Engine context resolve (safe in game)
	if (GEngine)
	{
		return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	}

	return nullptr;
}

// -------------------------
// Data API for Blueprint
// -------------------------

FGuid UMVVM_SettlerScreen::GetSettlerGuidByIndex(int32 Index) const
{
	return CachedSettlerGuids.IsValidIndex(Index) ? CachedSettlerGuids[Index] : FGuid();
}

FString UMVVM_SettlerScreen::GetSettlerNameByGuid(const FGuid& SettlerId) const
{
	if (!CachedRoster.IsValid())
	{
		return TEXT("(No Roster)");
	}

	const FSavedSettler Info = CachedRoster->GetSettlerInfo(SettlerId);
	return Info.DisplayName;
}

AStoneBaseChar* UMVVM_SettlerScreen::GetSettlerPawnByGuid(const FGuid& SettlerId)
{
	if (!CachedRoster.IsValid())
	{
		return nullptr;
	}

	return CachedRoster->GetOrSpawnSettlerPawn(SettlerId);
}

void UMVVM_SettlerScreen::SetNumSettlers(int32 InNumSettlers)
{
	UE_MVVM_SET_PROPERTY_VALUE(NumSettlers, InNumSettlers);
}

// -------------------------
// ViewModel Registration
// -------------------------

void UMVVM_SettlerScreen::RequestShowDetails(const FGuid& SettlerGuid, AStoneSettlerChar* SettlerChar)
{
	OnRequestShowDetails.Broadcast(SettlerGuid, SettlerChar);
}

// end Data API

// Roster
void UMVVM_SettlerScreen::BindToRoster(UObject* WorldContextObject)
{
	CachedWorldContextObject = WorldContextObject;

	UWorld* World = ResolveWorldFromContext(WorldContextObject);
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerScreen] BindToRoster failed: invalid WorldContext."));
		return;
	}

	UStoneRosterSubsystem* Roster = World->GetSubsystem<UStoneRosterSubsystem>();
	if (!Roster)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerScreen] BindToRoster failed: StoneRosterSubsystem missing."));
		return;
	}

	// Unbind old if any
	if (CachedRoster.IsValid())
	{
		CachedRoster->OnRosterChanged.RemoveDynamic(this, &UMVVM_SettlerScreen::HandleRosterChanged);
	}

	CachedRoster = Roster;
	Roster->OnRosterChanged.AddDynamic(this, &UMVVM_SettlerScreen::HandleRosterChanged);

	// Initial fill
	RefreshDataFromRoster(WorldContextObject);

	UE_LOG(LogTemp, Log, TEXT("[SettlerScreen] Bound to roster changes + initial refresh done."));
}

void UMVVM_SettlerScreen::RefreshDataFromRoster(UObject* WorldContextObject)
{
	UWorld* World = ResolveWorldFromContext(WorldContextObject);
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerScreen] RefreshDataFromRoster failed: invalid WorldContext."));
		return;
	}

	UStoneRosterSubsystem* Roster = World->GetSubsystem<UStoneRosterSubsystem>();
	if (!Roster)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SettlerScreen] RefreshDataFromRoster failed: StoneRosterSubsystem missing."));
		return;
	}

	// Cache settler GUIDs for index-based lookups
	CachedSettlerGuids = Roster->GetAllSettlerIds();
	SetNumSettlers(CachedSettlerGuids.Num());

	// Broadcast event: Blueprint should rebuild grid and register slot ViewModels
	OnSettlerListRebuilt.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[SettlerScreen] Refreshed data from roster. Settlers=%d"), CachedSettlerGuids.Num());
}

void UMVVM_SettlerScreen::HandleRosterChanged()
{
	UObject* Ctx = CachedWorldContextObject.Get();
	RefreshDataFromRoster(Ctx);
}
// end Roster
