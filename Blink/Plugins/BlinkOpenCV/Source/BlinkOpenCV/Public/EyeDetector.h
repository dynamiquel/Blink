#pragma once
#include "FeatureDetector.h"

class BLINKOPENCV_API FEyeDetector : public FFeatureDetector
{
public:
	FEyeDetector(UCameraReader* InCameraReader);
	
protected:
	virtual uint32 ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime) override;
	
};
