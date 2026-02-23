// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "StonePlayerController.generated.h"

class UStoneAbilitySystemComponent;
class UInputMappingContext;
class UInputAction;
class UStoneInputConfig;
struct FInputActionValue;
struct FInputActionInstance;

UCLASS()
class BONELAW_API AStonePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AStonePlayerController();
	virtual void PlayerTick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY()
	TObjectPtr<UStoneAbilitySystemComponent> StoneAbilitySystemComponent;

	UStoneAbilitySystemComponent* GetASC();
	
	// ==========================================
	// CAMERA / COMMANDER WORKFLOW
	// ==========================================

	// Camera State Management (GameplayTag-based SSOT)
	UFUNCTION(BlueprintPure, Category="Stone|Camera")
	FGameplayTag GetCameraState() const { return CurrentCameraState; }

	UFUNCTION(BlueprintCallable, Category="Stone|Camera")
	void SetCameraState(FGameplayTag NewState);

	// Build Placement
	UFUNCTION(BlueprintCallable, Category="Stone|Build")
	void StartBuildPlacement(FGameplayTag BuildRecipeTag);

	UFUNCTION(BlueprintCallable, Category="Stone|Build")
	void CancelBuildPlacement();

	UFUNCTION(BlueprintCallable, Category="Stone|Build")
	void PlaceBuilding();

	// Follow / Observe Settler
	UFUNCTION(BlueprintCallable, Category="Stone|Camera")
	void FollowSettler(AActor* Settler);

	UFUNCTION(BlueprintCallable, Category="Stone|Camera")
	void StopFollowing();

	// Blueprint event: Called when build recipe selected from radial menu
	UFUNCTION(BlueprintCallable, Category="Stone|Build")
	void OnRadialRecipeSelected(FGameplayTag RecipeTag);

protected:
	// ==========================================
	// ENHANCED INPUT
	// ==========================================

	// Input Mapping Contexts
	UPROPERTY(EditDefaultsOnly, Category="Stone|Input|Cam")
	TObjectPtr<UInputMappingContext> IMC_Cam;

	UPROPERTY(EditDefaultsOnly, Category="Stone|Input")
	TObjectPtr<UInputMappingContext> IMC_Build;

	// Input Actions (raw actions, not tagged)
	UPROPERTY(EditDefaultsOnly, Category="Stone|Input|Cam")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, Category="Stone|Input|Cam")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, Category="Stone|Input|Cam")
	TObjectPtr<UInputAction> IA_Ascend;

	UPROPERTY(EditDefaultsOnly, Category="Stone|Input|Cam")
	TObjectPtr<UInputAction> IA_Descend;

	UPROPERTY(EditDefaultsOnly, Category="Stone|Input|Cam")
	TObjectPtr<UInputAction> IA_ToggleBuildMenu;

	UPROPERTY(EditDefaultsOnly, Category="Stone|Input|Build")
	TObjectPtr<UInputAction> IA_RotateGhost;

	UPROPERTY(EditDefaultsOnly, Category="Stone|Input|Build")
	TObjectPtr<UInputAction> IA_AdjustGhostHeight;

	UPROPERTY(EditDefaultsOnly, Category="Stone|Input|Build")
	TObjectPtr<UInputAction> IA_Place;

	UPROPERTY(EditDefaultsOnly, Category="Stone|Input|Build")
	TObjectPtr<UInputAction> IA_Cancel;

	UPROPERTY(EditDefaultsOnly, Category="Stone|Input|Build")
	TObjectPtr<UInputAction> IA_Escape;

	// Input Config (for tagged ability-style inputs)
	UPROPERTY(EditDefaultsOnly, Category="Stone|Input")
	TObjectPtr<UStoneInputConfig> InputConfig;

	// Input Binding Functions
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	void Input_Ascend(const FInputActionValue& InputActionValue);
	void Input_Descend(const FInputActionValue& InputActionValue);
	void Input_ToggleBuildMenu();
	void Input_RotateGhost(const FInputActionValue& InputActionValue);
	void Input_AdjustGhostHeight(const FInputActionValue& InputActionValue);
	void Input_Place();
	void Input_Cancel();
	void Input_Escape();

	void AddInputMappingContext(UInputMappingContext* IMC, int32 Priority = 0);
	void RemoveInputMappingContext(UInputMappingContext* IMC);

	// ==========================================
	// CAMERA STATE
	// ==========================================

	// Current camera mode (SSOT)
	UPROPERTY(BlueprintReadOnly, Category="Stone|Camera", meta=(AllowPrivateAccess="true"))
	FGameplayTag CurrentCameraState;

	// Follow target (weak pointer, for Follow mode)
	UPROPERTY()
	TWeakObjectPtr<AActor> FollowTarget;

	// Build ghost reference (for BuildPlacement mode)
	UPROPERTY()
	TObjectPtr<AActor> BuildGhostActor;

	// Build ghost rotation/height accumulators
	float GhostRotationYaw = 0.f;
	float GhostHeightOffset = 0.f;

	// Update camera based on current state
	void UpdateCameraState(float DeltaTime);
	void UpdateFollowCamera(float DeltaTime);
	void UpdateBuildGhost(float DeltaTime);

	// Tracing for build placement (center-screen crosshair trace)
	bool TraceBuildLocation(FVector& OutLocation, FVector& OutNormal) const;

	// Input mode management
	void SetInputModeFreeCam();
	void SetInputModeUI();
	void SetInputModeBuildPlacement();
	
	virtual void OnRep_PlayerState() override;

private:
	void TryInitOverlay();

	UPROPERTY(Transient)
	bool bOverlayInitialized = false;
};
