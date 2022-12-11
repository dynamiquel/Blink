#pragma once

#include "VideoReader.h"

class FCascadeEyeDetector;
class BLINKOPENCV_API FTestVideoReader : public FVideoReader
{
public:
	FTestVideoReader(int32 InCameraIndex, float InRefreshRate = 1.f/30.f, FVector2D InResizeDimensions = FVector2D());
	FTestVideoReader(const FString& InVideoSource, float InRefreshRate = 1.f/30.f, FVector2D InResizeDimensions = FVector2D());
private:
	FCascadeEyeDetector* EyeDetector = nullptr;

protected:
	virtual void Start() override;
	virtual void Exit() override;
};
