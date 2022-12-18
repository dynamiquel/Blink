// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#include "CameraReader.h"
#include "BlinkOpenCV.h"
#include "EyeDetector.h"
#include "TestVideoReader.h"
#include "VideoReader.h"
#include "Kismet/KismetSystemLibrary.h"
#include "opencv2/unreal.hpp"

UCameraReader::UCameraReader()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.f / 30.f;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bUseCamera = true;
	CameraIndex = 0;
	VideoFileLocation = TEXT("C:/Users/Liamk/Downloads/destiny2.mp4");
	bResize = true;
	ResizeDimensions = FVector2D(1280, 720);
	bShowInSeparateWindow = true;
	WindowName = TEXT("Camera");
	VideoReader = nullptr;
	VideoReaderTickRate = 1.f / 30.f;
	EyeSampleRate = 1.f / 30.f;
	BlinkResetTime = 3;
	WinkResetTime = 3;
	ConsiderAsOpenTime = .25f;
}

void UCameraReader::BeginPlay()
{
	Super::BeginPlay();

	PrintOpenCVBuildInfo();
}

void UCameraReader::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (VideoReader)
	{
		// Video active status has changed.
		if (VideoReader->IsVideoActive() != bVideoActive)
		{
			bVideoActive = !bVideoActive;
			bVideoActive ? OnCameraFound() : OnCameraLost();
		}
	}

	#if UE_BUILD_DEVELOPMENT || UE_EDITOR
	// Debugging purposes. Uses the tick rate of the component as the refresh rate for the video stream output.
	// Window rendering can only be done from the game thread, hence why it is being done here.
	if (bShowInSeparateWindow && VideoReader)
		VideoReader->Render();
	#endif
}

void UCameraReader::Activate(bool bReset)
{
	if (bReset || ShouldActivate())
	{
		SetComponentTickEnabled(true);
		SetActiveFlag(true);

		OnComponentActivated.Broadcast(this, bReset);

		// Above code is copied from parent.

		// If VideoReader exists, stop it (can happen if bReset is true).
		if (VideoReader)
			Stop();
		
		if (bUseCamera)
		{
			VideoReader = new FTestVideoReader(
				CameraIndex,
				VideoReaderTickRate,
				bResize ? ResizeDimensions : FVector2D());
		}
		else
		{
			VideoReader = new FTestVideoReader(
				VideoFileLocation,
				VideoReaderTickRate,
				bResize ? ResizeDimensions : FVector2D());
		}
		
		GetWorld()->GetTimerManager().SetTimer(
			EyeSampleTimer,
			this,
			&UCameraReader::OnEyeSampleTick,
			EyeSampleRate,
			true);
	}
}

void UCameraReader::BeginDestroy()
{
	Super::BeginDestroy();
	Stop();
}

void UCameraReader::Deactivate()
{
	Super::Deactivate();
	Stop();
}

void UCameraReader::PrintOpenCVBuildInfo()
{
	UE_LOG(LogBlinkOpenCV, Display, TEXT("%s"), *FString(cv::getBuildInformation().c_str()));
}

void UCameraReader::Stop()
{
	// It's possible for world not to exist, such as game being stopped.
	if (GetWorld())
		GetWorld()->GetTimerManager().ClearTimer(EyeSampleTimer);
	
	if (VideoReader)
	{
		#if UE_BUILD_DEVELOPMENT || UE_EDITOR
		if (bShowInSeparateWindow)
			VideoReader->StopRendering();
		#endif
		
		delete VideoReader;
		VideoReader = nullptr;
	}
}

