// Fill out your copyright notice in the Description page of Project Settings.

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

	int32 GetScore() const { return Score; }
	void SetScore(int32 NewScore);

private:
	UPROPERTY(VisibleInstanceOnly, Category=GameState)
	int32 Score = 0;
};
