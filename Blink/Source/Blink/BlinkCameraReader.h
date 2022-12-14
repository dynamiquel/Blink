// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CameraReader.h"
#include "BlinkCameraReader.generated.h"

/**
 * 
 */
UCLASS()
class BLINK_API UBlinkCameraReader : public UCameraReader
{
	GENERATED_BODY()
	
protected:
	virtual void OnBlink_Implementation() override;
	virtual void OnLeftEyeWink_Implementation() override;
	virtual void OnRightEyeWink_Implementation() override;
};
