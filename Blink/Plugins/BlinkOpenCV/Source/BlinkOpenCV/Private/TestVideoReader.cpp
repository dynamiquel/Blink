#include "TestVideoReader.h"
#include "CascadeEyeDetector.h"

FTestVideoReader::FTestVideoReader(int32 InCameraIndex, float InRefreshRate, FVector2D InResizeDimensions)
	: FVideoReader(InCameraIndex, InRefreshRate, InResizeDimensions)
{ }

FTestVideoReader::FTestVideoReader(const FString& InVideoSource, float InRefreshRate, FVector2D InResizeDimensions)
	: FVideoReader(InVideoSource, InRefreshRate, InResizeDimensions)
{ }

void FTestVideoReader::Exit()
{
	if (EyeDetector)
	{
		RemoveChildRenderer(EyeDetector);
		delete EyeDetector;
		EyeDetector = nullptr;
	}
	
	FVideoReader::Exit();
}

void FTestVideoReader::Start()
{
	FVideoReader::Start();
	
	if (!EyeDetector)
		EyeDetector = new FCascadeEyeDetector(this);

	AddChildRenderer(EyeDetector);
}