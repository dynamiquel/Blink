#pragma once

#include "PreOpenCVHeaders.h"
#include <opencv2/core.hpp>
#include "PostOpenCVHeaders.h"

class FVideoReader;

class BLINKOPENCV_API FFeatureDetector : public FRunnable
{
public:
	FFeatureDetector(FVideoReader* InVideoReader);
	
public:
	// Overriden from FRunnable
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;
	virtual ~FFeatureDetector() override;

protected:
	const TCHAR* ThreadName = TEXT("UnnamedFeatureDetectorThread");
	TSharedPtr<cv::Mat> CurrentFrame;
	
private:
	FRunnableThread* Thread = nullptr;
	bool bActive = false;
	float RefreshRate = .05f;

	FVideoReader* VideoReader = nullptr;
	double PreviousTime = 0;
	double ST_TimeUntilRefresh = 0;

public:
	FORCEINLINE bool IsActive() const { return bActive; }
	TSharedPtr<cv::Mat> GetCurrentFrame() const { return CurrentFrame; }

protected:
	virtual uint32 ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime);

private:
	cv::Mat GetNextFrame() const;

	/**
	 * @brief Keeps track of delta-time in a thread-independent way.
	 * @return The current delta time.
	 */
	double UpdateAndGetDeltaTime();
};
