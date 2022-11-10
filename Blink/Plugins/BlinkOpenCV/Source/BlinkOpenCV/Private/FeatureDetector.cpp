#include "FeatureDetector.h"

#include "BlinkOpenCV.h"

FFeatureDetector::FFeatureDetector()
{
	// Executed on game thread.
	
	Thread = FRunnableThread::Create(this, ThreadName, 0, TPri_AboveNormal);
	checkf(Thread && FPlatformProcess::SupportsMultithreading(), TEXT("Could not create Thread '%s'."), ThreadName);
	if (!Thread)
	{
		UE_LOG(LogBlinkOpenCV, Warning,
			TEXT("Could not create Thread '%s' due to no multi-threading support. Will run on game thread!"), ThreadName);
	}
}

bool FFeatureDetector::Init()
{
	// Executed on game thread.
	
	UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' has been initialised."), ThreadName);
	return true;
}

uint32 FFeatureDetector::Run()
{
	// Executed on worker thread.
	
	UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' is running."), ThreadName);
	
	bActive = true;
	
	while (IsActive())
	{
		const double DeltaTime = UpdateAndGetDeltaTime();
		#if UE_BUILD_DEBUG
		UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' is ticking."), ThreadName);
		#endif

		if (cv::Mat NextFrame = GetNextFrame(); !NextFrame.empty())
		{
			#if UE_BUILD_DEBUG
			const double CurrentTime = FPlatformTime::Seconds();
			UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' is processing a frame."), ThreadName);
			#endif
			
			ProcessNextFrame(NextFrame, DeltaTime);

			#if UE_BUILD_DEBUG
			const double SecondsTook = FPlatformTime::Seconds() - CurrentTime;
			UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' processed a frame (%fms)."), ThreadName, SecondsTook * 1000.f);
			#endif
		}

		// Sleep until next refresh. Ensure minimum sleep time so it doesn't waste the OS resources.
		const float SleepTime = FMath::Max(.01f, (1.f / RefreshRate) - DeltaTime);
		// Not sure which one I'm supposed to use.
		//const float SleepTime = FMath::Max(.01f, (1.f / RefreshRate) - GWorld->RealTimeSeconds - PreviousTime);
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
}

void FFeatureDetector::Tick()
{
	// Executed on game thread. Only executed when multithreading is not supported on the platform.
	UE_LOG(LogBlinkOpenCV, Warning, TEXT("Thread '%s' is running on game thread."), ThreadName);

	const double DeltaTime = UpdateAndGetDeltaTime();
	if ((ST_TimeUntilRefresh -= DeltaTime) <= 0)
	{
		#if UE_BUILD_DEBUG
		UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' is ticking."), ThreadName);
		#endif
		
		if (cv::Mat NextFrame = GetNextFrame(); !NextFrame.empty())
		{
			#if UE_BUILD_DEBUG
			const double CurrentTime = FPlatformTime::Seconds();
			UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' is processing a frame."), ThreadName);
			#endif
			
			ProcessNextFrame(NextFrame, DeltaTime);

			#if UE_BUILD_DEBUG
			const double SecondsTook = FPlatformTime::Seconds() - CurrentTime;
			UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' processed a frame (%fms)."), ThreadName, SecondsTook * 1000.f);
			#endif
		}
		
		ST_TimeUntilRefresh = 1 / RefreshRate;
	}
}

void FFeatureDetector::QueueNewFrame(const cv::Mat& Frame)
{
	// Executed on game thread.
	QueuedFrames.Enqueue(Frame);
}

cv::Mat FFeatureDetector::GetNextFrame()
{
	// Executed on worker thread.
	cv::Mat PossibleFrame;
	QueuedFrames.Dequeue(OUT PossibleFrame);
	return PossibleFrame;
}

uint32 FFeatureDetector::ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime)
{
	// Executed on worker thread.

	// Simulate heavy work.
	FPlatformProcess::Sleep(0.2f);
	return 0;
}

double FFeatureDetector::UpdateAndGetDeltaTime()
{
	const double CurrentTime = FPlatformTime::Seconds();
	const double DeltaTime = CurrentTime - PreviousTime;
	PreviousTime = CurrentTime;
	return DeltaTime;
}
