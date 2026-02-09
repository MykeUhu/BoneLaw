// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"

/**
 * Shared helper functions for ModMagCalc calculations.
 * Prevents duplicate function definitions across multiple MMC_*.cpp files.
 * 
 * These are KCD-inspired scaling functions that normalize attributes to 0-1 range.
 */
namespace StoneModMagCalc
{
	/** Normalize primary attribute (0-50) to 0-1 range. */
	inline float Primary01(float V) { return FMath::Clamp(V, 0.f, 50.f) / 50.f; }

	/** Normalize knowledge delta (50-100) to 0-1 range (knowledge only matters after baseline 50). */
	inline float KnowledgeDelta01(float V) { return FMath::Clamp((V - 50.f) / 50.f, 0.f, 1.f); }

	/** Convert normalized ratio (0-1) to a score (60-170). */
	inline float ScoreFromR(float R) { return FMath::Clamp(100.f + 70.f * R, 60.f, 170.f); }
}
