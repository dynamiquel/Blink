// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#include "TestVideoReader.h"

#include "CascadeEyeDetector.h"
#include "DnnCascadeEyeDetector.h"

FTestVideoReader::FTestVideoReader(int32 InCameraIndex, float InRefreshRate, FVector2D InResizeDimensions)
	: FVideoReader(InCameraIndex, InRefreshRate, InResizeDimensions)
{ }

FTestVideoReader::FTestVideoReader(const FString& InVideoSource, float InRefreshRate, FVector2D InResizeDimensions)
	: FVideoReader(InVideoSource, InRefreshRate, InResizeDimensions)
{ }

void FTestVideoReader::Exit()
{
	FVideoReader::Exit();
}

void FTestVideoReader::Stop()
{
	FVideoReader::Stop();
	
	if (EyeDetector.IsValid())
	{
		RemoveChildRenderer(EyeDetector);
		EyeDetector->Kill();
		EyeDetector.Reset();
	}
}

void FTestVideoReader::Start()
{
	FVideoReader::Start();
	
	if (!EyeDetector.IsValid())
		EyeDetector = MakeShared<FCascadeEyeDetector>(this);

	AddChildRenderer(EyeDetector);
}
