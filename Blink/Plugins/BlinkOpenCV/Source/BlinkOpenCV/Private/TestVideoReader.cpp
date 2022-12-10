#include "TestVideoReader.h"
#include "EyeDetector.h"

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
		EyeDetector = new FEyeDetector(this);

	AddChildRenderer(EyeDetector);
}