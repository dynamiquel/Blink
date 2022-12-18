// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

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
	virtual void OnBothOpen_Implementation() override;
	virtual void OnCameraFound_Implementation() override;
	virtual void OnCameraLost_Implementation() override;
};
