#include "Game/StoneRunTraceBuffer.h"

void UStoneRunTraceBuffer::Init(int32 InMaxEntries)
{
	MaxEntries = FMath::Max(32, InMaxEntries);
	Entries.Reserve(MaxEntries);
}

void UStoneRunTraceBuffer::Add(const FStoneTraceEntry& Entry)
{
	if (Entries.Num() >= MaxEntries)
	{
		Entries.RemoveAt(0, 1, EAllowShrinking::No);
	}
	Entries.Add(Entry);
	OnTraceAdded.Broadcast(Entry);
}

void UStoneRunTraceBuffer::GetAll(TArray<FStoneTraceEntry>& Out) const
{
	Out = Entries;
}

void UStoneRunTraceBuffer::Clear()
{
	Entries.Reset();
}
