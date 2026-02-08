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
	// === Authoritative guard against stale/duplicate broadcasts ===
	// Multiple sources can call RefreshFromEvent (C++ binding + BP binding + BroadcastInitialValues).
	// If the parameter is nullptr, but the RunSubsystem says an event IS open, ignore this call.
	// This prevents a late BroadcastInitialValues(nullptr) or BP re-broadcast from wiping a valid event.
	if (!Event)
	{
		UStoneOverlayWidgetController* OC = GetOverlayController();
		UStoneRunSubsystem* Run = OC ? OC->GetRunSubsystem() : nullptr;
		const UStoneEventData* AuthEvent = Run ? Run->GetCurrentEvent() : nullptr;

		if (AuthEvent)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[StoneUI][EventPanel] RefreshFromEvent(NULL) BLOCKED: RunSubsystem still has open event '%s'. Ignoring stale null broadcast."),
				*AuthEvent->GetName());
			return;
		}
	}

	// Clear stale local state before applying new event.
	CurrentEvent = nullptr;

	// Clear existing buttons always (we rebuild if needed)
	if (VB_Choices)
	{
		VB_Choices->ClearChildren();
	}

	// Hard BindWidget verification: if these fire, the BP widget types don't match the C++ types.
	ensureMsgf(TB_Title, TEXT("[StoneUI][EventPanel] TB_Title is null! BindWidget type mismatch: BP widget must be UStoneCustomTextBlock, named exactly 'TB_Title', with 'Is Variable' enabled."));
	ensureMsgf(TB_Body,  TEXT("[StoneUI][EventPanel] TB_Body is null! BindWidget type mismatch: BP widget must be UStoneCustomTextBlock, named exactly 'TB_Body', with 'Is Variable' enabled."));

	UE_LOG(LogTemp, Log, TEXT("[StoneUI][EventPanel] RefreshFromEvent: Event=%s TB_Title=%s TB_Body=%s"),
		Event ? *Event->GetName() : TEXT("NULL"),
		TB_Title ? TEXT("OK") : TEXT("NULL"),
		TB_Body ? TEXT("OK") : TEXT("NULL"));

	// === No event open: show ACTION / TRAVEL status instead of empty panel ===
	if (!Event)
	{
		const bool bActionRunning = ActionSubsystem && ActionSubsystem->IsActionRunning();

		if (bActionRunning)
		{
			const FText Title = ActionSubsystem->GetActionTitleText();
			const FText Phase = ActionSubsystem->GetPhaseText();
			const float Remaining = ActionSubsystem->GetRemainingSeconds();

			if (TB_Title) TB_Title->SetText(Title);

			if (TB_Body)
			{
				// Example: "On the way… (Outbound)  |  ETA: 12s"
				const FText EtaText = FText::FromString(FString::Printf(TEXT("ETA: %.0fs"), Remaining));
				const FText Body = Phase.IsEmpty()
					? EtaText
					: FText::Format(FText::FromString(TEXT("{0}  |  {1}")), Phase, EtaText);

				TB_Body->SetText(Body);
			}

			if (PB_Progress)
			{
				PB_Progress->SetPercent(ActionSubsystem->GetActionProgress01());
				PB_Progress->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}

			// Optional BP hook: treat as "shown" so BP can animate in
			BP_OnEventShown();
			return;
		}

		// Truly idle: no event and no action.
		if (TB_Title) TB_Title->SetText(FText::GetEmpty());
		if (TB_Body)  TB_Body->SetText(FText::GetEmpty());
		if (PB_Progress)
		{
			PB_Progress->SetPercent(0.f);
			PB_Progress->SetVisibility(ESlateVisibility::Collapsed);
		}
		BP_OnEventHidden();
		return;
	}

	// === Normal event view ===
	CurrentEvent = Event;

	UE_LOG(LogTemp, Log, TEXT("[StoneUI][EventPanel] Showing event '%s': TitleLen=%d BodyLen=%d ChoiceCount=%d"),
		*Event->EventId.ToString(),
		Event->Title.ToString().Len(),
		Event->Body.ToString().Len(),
		Event->Choices.Num());

	if (TB_Title)
	{
		TB_Title->SetText(Event->Title);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneUI][EventPanel] TB_Title is NULL - cannot set title text '%s'. Check WBP_EventPanel BindWidget."), *Event->Title.ToString());
	}

	if (TB_Body)
	{
		TB_Body->SetText(Event->Body);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[StoneUI][EventPanel] TB_Body is NULL - cannot set body text. Check WBP_EventPanel BindWidget."));
	}

	// Progress: if action is running, keep progress visible while events happen too
	if (PB_Progress)
	{
		const bool bActionRunning = ActionSubsystem && ActionSubsystem->IsActionRunning();
		PB_Progress->SetVisibility(bActionRunning ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		if (bActionRunning)
		{
			PB_Progress->SetPercent(ActionSubsystem->GetActionProgress01());
		}
		else
		{
			PB_Progress->SetPercent(0.f);
		}
	}

	RebuildChoices();
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
