// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#pragma once

#include "CameraReader.generated.h"

class FVideoReader;
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
	virtual void Activate(bool bReset) override;
	
public:
	/**
	 * @brief The tick rate of the VideoReader thread.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Camera", meta = (ClampMin=0.f, ClampMax=1.f))
	float VideoReaderTickRate;
	
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

	/**
	 * @brief The tick rate of the VideoReader thread.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Eyes", meta = (ClampMin=0.f, ClampMax=1.f))
	float EyeSampleRate;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Eyes", meta = (ClampMin=0.f, ClampMax=1.f))
	double BlinkResetTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Eyes", meta = (ClampMin=0.f, ClampMax=1.f))
	double WinkResetTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Eyes", meta = (ClampMin=0.f, ClampMax=1.f))
	double ConsiderAsOpenTime;


	double PreviousBlinkTime;
	double PreviousLeftWinkTime;
	double PreviousRightWinkTime;
	bool bWasOpenLast = false;

	int32 BlinkCount;
	int32 LeftWinkCount;
	int32 RightWinkCount;

	bool bVideoActive;

protected:
	FVideoReader* VideoReader;

	FTimerHandle EyeSampleTimer;

public:
	// Overriden so the VideoStream can be stopped and released upon Destroy. 
	virtual void BeginDestroy() override;
	
	// Overriden so the VideoStream can be stopped and released upon Deactivation. 
	virtual void Deactivate() override;
	
	/**
	 * @brief Output the OpenCV build info, so we know which modules are active.
	 */
	static void PrintOpenCVBuildInfo();

protected:
	void Stop();

	UFUNCTION()
	void OnEyeSampleTick();

	UFUNCTION(BlueprintNativeEvent)
	void OnBlink();

	UFUNCTION(BlueprintNativeEvent)
	void OnLeftEyeWink();

	UFUNCTION(BlueprintNativeEvent)
	void OnRightEyeWink();

	UFUNCTION(BlueprintNativeEvent)
	void OnBothOpen();

	UFUNCTION(BlueprintNativeEvent)
	void OnCameraLost();

	UFUNCTION(BlueprintNativeEvent)
	void OnCameraFound();
};