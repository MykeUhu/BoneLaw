// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "StonePlayerController.generated.h"

class UStoneAbilitySystemComponent;

UCLASS()
class BONELAW_API AStonePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AStonePlayerController();
	
	virtual void BeginPlay() override;

	/** SSOT: Start-Pack ID hier setzen (z.B. "Pack_Core"). */
	UPROPERTY(EditDefaultsOnly, Category="Stone|Run")
	FName DefaultStartPack = NAME_None;

	/** UI ruft genau diese Funktion auf. */
	UFUNCTION(BlueprintCallable, Category="Stone|Run")
	void StartStoneRun();
	
	UFUNCTION(BlueprintCallable, Category="Stone|Sim")
	void SetSimSpeed(float NewSpeed);

	UFUNCTION(BlueprintPure, Category="Stone|Sim")
	float GetSimSpeed() const { return SimSpeed; }

	// === Actions / Expeditions ===
	// Simple demo action: send the tribe out to explore using a pack (e.g. Pack_Explore_01)
	// and let events appear over time.
	UPROPERTY(EditDefaultsOnly, Category="Stone|Expedition")
	FName DefaultExplorePack = TEXT("Pack_Explore_01");

	UFUNCTION(BlueprintCallable, Category="Stone|Expedition")
	void StartExploreExpedition(float DurationSeconds = 300.f, float MinEventGapSeconds = 20.f, float MaxEventGapSeconds = 60.f, bool bTriggerFirstEventImmediately = false);

	UFUNCTION(BlueprintCallable, Category="Stone|Expedition")
	void StopExpedition(bool bForceReturnEvent = true);

private:
	UPROPERTY()
	TObjectPtr<UStoneAbilitySystemComponent> StoneAbilitySystemComponent;
	
	UStoneAbilitySystemComponent* GetASC();
	
	UPROPERTY()
	float SimSpeed = 1.f;
};
