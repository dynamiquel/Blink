// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#include "BlinkPlayerController.h"
#include "BlinkCameraReader.h"


// Sets default values
ABlinkPlayerController::ABlinkPlayerController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CameraReader = CreateDefaultSubobject<UBlinkCameraReader>(TEXT("CameraReader"));
}

// Called when the game starts or when spawned
void ABlinkPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Disable player input until we have confirmation the camera has been detected.
	GetPawn()->DisableInput(this);

	// Start the camera reader in a few seconds.
	GetWorldTimerManager().SetTimer(StartCameraReaderTimer,
		this,
		&ABlinkPlayerController::StartCameraReader,
		2.f,
		false);
}

// Called every frame
void ABlinkPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABlinkPlayerController::EnableCameraReader(bool bEnable) const
{
	if (IsValid(CameraReader))
		CameraReader->SetActive(bEnable);
}