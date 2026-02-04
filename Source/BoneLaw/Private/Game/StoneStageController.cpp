#include "Game/StoneStageController.h"

#include "Core/StoneGameplayTags.h"
#include "Data/StoneStageTypes.h"
#include "Runtime/StoneRunSubsystem.h"

AStoneStageController::AStoneStageController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AStoneStageController::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		Run = GI->GetSubsystem<UStoneRunSubsystem>();
	}

	if (Run)
	{
		Run->OnSnapshotChanged.AddDynamic(this, &AStoneStageController::HandleSnapshotChanged);
		HandleSnapshotChanged(Run->GetSnapshot());
	}
}

void AStoneStageController::HandleSnapshotChanged(const FStoneSnapshot& Snapshot)
{
	const FStoneStageState StageState = MakeStageState(Snapshot);
	BP_ApplyStageState(StageState);
}

FStoneStageState AStoneStageController::MakeStageState(const FStoneSnapshot& Snap) const
{
	const FStoneGameplayTags& GTags = FStoneGameplayTags::Get();

	FStoneStageState S;
	S.Time = Snap.Time;
	S.Tags = Snap.RunTags;
	S.Focus = Snap.FocusTag;
	S.Morale = Snap.Morale;
	S.Warmth = Snap.Warmth;
	S.bFireUnlocked = Snap.RunTags.HasTag(GTags.Unlock_Fire);
	return S;
}

