// Fill out your copyright notice in the Description page of Project Settings.

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
};