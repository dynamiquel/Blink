#pragma once

#include "PreOpenCVHeaders.h"
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/highgui.hpp"
#include "PostOpenCVHeaders.h"

#include "CameraReader.generated.h"

/**
 * @brief Use Activate() to establish the connection to a VideoStream.
 * 
 * Use Deactivate() to shutdown the connection to a VideoStream.
 *
 * The VideoStream is read once per Tick, using the interval specified in PrimaryComponentTick.TickInterval.
 */
UCLASS(BlueprintType, Blueprintable)
class BLINKOPENCV_API UCameraReader : public UActorComponent
{
	GENERATED_BODY()

protected:
	UCameraReader();
	
	virtual void BeginPlay() override;
	
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	/**
	 * @brief If enabled, the VideoStream will use a camera device as a source, otherwise, it will use a video file.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Camera")
	bool bUseCamera;
	
	/**
	 * @brief The index of the camera that is connected to the device.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Camera", meta = (EditCondition="bUseCamera", EditConditionHides))
	int32 CameraIndex;
	
	/**
	 * @brief The location of the video file to use.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Camera", meta = (EditCondition="!bUseCamera", EditConditionHides))
	FString VideoFileLocation;

	/**
	 * @brief Should the frame from the VideoStream be resized? Usually used to forcefully lower the resolution to make
	 * processing cheaper.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Camera")
	bool bResize;
	
	/**
	 * @brief The dimensions to resize the frame to. Will only resize if bResize is enabled.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Camera", meta = (EditCondition="bResize", EditConditionHides))
	FVector2D ResizeDimensions;

	/**
	 * @brief If enabled, shows the VideoStream in an external window. Useful for debugging and should be disabled
	 * for release builds.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Camera")
	bool bShowInSeparateWindow;

	/**
	 * @brief The name used for the window showing the VideoStream feed. It must be unique from other OpenCV windows.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Camera", meta = (EditCondition="bShowInSeparateWindow", EditConditionHides))
	FString WindowName;

private:
	cv::VideoCapture VideoStream;
	TSharedPtr<cv::Mat> CurrentFrame;
	bool VideoActive;

public:
	// Overriden so the VideoStream can be stopped and released upon Destroy. 
	virtual void BeginDestroy() override;
	// Overriden so the VideoStream can be stopped and released upon Deactivation. 
	virtual void Deactivate() override;
	
	/**
	 * @brief Gets the current fully-processed frame.
	 */
	cv::Mat GetFrame() const
	{
		if (CurrentFrame.IsValid() && CurrentFrame->data)
			return *CurrentFrame;
		return cv::Mat();
	}

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

	/**
	 * @brief Override this to add any logic that should be executed everytime the VideoStream is closed.
	 * I.e. releasing and cleaning up objects.
	 */
	virtual void Stop();

private:
	/**
	 * @brief Establishes the connection to the VideoStream.
	 */
	void InitialiseVideoStream();
	
	/**
	 * @brief Output the OpenCV build info, so we know which modules are active.
	 */
	static void PrintOpenCVBuildInfo();

	void PrintVideoStreamProperties() const;
};