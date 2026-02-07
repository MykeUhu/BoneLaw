// Copyright by MykeUhu
// Following Aura/Druid Mechanics pattern exactly

#include "UI/WidgetController/StoneWidgetController.h"

#include "Core/StonePlayerController.h"
#include "Core/StonePlayerState.h"
#include "AbilitySystem/StoneAbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"

void UStoneWidgetController::SetWidgetControllerParams(const FStoneWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void UStoneWidgetController::BroadcastInitialValues()
{
}

void UStoneWidgetController::BindCallbacksToDependencies()
{
}

AStonePlayerController* UStoneWidgetController::GetStonePC()
{
	if (StonePlayerController == nullptr)
	{
		StonePlayerController = Cast<AStonePlayerController>(PlayerController);
	}
	return StonePlayerController;
}

AStonePlayerState* UStoneWidgetController::GetStonePS()
{
	if (StonePlayerState == nullptr)
	{
		StonePlayerState = Cast<AStonePlayerState>(PlayerState);
	}
	return StonePlayerState;
}

UStoneAbilitySystemComponent* UStoneWidgetController::GetStoneASC()
{
	if (StoneAbilitySystemComponent == nullptr)
	{
		StoneAbilitySystemComponent = Cast<UStoneAbilitySystemComponent>(AbilitySystemComponent);
	}
	return StoneAbilitySystemComponent;
}

UStoneAttributeSet* UStoneWidgetController::GetStoneAS()
{
	if (StoneAttributeSet == nullptr)
	{
		StoneAttributeSet = Cast<UStoneAttributeSet>(AttributeSet);
	}
	return StoneAttributeSet;
}
