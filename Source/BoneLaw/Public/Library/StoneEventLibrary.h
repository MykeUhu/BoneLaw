#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/AssetManager.h"
#include "StoneEventLibrary.generated.h"

class UStoneEventData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneEventsPreloaded, bool, bSuccess);

UCLASS()
class BONELAW_API UStoneEventLibrary : public UObject
{
	GENERATED_BODY()

public:
	// Initialize from AssetManager primary assets (StoneEvent)
	void Initialize();

	// Preload a set of events (by EventId/AssetName)
	// If bSynchronous=true, it will block until loaded (recommended for boot / first screen).
	bool PreloadByIds(const TArray<FName>& EventIds, bool bSynchronous);

	// Preload ALL StoneEvent primary assets found by AssetManager scan.
	bool PreloadAll(bool bSynchronous);

	// Get cached event (fast). Returns nullptr if missing/not loaded.
	UStoneEventData* GetEvent(FName EventId) const;

	// Does AssetManager know about this id (even if not loaded yet)?
	bool HasEvent(FName EventId) const;

	// Returns all known EventIds (from AssetManager scan)
	void GetAllKnownEventIds(TArray<FName>& OutEventIds) const;

	// For UI/debug
	UPROPERTY(BlueprintAssignable, Category="Stone|Events")
	FStoneEventsPreloaded OnEventsPreloaded;

private:
	void BuildPathCache();
	bool PreloadPrimaryAssetIds(const TArray<FPrimaryAssetId>& Ids, bool bSynchronous);

	// Cached mapping from EventId -> PrimaryAssetId
	UPROPERTY()
	TMap<FName, FPrimaryAssetId> EventIdToPrimaryId;

	// Cached mapping from EventId -> Loaded object
	UPROPERTY()
	TMap<FName, TObjectPtr<UStoneEventData>> LoadedEvents;

	// Keep handle alive for async loads (optional)
	TSharedPtr<FStreamableHandle> ActiveHandle;

	bool bInitialized = false;
};
