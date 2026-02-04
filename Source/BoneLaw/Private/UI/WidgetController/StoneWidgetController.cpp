#include "UI/WidgetController/StoneWidgetController.h"

void UStoneWidgetController::SetWidgetControllerParams(const FStoneWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
	RunSubsystem = WCParams.RunSubsystem;
}

void UStoneWidgetController::BroadcastInitialValues()
{
	// Base implementation does nothing.
	// Override in derived controllers to push initial UI state.
}

void UStoneWidgetController::BindCallbacksToDependencies()
{
	// Base implementation does nothing.
	// Override in derived controllers to push initial UI state.
}
