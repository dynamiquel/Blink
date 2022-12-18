// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

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
