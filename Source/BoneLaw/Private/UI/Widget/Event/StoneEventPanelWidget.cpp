// Copyright by MykeUhu

#include "UI/Widget/Event/StoneEventPanelWidget.h"

#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"

#include "Data/StoneEventData.h"
#include "Runtime/StoneActionSubsystem.h"
#include "Runtime/StoneRunSubsystem.h"

#include "UI/CustomElements/StoneCustomTextBlock.h"
#include "UI/Widget/Event/StoneChoiceButtonWidget.h"
#include "UI/WidgetController/StoneOverlayWidgetController.h"
#include "UI/WidgetController/StoneWidgetController.h" // <— wichtig, damit UStoneWidgetController bekannt ist

void UStoneEventPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// Binding happens in SetOverlayController to avoid double-binding when widgets are reconstructed.
}

void UStoneEventPanelWidget::NativeDestruct()
{
	UnbindAll();
	Super::NativeDestruct();
}

void UStoneEventPanelWidget::SetOverlayController(UStoneWidgetController* InController)
{
	// NOTE: HUD setzt/owned den WidgetController. Wir binden hier nur UI-Delegates.
	if (OverlayController == InController) return;

	UnbindAll();
	OverlayController = InController;

	// Subsystems (World Subsystem) cachen
	ActionSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UStoneActionSubsystem>() : nullptr;

	UStoneOverlayWidgetController* OC = GetOverlayController();
	if (!OC)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[StoneUI][EventPanel] SetOverlayController: not UStoneOverlayWidgetController (got %s)"),
			InController ? *InController->GetClass()->GetName() : TEXT("null"));

		ClearUI();
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	BindAll();
}

void UStoneEventPanelWidget::BindAll()
{
	UStoneOverlayWidgetController* OC = GetOverlayController();
	if (!OC) return;

	// Overlay delegates (dynamic multicast) => AddDynamic / RemoveDynamic
	OC->OnOverlayEventChanged.RemoveDynamic(this, &UStoneEventPanelWidget::HandleOverlayEventChanged);
	OC->OnOverlayEventChanged.AddDynamic(this, &UStoneEventPanelWidget::HandleOverlayEventChanged);

	OC->OnOverlaySnapshotChanged.RemoveDynamic(this, &UStoneEventPanelWidget::HandleOverlaySnapshotChanged);
	OC->OnOverlaySnapshotChanged.AddDynamic(this, &UStoneEventPanelWidget::HandleOverlaySnapshotChanged);

	// Action subsystem delegates (optional)
	if (ActionSubsystem)
	{
		ActionSubsystem->OnActionStateChanged.RemoveDynamic(this, &UStoneEventPanelWidget::HandleActionStateChanged);
		ActionSubsystem->OnActionStateChanged.AddDynamic(this, &UStoneEventPanelWidget::HandleActionStateChanged);

		ActionSubsystem->OnActionProgressChanged.RemoveDynamic(this, &UStoneEventPanelWidget::HandleActionProgressChanged);
		ActionSubsystem->OnActionProgressChanged.AddDynamic(this, &UStoneEventPanelWidget::HandleActionProgressChanged);
	}
}

void UStoneEventPanelWidget::UnbindAll()
{
	// Wichtig: nicht auf OverlayController (UObject/Base) zugreifen -> immer über OC casten
	if (UStoneOverlayWidgetController* OC = GetOverlayController())
	{
		OC->OnOverlayEventChanged.RemoveDynamic(this, &UStoneEventPanelWidget::HandleOverlayEventChanged);
		OC->OnOverlaySnapshotChanged.RemoveDynamic(this, &UStoneEventPanelWidget::HandleOverlaySnapshotChanged);
	}

	if (ActionSubsystem)
	{
		ActionSubsystem->OnActionStateChanged.RemoveDynamic(this, &UStoneEventPanelWidget::HandleActionStateChanged);
		ActionSubsystem->OnActionProgressChanged.RemoveDynamic(this, &UStoneEventPanelWidget::HandleActionProgressChanged);
	}

	OverlayController = nullptr;
	ActionSubsystem = nullptr;
	CurrentEvent = nullptr;
}

