#include "TestVideoReader.h"
#include "CascadeEyeDetector.h"
#include "DnnEyeDetector.h"

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
		EyeDetector = MakeShared<FDnnEyeDetector>(this);

	AddChildRenderer(EyeDetector);
}
