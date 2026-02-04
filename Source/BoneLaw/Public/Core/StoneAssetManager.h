// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "StoneAssetManager.generated.h"

/**
 *
 */
UCLASS()
class BONELAW_API UStoneAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UStoneAssetManager& Get();

protected:
	virtual void StartInitialLoading() override;
};
