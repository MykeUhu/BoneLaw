// Copyright by MykeUhu

#include "UI/Widget/Event/StoneActionPanelWidget.h"

#include "Components/ProgressBar.h"

#include "Data/StoneActionDefinitionData.h"

#include "Runtime/StoneRunSubsystem.h"

#include "UI/CustomElements/StoneCustomButton.h"
#include "UI/CustomElements/StoneCustomTextBlock.h"
#include "UI/WidgetController/StoneOverlayWidgetController.h"
#include "UI/WidgetController/StoneWidgetController.h"

void UStoneActionPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_StartAction)
	{
		Btn_StartAction->OnClicked.RemoveDynamic(this, &UStoneActionPanelWidget::HandleStartActionClicked);
		Btn_StartAction->OnClicked.AddDynamic(this, &UStoneActionPanelWidget::HandleStartActionClicked);
	}

	RefreshEnabledState();
	RefreshProgressVisual();
	RefreshInfoVisual();
}

void UStoneActionPanelWidget::NativeDestruct()
{
	UnbindAll();

	if (Btn_StartAction)
	{
		Btn_StartAction->OnClicked.RemoveDynamic(this, &UStoneActionPanelWidget::HandleStartActionClicked);
	}

	Super::NativeDestruct();
}

void UStoneActionPanelWidget::SetOverlayController(UStoneWidgetController* InController)
{
	if (OverlayController == InController) return;

	UnbindAll();
	OverlayController = InController;

	

	UStoneOverlayWidgetController* OC = GetOverlayController();
	if (!OC)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[StoneUI][ActionPanel] SetOverlayController: not UStoneOverlayWidgetController (got %s)"),
			InController ? *InController->GetClass()->GetName() : TEXT("null"));

		RefreshEnabledState();
		RefreshProgressVisual();
		RefreshInfoVisual();
		return;
	}

	BindAll();

	

	RefreshEnabledState();
	RefreshProgressVisual();
	RefreshInfoVisual();
}

void UStoneActionPanelWidget::BindAll()
{
	UStoneOverlayWidgetController* OC = GetOverlayController();
	if (!OC) return;

	OC->OnOverlaySnapshotChanged.RemoveDynamic(this, &UStoneActionPanelWidget::HandleOverlaySnapshotChanged);
	OC->OnOverlaySnapshotChanged.AddDynamic(this, &UStoneActionPanelWidget::HandleOverlaySnapshotChanged);

	OC->OnOverlayEventChanged.RemoveDynamic(this, &UStoneActionPanelWidget::HandleOverlayEventChanged);
	OC->OnOverlayEventChanged.AddDynamic(this, &UStoneActionPanelWidget::HandleOverlayEventChanged);

	
}

void UStoneActionPanelWidget::UnbindAll()
{
	// IMPORTANT: remove from overlay controller via cast, never through base UObject
	if (UStoneOverlayWidgetController* OC = GetOverlayController())
	{
		OC->OnOverlaySnapshotChanged.RemoveDynamic(this, &UStoneActionPanelWidget::HandleOverlaySnapshotChanged);
		OC->OnOverlayEventChanged.RemoveDynamic(this, &UStoneActionPanelWidget::HandleOverlayEventChanged);
	}


	OverlayController = nullptr;
}

void UStoneActionPanelWidget::HandleOverlaySnapshotChanged(const FStoneSnapshot& /*Snapshot*/)
{
	RefreshEnabledState();
	RefreshProgressVisual();
	RefreshInfoVisual();
}

void UStoneActionPanelWidget::HandleOverlayEventChanged(const UStoneEventData* /*Event*/)
{
	RefreshEnabledState();
	RefreshInfoVisual();
}

void UStoneActionPanelWidget::HandleActionStateChanged()
{
	RefreshEnabledState();
	RefreshProgressVisual();
	RefreshInfoVisual();
}

void UStoneActionPanelWidget::HandleActionProgressChanged(float /*Progress01*/)
{
	RefreshProgressVisual();
	RefreshInfoVisual();
}

void UStoneActionPanelWidget::RefreshEnabledState()
{
	if (!Btn_StartAction) return;

	UStoneOverlayWidgetController* OC = GetOverlayController();
	UStoneRunSubsystem* Run = OC ? OC->GetRunSubsystem() : nullptr;

	const bool bHasOpenEvent = Run ? Run->HasOpenEvent() : false;
	const bool bHasRealtimeRunAction = Run ? Run->IsAnyRealtimeActionActive() : false;

	const bool bHasActionDef = (ActionToStart != nullptr);


	// Optional: update button text if you don't style it in BP
	if (ActionToStart)
	{
		const FText NewText = ActionToStart->DisplayName.IsEmpty()
			? FText::FromString(ActionToStart->GetName())
			: ActionToStart->DisplayName;

		Btn_StartAction->SetButtonText(NewText);
	}
	
}

void UStoneActionPanelWidget::RefreshProgressVisual()
{
	if (!PB_ActionProgress) return;

	

}

void UStoneActionPanelWidget::RefreshInfoVisual()
{
	// If the BP doesn't have these TextBlocks, we just do nothing (BindWidgetOptional).
	if (!TB_ActionTitle && !TB_ActionSubtitle && !TB_ActionETA && !TB_ActionStatus)
	{
		return;
	}

	

	// Title
	if (TB_ActionTitle)
	{
	

	}

	// Subtitle (phase)
	if (TB_ActionSubtitle)
	{
		
	}

	// ETA
	if (TB_ActionETA)
	{
		
	}

	// Status (paused by event/pause)
	if (TB_ActionStatus)
	{
		/*if (bRunning && ActionSubsystem->IsPausedByGameState())
		{
			TB_ActionStatus->SetText(FText::FromString(TEXT("Paused")));
		}
		else
		{
			TB_ActionStatus->SetText(FText::GetEmpty());
		}
		*/
	}
}

void UStoneActionPanelWidget::HandleStartActionClicked()
{
	
	if (!ActionToStart)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneUI][ActionPanel] StartAction clicked but ActionToStart is null (set in BP defaults)"));
		return;
	}

	UStoneOverlayWidgetController* OC = GetOverlayController();
	UStoneRunSubsystem* Run = OC ? OC->GetRunSubsystem() : nullptr;

	// Guard again (anti-spam)
	if (Run && (Run->HasOpenEvent() || Run->IsAnyRealtimeActionActive()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneUI][ActionPanel] StartAction blocked: OpenEvent=%d RealtimeRun=%d"),
			Run->HasOpenEvent(), Run->IsAnyRealtimeActionActive());

		RefreshEnabledState();
		RefreshProgressVisual();
		RefreshInfoVisual();
		return;
	}
	

	


	RefreshEnabledState();
	RefreshProgressVisual();
	RefreshInfoVisual();
}

UStoneOverlayWidgetController* UStoneActionPanelWidget::GetOverlayController() const
{
	return Cast<UStoneOverlayWidgetController>(OverlayController);
}
