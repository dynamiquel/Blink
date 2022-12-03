// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetGameMode.h"

#include "BlinkGameState.h"
#include "Components/SphereComponent.h"
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
