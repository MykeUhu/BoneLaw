#pragma once

#include "CoreMinimal.h"
#include "Data/StoneTypes.h"
#include "UObject/Object.h"
#include "StoneWorldlineWeightPolicy.generated.h"

class UStoneEventData;
class UStoneWorldlineDirector;

UCLASS()
class BONELAW_API UStoneWorldlineWeightPolicy : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(const UStoneWorldlineDirector* InWorldline);

	// Returns multiplier in [0.25 .. 3.0] (clamped)
	float ComputeMultiplier(const UStoneEventData* Event, const FStoneSnapshot& Snapshot) const;

private:
	UPROPERTY()
	TObjectPtr<const UStoneWorldlineDirector> Worldline;

	float Axis01(float AxisValueAbs100) const; // 0..1 from |axis|/100

	// Helpers
	float ApplyMercyRuthless(const UStoneEventData* Event, const FStoneSnapshot& Snapshot, float Mul) const;
	float ApplyTraditionInnovation(const UStoneEventData* Event, const FStoneSnapshot& Snapshot, float Mul) const;
	float ApplyXeno(const UStoneEventData* Event, const FStoneSnapshot& Snapshot, float Mul) const;
	float ApplyTaboo(const UStoneEventData* Event, const FStoneSnapshot& Snapshot, float Mul) const;
};
