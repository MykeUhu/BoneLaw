// Copyright by MykeUhu

#include "UI/Widget/Event/StoneActionPanelWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

#include "Data/StoneActionDefinitionData.h"
#include "Data/StoneEventData.h"

#include "Runtime/StoneActionSubsystem.h"
#include "Runtime/StoneRunSubsystem.h"

#include "UI/CustomElements/StoneCustomButton.h"
#include "UI/CustomElements/StoneCustomTextBlock.h"
#include "UI/WidgetController/StoneOverlayWidgetController.h"
#include "UI/WidgetController/StoneWidgetController.h"

void UStoneActionPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Cache Action subsystem once (world subsystem)
	ActionSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UStoneActionSubsystem>() : nullptr;

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

	// recache (safe)
	ActionSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UStoneActionSubsystem>() : nullptr;

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

	UE_LOG(LogTemp, Display, TEXT("[StoneUI][ActionPanel] Controller set. HasRun=%d HasAction=%d ActionDef=%s"),
		OC->GetRunSubsystem() != nullptr,
		ActionSubsystem != nullptr,
		ActionToStart ? *ActionToStart->GetName() : TEXT("<null>"));

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

	if (ActionSubsystem)
	{
		ActionSubsystem->OnActionStateChanged.RemoveDynamic(this, &UStoneActionPanelWidget::HandleActionStateChanged);
		ActionSubsystem->OnActionStateChanged.AddDynamic(this, &UStoneActionPanelWidget::HandleActionStateChanged);

		ActionSubsystem->OnActionProgressChanged.RemoveDynamic(this, &UStoneActionPanelWidget::HandleActionProgressChanged);
		ActionSubsystem->OnActionProgressChanged.AddDynamic(this, &UStoneActionPanelWidget::HandleActionProgressChanged);
	}
}

void UStoneActionPanelWidget::UnbindAll()
{
	// IMPORTANT: remove from overlay controller via cast, never through base UObject
	if (UStoneOverlayWidgetController* OC = GetOverlayController())
	{
		OC->OnOverlaySnapshotChanged.RemoveDynamic(this, &UStoneActionPanelWidget::HandleOverlaySnapshotChanged);
		OC->OnOverlayEventChanged.RemoveDynamic(this, &UStoneActionPanelWidget::HandleOverlayEventChanged);
	}

	if (ActionSubsystem)
	{
		ActionSubsystem->OnActionStateChanged.RemoveDynamic(this, &UStoneActionPanelWidget::HandleActionStateChanged);
		ActionSubsystem->OnActionProgressChanged.RemoveDynamic(this, &UStoneActionPanelWidget::HandleActionProgressChanged);
	}

	OverlayController = nullptr;
	ActionSubsystem = nullptr;
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
	const bool bActionRunning = ActionSubsystem ? ActionSubsystem->IsActionRunning() : false;

	const bool bHasActionDef = (ActionToStart != nullptr);
	const bool bCanStart = bHasActionDef && !bHasOpenEvent && !bHasRealtimeRunAction && !bActionRunning;

	Btn_StartAction->SetIsEnabled(bCanStart);

	// Optional: update button text if you don't style it in BP
	if (ActionToStart)
	{
		const FText NewText = ActionToStart->DisplayName.IsEmpty()
			? FText::FromString(ActionToStart->GetName())
			: ActionToStart->DisplayName;

		Btn_StartAction->SetButtonText(NewText);
	}

	UE_LOG(LogTemp, Display, TEXT("[StoneUI][ActionPanel] Enabled=%d | HasDef=%d OpenEvent=%d RealtimeRun=%d ActionRunning=%d"),
		bCanStart, bHasActionDef, bHasOpenEvent, bHasRealtimeRunAction, bActionRunning);
}

void UStoneActionPanelWidget::RefreshProgressVisual()
{
	if (!PB_ActionProgress) return;

	float Progress01 = -1.f;
	if (ActionSubsystem && ActionSubsystem->IsActionRunning())
	{
		Progress01 = ActionSubsystem->GetActionProgress01();
	}

	if (Progress01 < 0.f)
	{
		PB_ActionProgress->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	PB_ActionProgress->SetVisibility(ESlateVisibility::Visible);
	PB_ActionProgress->SetPercent(FMath::Clamp(Progress01, 0.f, 1.f));
}

void UStoneActionPanelWidget::RefreshInfoVisual()
{
	// If the BP doesn't have these TextBlocks, we just do nothing (BindWidgetOptional).
	if (!TB_ActionTitle && !TB_ActionSubtitle && !TB_ActionETA && !TB_ActionStatus)
	{
		return;
	}

	const bool bRunning = (ActionSubsystem && ActionSubsystem->IsActionRunning());

	// Title
	if (TB_ActionTitle)
	{
		if (bRunning)
		{
			const FText Title = ActionSubsystem->GetActionTitleText();
			TB_ActionTitle->SetText(Title.IsEmpty() ? FText::FromString(TEXT("Action")) : Title);
		}
		else
		{
			// Show next selectable action as a preview (optional)
			if (ActionToStart)
			{
				const FText Preview = ActionToStart->DisplayName.IsEmpty()
					? FText::FromString(ActionToStart->GetName())
					: ActionToStart->DisplayName;

				TB_ActionTitle->SetText(Preview);
			}
			else
			{
				TB_ActionTitle->SetText(FText::FromString(TEXT("Actions")));
			}
		}
	}

	// Subtitle (phase)
	if (TB_ActionSubtitle)
	{
		TB_ActionSubtitle->SetText(bRunning ? ActionSubsystem->GetPhaseText() : FText::GetEmpty());
	}

	// ETA
	if (TB_ActionETA)
	{
		if (bRunning)
		{
			const float Remaining = ActionSubsystem->GetRemainingSeconds();
			TB_ActionETA->SetText(FText::FromString(FString::Printf(TEXT("ETA: %.0fs"), Remaining)));
		}
		else
		{
			TB_ActionETA->SetText(FText::GetEmpty());
		}
	}

	// Status (paused by event/pause)
	if (TB_ActionStatus)
	{
		if (bRunning && ActionSubsystem->IsPausedByGameState())
		{
			TB_ActionStatus->SetText(FText::FromString(TEXT("Paused")));
		}
		else
		{
			TB_ActionStatus->SetText(FText::GetEmpty());
		}
	}
}

void UStoneActionPanelWidget::HandleStartActionClicked()
{
	if (!ActionSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneUI][ActionPanel] StartAction clicked but ActionSubsystem null"));
		return;
	}
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
	if (ActionSubsystem->IsActionRunning())
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneUI][ActionPanel] StartAction blocked: Action already running"));
		RefreshEnabledState();
		RefreshInfoVisual();
		return;
	}

	const bool bStarted = ActionSubsystem->StartAction(ActionToStart);
	UE_LOG(LogTemp, Display, TEXT("[StoneUI][ActionPanel] StartAction(%s) -> %d"), *ActionToStart->GetName(), bStarted);

	RefreshEnabledState();
	RefreshProgressVisual();
	RefreshInfoVisual();
}

UStoneOverlayWidgetController* UStoneActionPanelWidget::GetOverlayController() const
{
	return Cast<UStoneOverlayWidgetController>(OverlayController);
}
