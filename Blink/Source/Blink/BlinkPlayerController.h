// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#pragma once

#include "CoreMinimal.h"
#include "BlinkPlayerController.generated.h"

class UCameraReader;
UCLASS(config=Game, BlueprintType, Blueprintable)
class BLINK_API ABlinkPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABlinkPlayerController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Exec)
	void EnableCameraReader(bool bEnable = true) const;

protected:
	UPROPERTY(VisibleDefaultsOnly)
	UCameraReader* CameraReader = nullptr;

	FTimerHandle StartCameraReaderTimer;

	UFUNCTION()
	void StartCameraReader() { EnableCameraReader(); }
};