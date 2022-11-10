#include "EyeDetector.h"

FEyeDetector::FEyeDetector() : FFeatureDetector()
{
	ThreadName = TEXT("EyeDetectorThread");
}

uint32 FEyeDetector::ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime)
{
	return FFeatureDetector::ProcessNextFrame(Frame, DeltaTime);
}
