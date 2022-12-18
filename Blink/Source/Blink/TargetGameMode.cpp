// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#include "TargetGameMode.h"

#include "BlinkGameState.h"
#include "Kismet/GameplayStatics.h"

TSubclassOf<AActor> ATargetGameMode::ChooseEnemyClass() const
{
	checkf(TargetClasses.Num() > 0, TEXT("No Enemy Classes were added to the GameMode."));

	const int32 RandomIndex = FMath::RandRange(0, TargetClasses.Num() - 1);
	return TargetClasses[RandomIndex];
}

void ATargetGameMode::FindEnemySpawnPoints()
{
	UGameplayStatics::GetAllActorsWithTag(this, FName(TEXT("TargetSpawn")), EnemySpawnPoints);
}

FTransform ATargetGameMode::ChooseEnemySpawnTransform(const TSubclassOf<AActor> EnemyClass) const
{
	FTransform Transform = Super::ChooseEnemySpawnTransform(EnemyClass);

	const float ActorScale = TargetSize.GetRichCurveConst()->Eval(GetGameState<ABlinkGameState>()->EnemySpawnTime);
	Transform.SetScale3D(FVector(ActorScale, ActorScale, ActorScale));
	return Transform;
}
