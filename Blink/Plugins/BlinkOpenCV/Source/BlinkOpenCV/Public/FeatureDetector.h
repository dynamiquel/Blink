#pragma once

// Importing most of the OpenCV headers for convenience sake.
#include "OpenCVHelper.h"
#include "PreOpenCVHeaders.h"
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/objdetect.hpp"
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudacodec.hpp>
#include "PostOpenCVHeaders.h"
#include "Renderable.h"

class FVideoReader;

class BLINKOPENCV_API FFeatureDetector : public FRunnable, public FRenderable
{
public:
	FFeatureDetector(FVideoReader* InVideoReader);
	
public:
	// Overriden from FRunnable
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;

	virtual void Render() override;
	virtual void StopRendering() override;
	virtual ~FFeatureDetector() override;

	void Kill();

protected:
	const TCHAR* ThreadName = TEXT("UnnamedFeatureDetectorThread");
	TSharedPtr<cv::Mat> CurrentFrame;
	
private:
	FRunnableThread* Thread = nullptr;
	bool bActive = false;
	float RefreshRate = .03f;

	FVideoReader* VideoReader = nullptr;
	double PreviousTime = 0;

public:
	FORCEINLINE bool IsActive() const { return bActive; }
	TSharedPtr<cv::Mat> GetCurrentFrame() const { return CurrentFrame; }

protected:
	virtual uint32 ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime);
	void CreateThread();

private:
	cv::Mat GetNextFrame() const;

	/**
	 * @brief Keeps track of delta-time in a thread-independent way.
	 * @return The current delta time.
	 */
	double UpdateAndGetDeltaTime();
};
