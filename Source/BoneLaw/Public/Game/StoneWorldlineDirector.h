#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "StoneWorldlineDirector.generated.h"

struct FStoneTimeState;
struct FStoneScheduledEvent;
class UStoneAttributeRegistry;
class UAbilitySystemComponent;

UENUM()
enum class EStoneWorldAxis : uint8
{
	MercyRuthless,
	TraditionInnovation,
	CollectiveIndividual,
	SpiritualPractical,
	XenoOpenXenoFear,
	TabooLooseTabooStrict
};

USTRUCT()
struct FStoneWorldAxisState
{
	GENERATED_BODY()

	// -100..+100 (negative is left pole, positive is right pole)
	UPROPERTY()
	float Value = 0.f;

	UPROPERTY()
	int32 LastMilestoneLevel = 0; // to avoid re-triggering
};

UCLASS()
class BONELAW_API UStoneWorldlineDirector : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UAbilitySystemComponent* InASC, const UStoneAttributeRegistry* InRegistry);

	// Called after every choice/time step
	void UpdateWorldline(FGameplayTagContainer& RunTags, FStoneTimeState& Time, TArray<FStoneScheduledEvent>& OutNewSchedules);

	// For debug/telemetry
	float GetAxisValue(EStoneWorldAxis Axis) const;

private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	TObjectPtr<const UStoneAttributeRegistry> Registry;

	UPROPERTY()
	TMap<EStoneWorldAxis, FStoneWorldAxisState> Axes;

	// Helpers
	float ReadCultureValue_MercyRuthless() const;
	float ReadCultureValue_TraditionInnovation() const;
	float ReadCultureValue_CollectiveIndividual() const;
	float ReadCultureValue_SpiritualPractical() const;
	float ReadCultureValue_Xeno() const;
	float ReadCultureValue_Taboo() const;

	void ApplyAxisTags(EStoneWorldAxis Axis, float Value, FGameplayTagContainer& RunTags);
	void ApplyMilestones(EStoneWorldAxis Axis, float Value, FGameplayTagContainer& RunTags, FStoneTimeState& Time, TArray<FStoneScheduledEvent>& OutSchedules);
};
