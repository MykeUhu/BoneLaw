// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/StoneAssetManager.h"

#include "Core/StoneGameplayTags.h"

UStoneAssetManager& UStoneAssetManager::Get()
{
	check(GEngine != nullptr);

	UStoneAssetManager* Manager = Cast<UStoneAssetManager>(GEngine->AssetManager);
	check(Manager != nullptr);
	return *Manager;
}

void UStoneAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FStoneGameplayTags::InitializeNativeGameplayTags();
}
