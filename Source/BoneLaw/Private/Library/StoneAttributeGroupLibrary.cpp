// Copyright by MykeUhu

#include "Library/StoneAttributeGroupLibrary.h"

#include "UI/Widget/StoneAttributeWidgetGroups.h"
#include "UObject/UnrealType.h"

static int32 GetGroupCountInternal()
{
	// Requires COUNT to be last enumerator.
	return static_cast<int32>(EAttributeWidgetGroups::COUNT);
}

int32 UStoneAttributeGroupLibrary::GetAttributeGroupCount()
{
	return GetGroupCountInternal();
}

int32 UStoneAttributeGroupLibrary::GetAttributeGroupIndex(EAttributeWidgetGroups Group)
{
	const int32 Count = GetGroupCountInternal();
	int32 Index = static_cast<int32>(Group);
	return FMath::Clamp(Index, 0, FMath::Max(0, Count - 1));
}

EAttributeWidgetGroups UStoneAttributeGroupLibrary::GetAttributeGroupByIndex(int32 Index)
{
	const int32 Count = GetGroupCountInternal();
	const int32 Clamped = FMath::Clamp(Index, 0, FMath::Max(0, Count - 1));
	return static_cast<EAttributeWidgetGroups>(Clamped);
}

void UStoneAttributeGroupLibrary::GetAttributeGroupNav(
	EAttributeWidgetGroups Current,
	bool bWrap,
	EAttributeWidgetGroups& OutPrev,
	bool& bHasPrev,
	EAttributeWidgetGroups& OutNext,
	bool& bHasNext,
	int32& OutIndex,
	int32& OutMaxIndex
)
{
	const int32 Count = GetGroupCountInternal();
	OutMaxIndex = FMath::Max(0, Count - 1);

	OutIndex = GetAttributeGroupIndex(Current);

	const bool bAtFirst = (OutIndex <= 0);
	const bool bAtLast  = (OutIndex >= OutMaxIndex);

	if (bWrap)
	{
		bHasPrev = (Count > 1);
		bHasNext = (Count > 1);

		const int32 PrevIndex = bAtFirst ? OutMaxIndex : (OutIndex - 1);
		const int32 NextIndex = bAtLast  ? 0          : (OutIndex + 1);

		OutPrev = GetAttributeGroupByIndex(PrevIndex);
		OutNext = GetAttributeGroupByIndex(NextIndex);
	}
	else
	{
		bHasPrev = !bAtFirst;
		bHasNext = !bAtLast;

		const int32 PrevIndex = bHasPrev ? (OutIndex - 1) : OutIndex;
		const int32 NextIndex = bHasNext ? (OutIndex + 1) : OutIndex;

		OutPrev = GetAttributeGroupByIndex(PrevIndex);
		OutNext = GetAttributeGroupByIndex(NextIndex);
	}
}

EAttributeWidgetGroups UStoneAttributeGroupLibrary::StepAttributeGroup(
	EAttributeWidgetGroups Current,
	int32 Delta,
	bool bWrap,
	bool& bOutChanged
)
{
	bOutChanged = false;

	const int32 Count = GetGroupCountInternal();
	if (Count <= 0 || Delta == 0)
	{
		return GetAttributeGroupByIndex(0);
	}

	const int32 MaxIndex = FMath::Max(0, Count - 1);
	int32 Index = GetAttributeGroupIndex(Current);
	int32 NewIndex = Index + Delta;

	if (bWrap && Count > 1)
	{
		// Proper wrap (handles Delta > 1 too)
		NewIndex = (NewIndex % Count + Count) % Count;
	}
	else
	{
		NewIndex = FMath::Clamp(NewIndex, 0, MaxIndex);
	}

	bOutChanged = (NewIndex != Index);
	return GetAttributeGroupByIndex(NewIndex);
}