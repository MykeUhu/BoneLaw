// Copyright by MykeUhu
// Following Aura pattern - typed getters inherited from base

#include "UI/WidgetController/StoneOverlayWidgetController.h"

#include "Data/StoneEventData.h"
#include "Kismet/GameplayStatics.h"

void UStoneOverlayWidgetController::BroadcastInitialValues()
{
	UStoneRunSubsystem* RunSS = GetRunSubsystem();
	if (!RunSS) return;

	HandleSnapshotChanged(RunSS->GetSnapshot());
	HandleEventChanged(RunSS->GetCurrentEvent());
}

void UStoneOverlayWidgetController::BindCallbacksToDependencies()
{
	UStoneRunSubsystem* RunSS = GetRunSubsystem();
	if (!RunSS) return;

	RunSS->OnSnapshotChanged.AddDynamic(this, &UStoneOverlayWidgetController::HandleSnapshotChanged);
	RunSS->OnEventChanged.AddDynamic(this, &UStoneOverlayWidgetController::HandleEventChanged);
}

UStoneRunSubsystem* UStoneOverlayWidgetController::GetRunSubsystem() const
{
	if (CachedRunSubsystem == nullptr)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(PlayerController, EGetWorldErrorMode::LogAndReturnNull))
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				CachedRunSubsystem = GI->GetSubsystem<UStoneRunSubsystem>();
			}
		}
	}
	return CachedRunSubsystem;
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
	UStoneRunSubsystem* RunSS = GetRunSubsystem();
	if (!RunSS) return;

	RunSS->GetResolvedChoices(OutChoices);
}
