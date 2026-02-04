#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/AssetManager.h"
#include "StonePackLibrary.generated.h"

class UStoneEventPackData;

UCLASS()
class BONELAW_API UStonePackLibrary : public UObject
{
	GENERATED_BODY()

public:
	void Initialize();

	bool PreloadAll(bool bSynchronous);
	bool PreloadByIds(const TArray<FName>& PackIds, bool bSynchronous);

	UStoneEventPackData* GetPack(FName PackId) const;
	bool HasPack(FName PackId) const;
	void GetAllKnownPackIds(TArray<FName>& Out) const;

private:
	void BuildPathCache();
	bool PreloadPrimaryAssetIds(const TArray<FPrimaryAssetId>& Ids, bool bSynchronous);

	UPROPERTY()
	TMap<FName, FPrimaryAssetId> PackIdToPrimaryId;

	UPROPERTY()
	TMap<FName, TObjectPtr<UStoneEventPackData>> LoadedPacks;

	TSharedPtr<FStreamableHandle> ActiveHandle;
	bool bInitialized = false;
};
