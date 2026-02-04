#pragma once

#include "CoreMinimal.h"
#include "Data/StoneRunTrace.h"
#include "UObject/Object.h"
#include "StoneRunTraceBuffer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneTraceAdded, const FStoneTraceEntry&, Entry);

UCLASS()
class BONELAW_API UStoneRunTraceBuffer : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="Stone|Trace")
	FStoneTraceAdded OnTraceAdded;

	void Init(int32 InMaxEntries = 512);
	void Add(const FStoneTraceEntry& Entry);

	UFUNCTION(BlueprintCallable, Category="Stone|Trace")
	void GetAll(TArray<FStoneTraceEntry>& Out) const;

	UFUNCTION(BlueprintCallable, Category="Stone|Trace")
	void Clear();

private:
	UPROPERTY()
	int32 MaxEntries = 512;

	UPROPERTY()
	TArray<FStoneTraceEntry> Entries;
};
