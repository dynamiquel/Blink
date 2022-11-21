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
	FVideoReader::Exit();

	if (EyeDetector)
	{
		delete EyeDetector;
		EyeDetector = nullptr;
		cv::destroyWindow("Eye Result");
	}
}

void FTestVideoReader::Start()
{
	FVideoReader::Start();
	
	if (!EyeDetector)
	{
		cv::namedWindow("Eye Result", cv::WINDOW_NORMAL);
		EyeDetector = new FEyeDetector(this);
	}
}

void FTestVideoReader::ProcessNextFrame(cv::Mat& Frame)
{
	FVideoReader::ProcessNextFrame(Frame);
	
	if (EyeDetector)
	{
		const auto ThreadFrame = EyeDetector->GetCurrentFrame();
		if (ThreadFrame->data != nullptr)
			cv::imshow("Eye Result", *ThreadFrame);
	}
}
