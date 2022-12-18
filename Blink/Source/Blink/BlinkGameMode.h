// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "BlinkGameMode.generated.h"

UCLASS(minimalapi)
class ABlinkGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	int32 MaxEnemiesAtOnce = 20;

	UPROPERTY(EditAnywhere)
	FRuntimeFloatCurve SpawnRate;

	UPROPERTY(EditAnywhere)
	float RandomSpawnRateMultiplier = .2f; // +0-20% SpawnRate
	
	UPROPERTY(EditAnywhere)
	int32 BlinksAllowed = 2;

	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> EnemySpawnPoints;

	UPROPERTY(EditAnywhere)
	int32 BulletCost = 1;

	UPROPERTY(EditAnywhere)
	bool bTreatDifferentWinksAsBlink = true;

public:
	ABlinkGameMode();
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	void EnemyKilled(int32 EnemyValue = 1);
	
	UFUNCTION(Exec)
	void PlayerDied();

	UFUNCTION(Exec)
	void PlayerBlinked();

	UFUNCTION(Exec)
	void PlayerWinked(const class UCameraReader* CameraReader, bool bRightEye);

	UFUNCTION(Exec)
	void PlayerBothEyesOpen(const class UCameraReader* CameraReader);

	UFUNCTION(Exec)
	void PlayerFound();
	
	UFUNCTION(Exec)
	void PlayerLost();

	UFUNCTION(Exec)
	void StartEnemySpawning();

	UFUNCTION(Exec)
	void StopEnemySpawning();

	void GunFired() const;

	UFUNCTION(Exec)
	void DisableDeath(bool bDisable);

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void SpawnNextEnemy();

	virtual TSubclassOf<AActor> ChooseEnemyClass() const;
	virtual FTransform ChooseEnemySpawnTransform(const TSubclassOf<AActor> EnemyClass) const;
	virtual void InitialiseSpawnedEnemy(AActor* Enemy);
	virtual void FindEnemySpawnPoints();
	
	FTimerHandle NextEnemySpawnTimer;
};



