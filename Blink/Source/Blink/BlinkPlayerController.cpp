// Fill out your copyright notice in the Description page of Project Settings.


#include "BlinkPlayerController.h"

#include "BlinkOpenCV.h"
#include "TestCameraReader.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
ABlinkPlayerController::ABlinkPlayerController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CameraReader = CreateDefaultSubobject<UTestCameraReader>(TEXT("CameraReader"));
}

// Called when the game starts or when spawned
void ABlinkPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
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