// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#include "BlinkGameState.h"

#include "Kismet/KismetSystemLibrary.h"

void ABlinkGameState::SetScore(int32 NewScore)
{
	Score = NewScore;

	// Keep on screen.
	UKismetSystemLibrary::PrintString(
		this,
		FString::Printf(TEXT("Score: %d"), Score),
		true,
		false,
		FLinearColor(0, 1, 0),
		FLT_MAX,
		FName(TEXT("Score")));
}
