// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"

AEnemyCharacter::AEnemyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer
		.DoNotCreateDefaultSubobject(TEXT("FirstPersonCamera"))
		.DoNotCreateDefaultSubobject(TEXT("CharacterMesh1P")))
{
}
