// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#pragma once

#include "CoreMinimal.h"
#include "BlinkGameMode.h"
#include "TargetGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLINK_API ATargetGameMode : public ABlinkGameMode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> TargetClasses;

	UPROPERTY(EditAnywhere)
	FRuntimeFloatCurve TargetSize;

protected:
	virtual TSubclassOf<AActor> ChooseEnemyClass() const override;
	virtual void FindEnemySpawnPoints() override;
	virtual FTransform ChooseEnemySpawnTransform(const TSubclassOf<AActor> EnemyClass) const override;
};
