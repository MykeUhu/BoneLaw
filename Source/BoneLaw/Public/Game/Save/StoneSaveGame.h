#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "Data/StoneTypes.h"
#include "StoneSaveGame.generated.h"

UCLASS()
class BONELAW_API UStoneSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 SaveVersion = 1;

	UPROPERTY()
	FStoneTimeState Time;

	UPROPERTY()
	FGameplayTagContainer RunTags;

	UPROPERTY()
	FGameplayTag FocusTag;

	UPROPERTY()
	FName CurrentEventId;
	
	UPROPERTY()
	TArray<FName> EventPoolIds;
	
	UPROPERTY()
	TArray<FStoneScheduledEvent> ScheduledQueue;

	UPROPERTY()
	int32 RNGSeed = 1337;

	// Simple attribute snapshots (stable & explicit)
	UPROPERTY()
	TMap<FName, float> AttributeValues;
	
	UPROPERTY()
	TArray<FName> ActivePackIds;

	UPROPERTY()
	TMap<FName, int32> EventSeenCountByPack; // PackId -> count

	UPROPERTY()
	TSet<FName> SeenEventIds; // prevents repeats if you want
	
};