void UStoneEventPanelWidget::HandleOverlayEventChanged(const UStoneEventData* Event)
{
	RefreshFromEvent(Event);
}

void UStoneEventPanelWidget::HandleOverlaySnapshotChanged(const FStoneSnapshot& /*Snapshot*/)
{
	// Snapshot changes can affect choice enabled/visibility and progress visibility.
	if (!CurrentEvent) return;
	RebuildChoices();
	RefreshProgressVisual();
}

void UStoneEventPanelWidget::HandleActionStateChanged()
{
	RefreshProgressVisual();
}

void UStoneEventPanelWidget::HandleActionProgressChanged(float /*Progress01*/)
{
	RefreshProgressVisual();
}

void UStoneEventPanelWidget::RefreshFromEvent(const UStoneEventData* Event)
{
	CurrentEvent = Event;

	if (!CurrentEvent)
	{
		// No event open. We still may want to show progress (travel/expedition/action) in this panel.
		ClearUI();

		bool bHasProgressSource = false;

		if (UStoneOverlayWidgetController* OC = GetOverlayController())
		{
			if (UStoneRunSubsystem* Run = OC->GetRunSubsystem())
			{
				bHasProgressSource = (Run->IsTravelActive() || Run->IsOnExpedition());
			}
		}

		if (ActionSubsystem && ActionSubsystem->IsActionRunning())
		{
			bHasProgressSource = true;
		}

		if (bHasProgressSource && PB_Progress)
		{
			UE_LOG(LogTemp, Display, TEXT("[StoneUI][EventPanel] RefreshFromEvent: nullptr -> show progress only"));
			SetVisibility(ESlateVisibility::Visible);
			RefreshProgressVisual();
			return;
		}

		UE_LOG(LogTemp, Display, TEXT("[StoneUI][EventPanel] RefreshFromEvent: nullptr -> clear/hide"));
		SetVisibility(ESlateVisibility::Collapsed);
		BP_OnEventHidden();
		return;
	}

	SetVisibility(ESlateVisibility::Visible);

	if (TB_Title) TB_Title->SetText(CurrentEvent->Title);
	if (TB_Body)  TB_Body->SetText(CurrentEvent->Body);

	UE_LOG(LogTemp, Display, TEXT("[StoneUI][EventPanel] RefreshFromEvent: %s | Choices=%d"),
		*CurrentEvent->EventId.ToString(),
		CurrentEvent->Choices.Num());

	RebuildChoices();
	RefreshProgressVisual();

	BP_OnEventShown();
}

