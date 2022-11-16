#pragma once

#include "CameraReader.h"
#include "TestCameraReader.generated.h"


class FEyeDetector;
UCLASS()
class BLINKOPENCV_API UTestCameraReader : public UCameraReader
{
	GENERATED_BODY()

private:
	FEyeDetector* EyeDetector = nullptr;

protected:
	virtual void ProcessNextFrame(cv::Mat& Frame) override;
	virtual void Start() override;
	virtual void Stop() override;
};