#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/StoneWidgetController.h"
#include "Runtime/StoneRunSubsystem.h" // FStoneSnapshot, FStoneChoiceResolved, delegates
#include "StoneOverlayWidgetController.generated.h"

class UStoneEventData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneOverlaySnapshotSig, const FStoneSnapshot&, Snapshot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneOverlayEventSig, const UStoneEventData*, Event);

UCLASS(BlueprintType)
class BONELAW_API UStoneOverlayWidgetController : public UStoneWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValues() override;

	UPROPERTY(BlueprintAssignable, Category="Stone|UI")
	FStoneOverlaySnapshotSig OnOverlaySnapshotChanged;

	UPROPERTY(BlueprintAssignable, Category="Stone|UI")
	FStoneOverlayEventSig OnOverlayEventChanged;

	UFUNCTION(BlueprintPure, Category="Stone|UI")
	UStoneRunSubsystem* GetRunSubsystem() const { return RunSubsystem; }

	UFUNCTION(BlueprintCallable, Category="Stone|UI")
	void GetResolvedChoices(TArray<FStoneChoiceResolved>& OutChoices) const;

private:
	UFUNCTION()
	void HandleSnapshotChanged(const FStoneSnapshot& Snapshot);

	UFUNCTION()
	void HandleEventChanged(const UStoneEventData* Event);
};
