// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#include "VideoReader.h"
#include "BlinkOpenCV.h"

FVideoReader::FVideoReader(int32 InCameraIndex, float InRefreshRate, FVector2D InResizeDimensions, const FString InWindowName)
{
	// Executed on game thread.
	
	checkf(InCameraIndex >= 0, TEXT("VideoReader: Provided an invalid camera index. Index: %d"), InCameraIndex);
	
	CameraIndex = InCameraIndex;
	RefreshRate = InRefreshRate;
	ResizeDimensions = cv::Point(InResizeDimensions.X, InResizeDimensions.Y);
	bVideoActive = false;
	CurrentFrame = MakeShared<cv::Mat>();
	PreviousTime = 0;
	WindowName = TCHAR_TO_UTF8(*InWindowName);

	Thread = FRunnableThread::Create(this, TEXT("VideoReader"), 0, TPri_AboveNormal);
	checkf(Thread, TEXT("Could not create Thread '%s'"), TEXT("VideoReader"));
}

FVideoReader::FVideoReader(const FString& InVideoSource, float InRefreshRate, FVector2D InResizeDimensions, const FString InWindowName)
{
	// Executed on game thread.
	
	VideoSource = TCHAR_TO_UTF8(*InVideoSource);
	RefreshRate = InRefreshRate;
	ResizeDimensions = cv::Point(InResizeDimensions.X, InResizeDimensions.Y);
	bVideoActive = false;
	CurrentFrame = MakeShared<cv::Mat>();
	PreviousTime = 0;
	WindowName = TCHAR_TO_UTF8(*InWindowName);

	Thread = FRunnableThread::Create(this, TEXT("VideoReader"), 0, TPri_AboveNormal);
	checkf(Thread, TEXT("Could not create Thread '%s'"), TEXT("VideoReader"));
}

bool FVideoReader::Init()
{
	// Executed on game thread.
	
	UE_LOG(LogBlinkOpenCV, Display, TEXT("VideoReader: Initialised"));
	return true;
}

uint32 FVideoReader::Run()
{
	// Executed on worker thread.
	
	UE_LOG(LogBlinkOpenCV, Display, TEXT("VideoReader: Running"));

	bThreadActive = true;
	
	while (IsActive())
	{
		const double DeltaTime = UpdateAndGetDeltaTime();
		#if UE_BUILD_DEBUG || UE_EDITOR
		UE_LOG(LogBlinkOpenCV, Display, TEXT("VideoReader: Ticking"));
		#endif
		
		// VideoStream not active, attempt to initialise it.
		if (!bVideoActive)
		{
			InitialiseVideoStream();
		}
		else
		{
			// Attempt to read the current frame in the VideoStream.
			// Note: It takes a few seconds for the Video Stream to return an empty frame.
			cv::Mat TmpFrame;
			if (VideoStream.read(OUT TmpFrame))
			{
				#if UE_BUILD_DEBUG || UE_EDITOR
				const double CurrentTime = FPlatformTime::Seconds();
				UE_LOG(LogBlinkOpenCV, Display, TEXT("VideoReader: Processing a frame"));
				#endif
				
				// Frame retrieved, process it.
				ProcessNextFrame(TmpFrame);

				#if UE_BUILD_DEBUG || UE_EDITOR
				const double SecondsTook = FPlatformTime::Seconds() - CurrentTime;
				UE_LOG(LogBlinkOpenCV, Display,
					TEXT("VideoReader: Processed a frame in %fms"), SecondsTook * 1000.f);
				#endif

				// Setting after ensures any thread that wants access to the video frame, only gets FULLY processed frames
				// from the CameraReader. Otherwise, it is possible for other threads to get partially processed frames.
				CurrentFrame = MakeShared<cv::Mat>(TmpFrame);
			}
			else
			{
				UE_LOG(LogBlinkOpenCV, Error, TEXT("VideoReader: VideoStream could not be read"));
				bVideoActive = false;
				CurrentFrame.Reset();
			}
		}

		// Sleep until next refresh. Ensure minimum sleep time so it doesn't waste the OS resources.
		const float SleepTime = FMath::Max(0.01f, RefreshRate - (FPlatformTime::Seconds() - PreviousTime));
		FPlatformProcess::Sleep(SleepTime);
	}

	return true;
}

void FVideoReader::Exit()
{
	// Executed on worker thread.
	
	UE_LOG(LogBlinkOpenCV, Display, TEXT("VideoReader: Exiting"));
	if (bVideoActive)
	{
		bVideoActive = false;
		VideoStream.release();
	}
	
	CurrentFrame.Reset();
}

void FVideoReader::Stop()
{
	// Executed on game thread.
	
	UE_LOG(LogBlinkOpenCV, Display, TEXT("VideoReader: Requested to stop."));
	bThreadActive = false;
}

void FVideoReader::Render()
{
	if (IsActive())
	{
		const auto Frame = GetFrame();
		if (Frame.IsValid() && Frame->data != nullptr)
			cv::imshow(WindowName, *Frame);

		// Render any child renderers.
		for (const auto ChildRenderer : ChildRenderers)
			if (const auto LockedChildRenderer = ChildRenderer.Pin(); LockedChildRenderer.IsValid())
				LockedChildRenderer->Render();
	}
}

