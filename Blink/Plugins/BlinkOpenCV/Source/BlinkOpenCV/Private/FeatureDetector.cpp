#include "FeatureDetector.h"
#include "BlinkOpenCV.h"
#include "CameraReader.h"
#include "VideoReader.h"

FFeatureDetector::FFeatureDetector(FVideoReader* InVideoReader)
{
	// Executed on game thread.

	checkf(InVideoReader, TEXT("FeatureDetector is missing a valid VideoReader"));
	VideoReader = InVideoReader;
	CurrentFrame = MakeShared<cv::Mat>();
	
	Thread = FRunnableThread::Create(this, ThreadName, 0, TPri_AboveNormal);
	checkf(Thread, TEXT("Could not create Thread '%s'"), ThreadName);
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

			NextFrame.copyTo(*CurrentFrame);
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
}

void FFeatureDetector::Stop()
{
	// Executed on game thread.
	
	UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' has been requested to stop."), ThreadName);
	bActive = false;
}

FFeatureDetector::~FFeatureDetector()
{
	// Executed on game thread.
	
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
	}

	CurrentFrame.Reset();
}

cv::Mat FFeatureDetector::GetNextFrame() const
{
	// Executed on worker thread.
	return VideoReader->GetFrame();
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
