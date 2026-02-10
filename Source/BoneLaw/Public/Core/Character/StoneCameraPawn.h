// Copyright by MykeUhu
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "StoneCameraPawn.generated.h"

/**
 * FreeCam / Commander Camera Pawn
 * 
 * Flying spectator camera with FPS-style mouse look (no RMB required).
 * - WASD for planar movement
 * - Q/E for vertical movement (descend/ascend)
 * - Mouse look for yaw/pitch (FPS-like, mouse captured)
 * - Supports Enhanced Input (IMC_Cam)
 * 
 * This pawn is the default for Commander/Colony-style gameplay.
 * Player does NOT directly control settlers; this camera observes and commands.
 */
UCLASS()
class BONELAW_API AStoneCameraPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	AStoneCameraPawn();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	// Movement speed (units/sec)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stone|Camera")
	float BaseMoveSpeed = 1000.f;

	// Look sensitivity (degrees/sec per input unit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stone|Camera")
	float LookSensitivity = 45.f;

	// Vertical movement speed (Q/E)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stone|Camera")
	float VerticalSpeed = 600.f;

protected:
	virtual void BeginPlay() override;
};
