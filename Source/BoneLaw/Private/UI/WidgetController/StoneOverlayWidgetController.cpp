#include "UI/WidgetController/StoneOverlayWidgetController.h"

#include "Data/StoneEventData.h"

void UStoneOverlayWidgetController::BindCallbacksToDependencies()
{
	if (!RunSubsystem) return;

	RunSubsystem->OnSnapshotChanged.AddDynamic(this, &UStoneOverlayWidgetController::HandleSnapshotChanged);
	RunSubsystem->OnEventChanged.AddDynamic(this, &UStoneOverlayWidgetController::HandleEventChanged);
}

void UStoneOverlayWidgetController::BroadcastInitialValues()
{
	if (!RunSubsystem) return;

	HandleSnapshotChanged(RunSubsystem->GetSnapshot());
	HandleEventChanged(RunSubsystem->GetCurrentEvent());
}

void UStoneOverlayWidgetController::HandleSnapshotChanged(const FStoneSnapshot& Snapshot)
{
	OnOverlaySnapshotChanged.Broadcast(Snapshot);
}

void UStoneOverlayWidgetController::HandleEventChanged(const UStoneEventData* Event)
{
	OnOverlayEventChanged.Broadcast(Event);
}

void UStoneOverlayWidgetController::GetResolvedChoices(TArray<FStoneChoiceResolved>& OutChoices) const
{
	OutChoices.Reset();
	if (!RunSubsystem) return;

	RunSubsystem->GetResolvedChoices(OutChoices);
}
