// Copyright by MykeUhu

#include "UI/Widget/Event/StoneActionPanelWidget.h"

#include "Components/ProgressBar.h"

#include "Data/StoneActionDefinitionData.h"
#include "Data/StoneEventData.h"

#include "Runtime/StoneActionSubsystem.h"
#include "Runtime/StoneRunSubsystem.h"

#include "UI/CustomElements/StoneCustomButton.h"
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
		return;
	}

	BindAll();

	UE_LOG(LogTemp, Display, TEXT("[StoneUI][ActionPanel] Controller set. HasRun=%d HasAction=%d ActionDef=%s"),
		OC->GetRunSubsystem() != nullptr,
		ActionSubsystem != nullptr,
		ActionToStart ? *ActionToStart->GetName() : TEXT("<null>"));

	RefreshEnabledState();
	RefreshProgressVisual();
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
}

void UStoneActionPanelWidget::HandleOverlayEventChanged(const UStoneEventData* /*Event*/)
{
	RefreshEnabledState();
}

void UStoneActionPanelWidget::HandleActionStateChanged()
{
	RefreshEnabledState();
	RefreshProgressVisual();
}

void UStoneActionPanelWidget::HandleActionProgressChanged(float /*Progress01*/)
{
	RefreshProgressVisual();
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
		return;
	}
	if (ActionSubsystem->IsActionRunning())
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneUI][ActionPanel] StartAction blocked: Action already running"));
		RefreshEnabledState();
		return;
	}

	const bool bStarted = ActionSubsystem->StartAction(ActionToStart);
	UE_LOG(LogTemp, Display, TEXT("[StoneUI][ActionPanel] StartAction(%s) -> %d"), *ActionToStart->GetName(), bStarted);

	RefreshEnabledState();
	RefreshProgressVisual();
}

UStoneOverlayWidgetController* UStoneActionPanelWidget::GetOverlayController() const
{
	return Cast<UStoneOverlayWidgetController>(OverlayController);
}
