// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#include "BlinkCameraReader.h"
#include "BlinkGameMode.h"

void UBlinkCameraReader::OnBlink_Implementation()
{
	GetWorld()->GetAuthGameMode<ABlinkGameMode>()->PlayerBlinked();
}

void UBlinkCameraReader::OnLeftEyeWink_Implementation()
{
	Super::OnLeftEyeWink_Implementation();

	GetWorld()->GetAuthGameMode<ABlinkGameMode>()->PlayerWinked(this, false);
}

void UBlinkCameraReader::OnRightEyeWink_Implementation()
{	
	Super::OnRightEyeWink_Implementation();
	
	GetWorld()->GetAuthGameMode<ABlinkGameMode>()->PlayerWinked(this, true);
}

void UBlinkCameraReader::OnBothOpen_Implementation()
{
	Super::OnBothOpen_Implementation();

	GetWorld()->GetAuthGameMode<ABlinkGameMode>()->PlayerBothEyesOpen(this);
}

void UBlinkCameraReader::OnCameraFound_Implementation()
{
	Super::OnCameraFound_Implementation();

	GetWorld()->GetAuthGameMode<ABlinkGameMode>()->PlayerFound();
}

void UBlinkCameraReader::OnCameraLost_Implementation()
{
	Super::OnCameraLost_Implementation();

	GetWorld()->GetAuthGameMode<ABlinkGameMode>()->PlayerLost();
}
