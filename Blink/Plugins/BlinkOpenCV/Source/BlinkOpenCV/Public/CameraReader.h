#pragma once

#include "PreOpenCVHeaders.h"
#include "opencv2/opencv.hpp"
#include "PostOpenCVHeaders.h"
#include "OpenCVHelper.h"

#include "CameraReader.generated.h"

UCLASS()
class UCameraReader : public UObject
{
	GENERATED_BODY()
	
public:
	int32 CameraIndex;
	bool bResize;
	FIntVector2 ResizeDimensions;
	float RefreshRate;

	cv::VideoCapture CameraStream;
	cv::Mat CurrentFrame;
};
