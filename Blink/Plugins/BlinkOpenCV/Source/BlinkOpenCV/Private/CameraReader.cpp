#include "CameraReader.h"
#include "BlinkOpenCV.h"
#include "TestVideoReader.h"
#include "VideoReader.h"

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
}

void UCameraReader::BeginPlay()
{
	Super::BeginPlay();

	PrintOpenCVBuildInfo();
}

void UCameraReader::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	#if UE_BUILD_DEBUG || UE_EDITOR
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
	if (VideoReader)
	{
		if (bShowInSeparateWindow)
			VideoReader->StopRendering();
		
		delete VideoReader;
		VideoReader = nullptr;
	}
}
