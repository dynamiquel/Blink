// Fill out your copyright notice in the Description page of Project Settings.


#include "BlinkCameraReader.h"

#include "BlinkGameMode.h"

void UBlinkCameraReader::OnBlink_Implementation()
{
	Super::OnBlink_Implementation();

	GetWorld()->GetAuthGameMode<ABlinkGameMode>()->PlayerBlinked();
}

void UBlinkCameraReader::OnLeftEyeWink_Implementation()
{
	//Super::OnLeftEyeWink_Implementation();
}

void UBlinkCameraReader::OnRightEyeWink_Implementation()
{
	//Super::OnRightEyeWink_Implementation();
}