void UStoneEventPanelWidget::ClearUI()
{
	if (TB_Title) TB_Title->SetText(FText::GetEmpty());
	if (TB_Body)  TB_Body->SetText(FText::GetEmpty());
	if (VB_Choices) VB_Choices->ClearChildren();

	if (PB_Progress)
	{
		PB_Progress->SetPercent(0.f);
		PB_Progress->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UStoneEventPanelWidget::RebuildChoices()
{
	if (!VB_Choices)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneUI][EventPanel] RebuildChoices: VB_Choices missing (BindWidget?)"));
		return;
	}

	VB_Choices->ClearChildren();

	UStoneOverlayWidgetController* OC = GetOverlayController();
	if (!OC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneUI][EventPanel] RebuildChoices: controller is not overlay controller"));
		return;
	}

	if (!CurrentEvent) return;

	TArray<FStoneChoiceResolved> Resolved;
	OC->GetResolvedChoices(Resolved);

	const int32 ChoiceCount = CurrentEvent->Choices.Num();
	const int32 ResolvedCount = Resolved.Num();
	const int32 N = FMath::Min(ChoiceCount, ResolvedCount);

	if (N == 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[StoneUI][EventPanel] RebuildChoices: no choices to build (EventChoices=%d Resolved=%d)"),
			ChoiceCount, ResolvedCount);
		return;
	}

	if (!ChoiceButtonClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneUI][EventPanel] RebuildChoices: ChoiceButtonClass not set on BP child."));
		return;
	}

	int32 VisibleBuilt = 0;
	for (int32 i = 0; i < N; ++i)
	{
		const FStoneChoiceData& ChoiceData = CurrentEvent->Choices[i];
		const FStoneChoiceResolved& R = Resolved[i];

		if (!R.bVisible) continue;

		UStoneChoiceButtonWidget* Btn = CreateWidget<UStoneChoiceButtonWidget>(GetWorld(), ChoiceButtonClass);
		if (!Btn) continue;

		Btn->InitChoice(i, ChoiceData.ChoiceText, R.bEnabled, R.bSoftFail, R.DisabledReason);

		Btn->OnChoiceClicked.RemoveDynamic(this, &UStoneEventPanelWidget::HandleChoiceClicked);
		Btn->OnChoiceClicked.AddDynamic(this, &UStoneEventPanelWidget::HandleChoiceClicked);

		VB_Choices->AddChild(Btn);
		VisibleBuilt++;
	}

	UE_LOG(LogTemp, Display, TEXT("[StoneUI][EventPanel] RebuildChoices: built=%d (EventChoices=%d Resolved=%d)"),
		VisibleBuilt, ChoiceCount, ResolvedCount);
}

void UStoneEventPanelWidget::HandleChoiceClicked(int32 ChoiceIndex)
{
	UStoneOverlayWidgetController* OC = GetOverlayController();
	if (!OC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneUI][EventPanel] ChoiceClicked(%d) but controller not overlay"), ChoiceIndex);
		return;
	}

	UStoneRunSubsystem* Run = OC->GetRunSubsystem();
	if (!Run)
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneUI][EventPanel] ChoiceClicked(%d) but RunSubsystem null"), ChoiceIndex);
		return;
	}

	if (!CurrentEvent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneUI][EventPanel] ChoiceClicked(%d) but CurrentEvent null (stale UI?)"), ChoiceIndex);
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("[StoneUI][EventPanel] ApplyChoice(%d) Event=%s"),
		ChoiceIndex, *CurrentEvent->EventId.ToString());

	Run->ApplyChoice(ChoiceIndex);
}

void UStoneEventPanelWidget::RefreshProgressVisual()
{
	if (!PB_Progress)
	{
		return;
	}

	UStoneOverlayWidgetController* OC = GetOverlayController();
	if (!OC)
	{
		PB_Progress->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	UStoneRunSubsystem* Run = OC->GetRunSubsystem();
	if (!Run)
	{
		PB_Progress->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// Priority: ActionSubsystem (world actions) > Travel/Expedition (run realtime)
	float Progress01 = -1.f;
	const TCHAR* Source = TEXT("");

	if (ActionSubsystem && ActionSubsystem->IsActionRunning())
	{
		Progress01 = ActionSubsystem->GetActionProgress01();
		Source = TEXT("Action");
	}
	else if (Run->IsTravelActive())
	{
		Progress01 = Run->GetTravelProgress01();
		Source = TEXT("Travel");
	}
	else if (Run->IsOnExpedition())
	{
		Progress01 = Run->GetExpeditionProgress01();
		Source = TEXT("Expedition");
	}

	if (Progress01 < 0.f)
	{
		PB_Progress->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	PB_Progress->SetVisibility(ESlateVisibility::Visible);
	PB_Progress->SetPercent(FMath::Clamp(Progress01, 0.f, 1.f));

	// log only if visible + event exists to avoid spam
	if (IsVisible() && CurrentEvent)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[StoneUI][EventPanel] Progress %s=%.3f"), Source, Progress01);
	}
}

UStoneOverlayWidgetController* UStoneEventPanelWidget::GetOverlayController() const
{
	// OverlayController ist absichtlich generisch (Tutorial-Style). Panels casten auf den spezialisierten Controller.
	return Cast<UStoneOverlayWidgetController>(OverlayController);
}
