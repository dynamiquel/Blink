#include "TestCameraReader.h"

#include "EyeDetector.h"


void UTestCameraReader::Stop()
{
	Super::Stop();

	if (EyeDetector)
	{
		delete EyeDetector;
		EyeDetector = nullptr;
		cv::destroyWindow("Eye Result");
	}
}

void UTestCameraReader::Start()
{
	Super::Start();
	
	if (!EyeDetector)
	{
		cv::namedWindow("Eye Result", cv::WINDOW_NORMAL);
		//EyeDetector = new FEyeDetector(this);
	}
}

void UTestCameraReader::ProcessNextFrame(cv::Mat& Frame)
{
	Super::ProcessNextFrame(Frame);
	if (EyeDetector)
	{
		const auto ThreadFrame = EyeDetector->GetCurrentFrame();
		if (ThreadFrame->data != nullptr)
			cv::imshow("Eye Result", *ThreadFrame);
	}
}
