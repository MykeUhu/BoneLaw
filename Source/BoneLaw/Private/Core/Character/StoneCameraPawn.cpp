// Copyright by MykeUhu
#include "Core/Character/StoneCameraPawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"

AStoneCameraPawn::AStoneCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// SpectatorPawn already has FloatingPawnMovement component
	// We just configure it
	if (UFloatingPawnMovement* Movement = FindComponentByClass<UFloatingPawnMovement>())
	{
		Movement->MaxSpeed = BaseMoveSpeed;
		Movement->Acceleration = BaseMoveSpeed * 5.f;
		Movement->Deceleration = BaseMoveSpeed * 5.f;
	}

	// Kill DefaultPawn legacy bindings
	bAddDefaultMovementBindings = false;
	
	// Use controller rotation for FPS-like look
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
}

void AStoneCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	// Ensure mouse is captured and cursor hidden for FPS-like mouse look
	// (PlayerController will handle input mode transitions)
}

void AStoneCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update movement speed if BaseMoveSpeed changes at runtime
	if (UFloatingPawnMovement* Movement = Cast<UFloatingPawnMovement>(GetMovementComponent()))
	{
		Movement->MaxSpeed = BaseMoveSpeed;
	}
}

void AStoneCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Input bindings will be set up in PlayerController
	// This pawn does NOT bind input directly (SSOT principle: PC owns input)
}
