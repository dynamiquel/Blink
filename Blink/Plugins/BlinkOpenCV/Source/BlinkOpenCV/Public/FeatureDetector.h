#pragma once

#include "PreOpenCVHeaders.h"
#include <opencv2/core.hpp>
#include "PostOpenCVHeaders.h"
#include "Containers/CircularQueue.h"

class UCameraReader;

class BLINKOPENCV_API FFeatureDetector : public FRunnable, FSingleThreadRunnable
{
public:
	FFeatureDetector(UCameraReader* InCameraReader);
	
public:
	// Overriden from FRunnable
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;
	virtual FSingleThreadRunnable* GetSingleThreadInterface() override { return this; }
	virtual ~FFeatureDetector() override;
	// -----------------------
	// Overriden from FSingleRunnable
	virtual void Tick() override;
	// -----------------------

protected:
	const TCHAR* ThreadName = TEXT("UnnamedFeatureDetectorThread");
	TSharedPtr<cv::Mat> CurrentFrame;
	
private:
	FRunnableThread* Thread = nullptr;
	bool bActive = false;
	float RefreshRate = .05f;

	UCameraReader* CameraReader = nullptr;
	double PreviousTime = 0;
	double ST_TimeUntilRefresh = 0;

public:
	FORCEINLINE bool IsActive() const { return bActive; }
	TSharedPtr<cv::Mat> GetCurrentFrame() const { return CurrentFrame; }

protected:
	virtual uint32 ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime);

private:
	cv::Mat GetNextFrame() const;
	double UpdateAndGetDeltaTime();
};
