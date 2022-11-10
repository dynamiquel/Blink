#pragma once
#include "FeatureDetector.h"

class FEyeDetector : public FFeatureDetector
{
public:
	FEyeDetector();
	
protected:
	virtual uint32 ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime) override;
	
};
