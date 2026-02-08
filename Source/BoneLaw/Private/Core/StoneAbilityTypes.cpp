// Copyright by MykeUhu

#include "Core/StoneAbilityTypes.h"

// Optional: nur nötig wenn du SafeNetSerializeTArray oder ähnliches verwendest
// #include "Net/UnrealNetwork.h"

bool FStoneGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	// Wir replizieren nur Felder, die wirklich gebraucht werden.
	// Dazu bauen wir ein Bitmask-Set ("RepBits") und serialisieren nur aktive Felder.
	// -> spart Bandbreite und bleibt stabil, wenn Felder später dazu kommen.

	uint32 RepBits = 0;

	if (Ar.IsSaving())
	{
		if (bIsSuccessfulDebuff)
		{
			RepBits |= 1 << 0;
		}
		if (DebuffDamage > 0.f)
		{
			RepBits |= 1 << 1;
		}
		if (DebuffDuration > 0.f)
		{
			RepBits |= 1 << 2;
		}
		if (DebuffFrequency > 0.f)
		{
			RepBits |= 1 << 3;
		}
	}

	// 4 Bits -> wir serialisieren genau 4
	Ar.SerializeBits(&RepBits, 4);

	if (RepBits & (1 << 0))
	{
		Ar << bIsSuccessfulDebuff;
	}
	if (RepBits & (1 << 1))
	{
		Ar << DebuffDamage;
	}
	if (RepBits & (1 << 2))
	{
		Ar << DebuffDuration;
	}
	if (RepBits & (1 << 3))
	{
		Ar << DebuffFrequency;
	}

	bOutSuccess = true;
	return true;
}
