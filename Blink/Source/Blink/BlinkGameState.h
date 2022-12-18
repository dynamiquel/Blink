// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BlinkGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLINK_API ABlinkGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleInstanceOnly, Category=GameState)
	int32 Deaths = 0;

	UPROPERTY(VisibleInstanceOnly, Category=GameState)
	int32 Blinks = 0;

	UPROPERTY(VisibleInstanceOnly, Category=GameState)
	int32 CurrentEnemies = 0;

	UPROPERTY(VisibleInstanceOnly, Category=GameState)
	float EnemySpawnTime = 0;

	UPROPERTY(VisibleInstanceOnly, Category=GameState)
	bool bHasStarted = false;

	UPROPERTY(VisibleInstanceOnly, Category=GameState)
	bool bPreparingEyeStrike = false;

	UPROPERTY(VisibleInstanceOnly, Category=GameState)
	bool bDeathDisabled = false;

	int32 GetScore() const { return Score; }
	void SetScore(int32 NewScore);

private:
	UPROPERTY(VisibleInstanceOnly, Category=GameState)
	int32 Score = 0;
};