void UCameraReader::OnEyeSampleTick()
{
	if (VideoReader && VideoReader->IsVideoActive())
	{
		// Ensure correct Video Reader type.
		if (const FTestVideoReader* Casted = static_cast<FTestVideoReader*>(VideoReader))
		{
			// Safely retrieve the eye detector.
			if (const auto EyeDetector = Casted->GetEyeDetector().Pin())
			{
				const double CurrentTime = FPlatformTime::Seconds();

				double LastBlinkTimeValue = 0;
				if (const auto LastBlinkTime = EyeDetector->GetLastBlinkTime().Pin())
					LastBlinkTimeValue = *LastBlinkTime;

				if (CurrentTime > PreviousBlinkTime + BlinkResetTime /* different* blink */ && LastBlinkTimeValue > PreviousBlinkTime /* new blink */)
				{
					bWasOpenLast = false;
					PreviousBlinkTime = CurrentTime;
					OnBlink();
					return;
				}

				double LastLeftWinkTimeValue = 0;
				if (const auto LastLeftWinkTime = EyeDetector->GetLastLeftWinkTime().Pin())
					LastLeftWinkTimeValue = *LastLeftWinkTime;
				
				if (LastLeftWinkTimeValue > PreviousLeftWinkTime && CurrentTime > PreviousLeftWinkTime + WinkResetTime)
				{
					bWasOpenLast = false;
					PreviousLeftWinkTime = CurrentTime;
					OnLeftEyeWink();
					return;
				}

				double LastRightWinkTimeValue = 0;
				if (const auto LastRightWinkTime = EyeDetector->GetLastRightWinkTime().Pin())
					LastRightWinkTimeValue = *LastRightWinkTime;
				
				if (LastRightWinkTimeValue > PreviousRightWinkTime && CurrentTime > PreviousRightWinkTime + WinkResetTime)
				{
					bWasOpenLast = false;
					PreviousRightWinkTime = CurrentTime;
					OnRightEyeWink();
					return;
				}

				double OpenTime = CurrentTime - ConsiderAsOpenTime;
				if (!bWasOpenLast && LastBlinkTimeValue < OpenTime && LastLeftWinkTimeValue < OpenTime && LastRightWinkTimeValue < OpenTime)
				{
					bWasOpenLast = true;
					OnBothOpen();
				}
			}
		}
	}
}

void UCameraReader::OnBothOpen_Implementation()
{
}

void UCameraReader::OnBlink_Implementation()
{
	BlinkCount++;
	
	UKismetSystemLibrary::PrintString(
		this,
		FString::Printf(TEXT("Blinks: %d"), BlinkCount),
		true,
		false,
		FLinearColor(0, 1.f, .66f),
		FLT_MAX,
		FName(TEXT("BlinkNum")));

	UKismetSystemLibrary::PrintString(this, TEXT("Blinked"), true, false);
}

void UCameraReader::OnLeftEyeWink_Implementation()
{
	LeftWinkCount++;
	
	UKismetSystemLibrary::PrintString(
		this,
		FString::Printf(TEXT("Left Winks: %d"), LeftWinkCount),
		true,
		false,
		FLinearColor(1.f, .66, 0.f),
		FLT_MAX,
		FName(TEXT("LeftWinkNum")));

	UKismetSystemLibrary::PrintString(this, TEXT("Left Winked"), true, false);
}

void UCameraReader::OnRightEyeWink_Implementation()
{
	RightWinkCount++;
	
	UKismetSystemLibrary::PrintString(
		this,
		FString::Printf(TEXT("Right Winks: %d"), RightWinkCount),
		true,
		false,
		FLinearColor(.66f, 0, 1.f),
		FLT_MAX,
		FName(TEXT("RightinkNum")));

	UKismetSystemLibrary::PrintString(this, TEXT("Right Winked"), true, false);
}

void UCameraReader::OnCameraLost_Implementation()
{
	UKismetSystemLibrary::PrintString(
		this,
		TEXT("CAMERA CANNOT BE FOUND"),
		true,
		false,
		FLinearColor(1.f, 0, 1.f),
		FLT_MAX,
		FName(TEXT("CameraLost")));
}

void UCameraReader::OnCameraFound_Implementation()
{
	UKismetSystemLibrary::PrintString(
		this,
		TEXT("CAMERA FOUND"),
		true,
		false,
		FLinearColor(0.f, 1.f, 0.f),
		2.f,
		FName(TEXT("CameraLost")));
}
