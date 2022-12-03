// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlinkCharacter.h"
#include "EnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class BLINK_API AEnemyCharacter : public ABlinkCharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
