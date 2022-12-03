// Fill out your copyright notice in the Description page of Project Settings.

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
