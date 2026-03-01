// Copyright by MykeUhu

#include "Data/StoneSettlerNameDataAsset.h"

FString UStoneSettlerNameDataAsset::GenerateRandomNameWithStream(const FRandomStream& RandomStream) const
{
	if (FirstNames.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneSettlerNameDataAsset] No first names configured. Returning fallback."));
		return TEXT("Unnamed");
	}

	const int32 FirstIndex = RandomStream.RandRange(0, FirstNames.Num() - 1);
	FString Result = FirstNames[FirstIndex];

	if (bUseSecondName && SecondNames.Num() > 0)
	{
		const int32 SecondNameIndex = RandomStream.RandRange(0, SecondNames.Num() - 1);
		Result += TEXT(" ") + SecondNames[SecondNameIndex];
	}

	return Result;
}

FString UStoneSettlerNameDataAsset::GenerateRandomName() const
{
	if (FirstNames.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneSettlerNameDataAsset] No first names configured. Returning fallback."));
		return TEXT("Unnamed");
	}

	const int32 FirstIndex = FMath::RandRange(0, FirstNames.Num() - 1);
	FString Result = FirstNames[FirstIndex];

	if (bUseSecondName && SecondNames.Num() > 0)
	{
		const int32 SecondNameIndex = FMath::RandRange(0, SecondNames.Num() - 1);
		Result += TEXT(" ") + SecondNames[SecondNameIndex];
	}

	return Result;
}

bool UStoneSettlerNameDataAsset::IsValid() const
{
	return FirstNames.Num() > 0;
}
