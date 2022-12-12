#pragma once

// Importing most of the OpenCV headers for convenience sake.
#include "OpenCVHelper.h"
#include "PreOpenCVHeaders.h"
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudacodec.hpp>
#include "PostOpenCVHeaders.h"
#include "Renderable.h"

class BLINKOPENCV_API FVideoReader : public FRunnable, public FRenderable
{
public:
	FVideoReader(int32 InCameraIndex, float InRefreshRate = 1.f / 30.f, FVector2D InResizeDimensions = FVector2D(),
	             const FString InWindowName = "Camera");
	FVideoReader(const FString& InVideoSource, float InRefreshRate = 1.f / 30.f,
	             FVector2D InResizeDimensions = FVector2D(), const FString InWindowName = "Video");
	
public:
	// Overriden from FRunnable
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;

	// Overriden from FRenderable
	virtual void Render() override;
	virtual void StopRendering() override;
	
	virtual ~FVideoReader() override;

private:
	// Config vars.
	int32 CameraIndex;
	std::string VideoSource;
	cv::Point ResizeDimensions;
	float RefreshRate;
	std::string WindowName;

	// State vars.
	FRunnableThread* Thread;
	bool bThreadActive;
	cv::VideoCapture VideoStream;
	TSharedPtr<cv::Mat> CurrentFrame;
	bool bVideoActive;
	double PreviousTime;
	TArray<TWeakPtr<FRenderable>> ChildRenderers;
	
public:
	/**
	 * @brief Gets the current fully-processed frame.
	 */
	cv::Mat GetFrame() const
	{
		if (CurrentFrame.IsValid() && CurrentFrame->data)
			return CurrentFrame->clone();
		return cv::Mat();
	}

	FORCEINLINE bool IsActive() const { return bThreadActive; }

protected:
	/**
	 * @brief Executed whenever a new frame has been retrieved from the VideoStream.
	 * Executes at most once per Tick.
	 * @param Frame The current frame from the VideoStream.
	 */
	virtual void ProcessNextFrame(cv::Mat& Frame);
	
	/**
	 * @brief Override this to add any logic that should be executed everytime the VideoStream has been initialised and
	 * is ready to be read from.
	 */
	virtual void Start();

	void AddChildRenderer(TWeakPtr<FRenderable> ChildRenderer);
	void RemoveChildRenderer(TWeakPtr<FRenderable> ChildRenderer);
	bool HasChildRenderer(TWeakPtr<FRenderable> ChildRenderer) const;

private:
	/**
	 * @brief Establishes the connection to the VideoStream.
	 */
	void InitialiseVideoStream();

	void PrintVideoStreamProperties() const;

	/**
	 * @brief Keeps track of delta-time in a thread-independent way.
	 * @return The current delta time.
	 */
	double UpdateAndGetDeltaTime();
};
