#include "FeatureDetector.h"
#include "BlinkOpenCV.h"
#include "VideoReader.h"

void FFeatureDetector::CreateThread()
{
	Thread = FRunnableThread::Create(this, ThreadName, 0, TPri_AboveNormal);
	checkf(Thread, TEXT("Could not create Thread '%s'"), ThreadName);
}

FFeatureDetector::FFeatureDetector(FVideoReader* InVideoReader)
{
	// Executed on game thread.

	checkf(InVideoReader, TEXT("FeatureDetector is missing a valid VideoReader"));
	VideoReader = InVideoReader;
	CurrentFrame = MakeShared<cv::Mat>();
}

bool FFeatureDetector::Init()
{
	// Executed on game thread.
	UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' has been initialised"), ThreadName);
	return true;
}

uint32 FFeatureDetector::Run()
{
	// Executed on worker thread.
	
	UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' is running"), ThreadName);
	
	bActive = true;
	
	while (IsActive())
	{
		const double DeltaTime = UpdateAndGetDeltaTime();
		#if UE_BUILD_DEBUG || UE_EDITOR
		UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' is ticking."), ThreadName);
		#endif

		if (auto NextFrame = GetNextFrame(); !NextFrame.empty())
		{
			#if UE_BUILD_DEBUG || UE_EDITOR
			const double CurrentTime = FPlatformTime::Seconds();
			UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' is processing a frame."), ThreadName);
			#endif
			
			ProcessNextFrame(NextFrame, DeltaTime);

			#if UE_BUILD_DEBUG || UE_EDITOR
			const double SecondsTook = FPlatformTime::Seconds() - CurrentTime;
			UE_LOG(LogBlinkOpenCV, Display,
				TEXT("Thread '%s' processed a frame (%fms)."), ThreadName, SecondsTook * 1000.f);
			#endif

			CurrentFrame = MakeShared<cv::Mat>(NextFrame.clone());
		}

		// Sleep until next refresh. Ensure minimum sleep time so it doesn't waste the OS resources.
		const float SleepTime = FMath::Max(.01f, RefreshRate - (FPlatformTime::Seconds() - PreviousTime));
		FPlatformProcess::Sleep(SleepTime);
	}

	UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' has stopped running."), ThreadName);

	return 0;
}

void FFeatureDetector::Exit()
{
	// Executed on worker thread.
	
	UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' is exiting."), ThreadName);
	CurrentFrame.Reset();
}

void FFeatureDetector::Stop()
{
	// Executed on game thread.
	
	UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' has been requested to stop."), ThreadName);
	bActive = false;
}

void FFeatureDetector::Render()
{
	if (IsActive())
	{
		const auto Frame = GetCurrentFrame();
		if (Frame.IsValid() && Frame->data != nullptr)
			cv::imshow(TCHAR_TO_UTF8(ThreadName), *Frame);
	}
}

void FFeatureDetector::StopRendering()
{
	cv::destroyWindow(TCHAR_TO_UTF8(ThreadName));
}

FFeatureDetector::~FFeatureDetector()
{
	Kill();
}

void FFeatureDetector::Kill()
{
	// Executed on game thread.
	
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
		Thread = nullptr;
	}
}

cv::Mat FFeatureDetector::GetNextFrame() const
{
	// Executed on worker thread.
	
	// Create a copy of the frame if it exists.
	if (const TSharedPtr<cv::Mat> Frame = VideoReader->GetFrame(); Frame.IsValid())
		return Frame->clone();
	
	return cv::Mat();
}

uint32 FFeatureDetector::ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime)
{
	// Executed on worker thread.

	// Simulate heavy work.
	FPlatformProcess::Sleep(0.1f);
	return 0;
}

double FFeatureDetector::UpdateAndGetDeltaTime()
{
	const double CurrentTime = FPlatformTime::Seconds();
	const double DeltaTime = CurrentTime - PreviousTime;
	PreviousTime = CurrentTime;
	return DeltaTime;
}