void FVideoReader::StopRendering()
{
	cv::destroyWindow(WindowName);
	for (const auto ChildRenderer : ChildRenderers)
		if (const auto LockedChildRenderer = ChildRenderer.Pin())
			LockedChildRenderer->StopRendering();
}

FVideoReader::~FVideoReader()
{
	// Executed on game thread.
	
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
		Thread = nullptr;
	}
}

void FVideoReader::ProcessNextFrame(cv::Mat& Frame)
{
	// Executed on worker thread.
	
	/*if (bResize && ResizeDimensions.X > 2 && ResizeDimensions.Y > 2)
	{
		const auto Resolution = cv::Size(ResizeDimensions.X, ResizeDimensions.Y);
		cv::cuda::GpuMat CMat;
		CMat.upload(Frame);
		cv::cuda::resize(CMat, CMat, Resolution);
		CMat.download(Frame);
	}*/
}

void FVideoReader::Start()
{
	// Executed on worker thread.
	
	UE_LOG(LogBlinkOpenCV, Display, TEXT("VideoaReader: Started (camera initialised)"));	
	bVideoActive = true;
}

void FVideoReader::AddChildRenderer(TWeakPtr<FRenderable> ChildRenderer)
{
	ChildRenderers.AddUnique(ChildRenderer);
}

void FVideoReader::RemoveChildRenderer(TWeakPtr<FRenderable> ChildRenderer)
{
	ChildRenderers.Remove(ChildRenderer);
}

bool FVideoReader::HasChildRenderer(TWeakPtr<FRenderable> ChildRenderer) const
{
	return ChildRenderers.Contains(ChildRenderer);
}

void FVideoReader::InitialiseVideoStream()
{
	// Executed on worker thread.
	
	UE_LOG(LogBlinkOpenCV, Display, TEXT("VideoReader: Initialising VideoStream"));
	
	// Attempts to open the VideoStream with the desired video input device/file.
	bool bOpened;
	if (VideoSource.empty())
	{
		// Works but very low frame rate and long start-up times.
		// Doesn't use any hardware acceleration.
		// Performance could be increased by using hardware acceleration (ksvideosrc and NVENC).
		std::string GPipeline = 
		std::basic_string("ksvideosrc")
		+ " ! decodebin"
		+ " ! videoconvert"
		+ " ! appsink";
		bOpened = VideoStream.open(GPipeline, cv::CAP_GSTREAMER);
	}
	else
	{
		// Works well and is automatically using some type of hardware acceleration.
		std::string GPipeline = 
		"filesrc location=" + std::basic_string(VideoSource)
		+ " ! decodebin"
		+ " ! videoconvert"
		+ " ! appsink";
		
		bOpened = VideoStream.open(GPipeline, cv::CAP_GSTREAMER);
	}
	
	if (bOpened)
	{
		if (!VideoStream.grab())
		{
			UE_LOG(LogBlinkOpenCV, Error, TEXT("VideoReader: VideoStream could not be grabbed"));
		}

		PrintVideoStreamProperties();
		Start();
	}
	else
	{
		UE_LOG(LogBlinkOpenCV, Error,
			TEXT("VideoReader: VideoStream could not be initialised. Does it exist or is it being used by another program?"));

		// Prevent constantly trying to initialise the camera as it can cause a lot of stutter.
		FPlatformProcess::Sleep(1.f);
	}
}

void FVideoReader::PrintVideoStreamProperties() const
{
	// Executed on worker thread.
	
	FString Output = FString::Printf(TEXT("Backend: %s"), *FString(VideoStream.getBackendName().c_str()));
	Output.Appendf(TEXT("\nHeight: %d"), (int32)VideoStream.get(cv::CAP_PROP_FRAME_HEIGHT));
	Output.Appendf(TEXT("\nWidth: %d"), (int32)VideoStream.get(cv::CAP_PROP_FRAME_WIDTH));
	Output.Appendf(TEXT("\nFPS: %f"), VideoStream.get(cv::CAP_PROP_FPS));
	Output.Appendf(TEXT("\nFormat: %d"), (int32)VideoStream.get(cv::CAP_PROP_FORMAT));
	Output.Appendf(TEXT("\nGPU Acceleration: %d"), (int32)VideoStream.get(cv::CAP_PROP_HW_ACCELERATION));
	
	// Convert double to 4 char array.
	int32 Codec = VideoStream.get(cv::CAP_PROP_FOURCC);
	Output.Appendf(TEXT("\nCodec: %c %c %c %c"), Codec & 255, (Codec >> 8) & 255, (Codec >> 16) & 255, (Codec >> 24) & 255);
	UE_LOG(LogBlinkOpenCV, Display, TEXT("%s"), *Output);
}

double FVideoReader::UpdateAndGetDeltaTime()
{
	// Executed on worker thread.
	
	const double CurrentTime = FPlatformTime::Seconds();
	const double DeltaTime = CurrentTime - PreviousTime;
	PreviousTime = CurrentTime;
	return DeltaTime;
}