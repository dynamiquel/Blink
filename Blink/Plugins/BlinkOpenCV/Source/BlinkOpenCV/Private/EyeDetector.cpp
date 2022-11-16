#include "EyeDetector.h"

#include "PreOpenCVHeaders.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/core/cuda.hpp"
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaoptflow.hpp>
#include <opencv2/cudaobjdetect.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include "PostOpenCVHeaders.h"

FEyeDetector::FEyeDetector(UCameraReader* InCameraReader)
	: FFeatureDetector(InCameraReader)
{
	ThreadName = TEXT("EyeDetectorThread");
}

uint32 FEyeDetector::ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime)
{
	cv::cuda::GpuMat Src;
	Src.upload(Frame);
	
	cv::cuda::cvtColor(Src, Src, cv::COLOR_BGR2GRAY);
	
	const auto Canny = cv::cuda::createCannyEdgeDetector(100, 200);
	Canny->detect(Src, Src);
	
	Src.download(Frame);

	return 0;
	//return FFeatureDetector::ProcessNextFrame(Frame, DeltaTime);
}
