// Fill out your copyright notice in the Description page of Project Settings.


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
