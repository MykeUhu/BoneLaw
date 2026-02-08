// Copyright by MykeUhu


#include "Core/StoneGameInstance.h"

#include "UI/CustomElements/StoneUIThemeDataAsset.h"

void UStoneGameInstance::Init()
{
	Super::Init();
	// --------------------------
	// UI Theme
	// --------------------------
	if (!DefaultUITheme)
	{
		UE_LOG(LogTemp, Warning, TEXT("DefaultUITheme is NULL (UI widgets will use fallback/default look)."));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("UI Theme SET -> %s"),
			*GetNameSafe(DefaultUITheme));
	}
}

