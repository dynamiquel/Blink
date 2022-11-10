#pragma once

#include "PreOpenCVHeaders.h"
#include "opencv2/opencv.hpp"
#include "PostOpenCVHeaders.h"
#include "Containers/CircularQueue.h"

class FFeatureDetector : public FRunnable, FSingleThreadRunnable
{
public:
	FFeatureDetector();
	
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
	
private:
	FRunnableThread* Thread = nullptr;
	bool bActive = false;
	float RefreshRate = 30;
	TCircularQueue<cv::Mat> QueuedFrames = TCircularQueue<cv::Mat>(3);

	double PreviousTime = 0;
	double ST_TimeUntilRefresh = 0;

public:
	void QueueNewFrame(const cv::Mat& Frame);
	FORCEINLINE bool IsActive() const { return bActive; }

protected:
	virtual uint32 ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime);

private:
	cv::Mat GetNextFrame();
	double UpdateAndGetDeltaTime();
};