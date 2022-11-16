#include "FeatureDetector.h"

#include "BlinkOpenCV.h"
#include "CameraReader.h"

FFeatureDetector::FFeatureDetector(UCameraReader* InCameraReader)
{
	// Executed on game thread.

	checkf(InCameraReader, TEXT("FeatureDetector is missing a valid CameraReader"));
	CameraReader = InCameraReader;
	CurrentFrame = MakeShared<cv::Mat>();
	
	Thread = FRunnableThread::Create(this, ThreadName, 0, TPri_AboveNormal);
	checkf(Thread && FPlatformProcess::SupportsMultithreading(), TEXT("Could not create Thread '%s'"), ThreadName);
	if (!Thread)
	{
		UE_LOG(LogBlinkOpenCV, Warning,
			TEXT("Could not create Thread '%s' due to no multi-threading support. Will run on game thread instead!"),
			ThreadName);
	}
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

	CurrentFrame.Reset();
}

void FFeatureDetector::Tick()
{
	// Executed on game thread. Only executed when multithreading is not supported on the platform.
	UE_LOG(LogBlinkOpenCV, Warning, TEXT("Thread '%s' is running on game thread."), ThreadName);

	const double DeltaTime = UpdateAndGetDeltaTime();
	if ((ST_TimeUntilRefresh -= DeltaTime) <= 0)
	{
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
			UE_LOG(LogBlinkOpenCV, Display, TEXT("Thread '%s' processed a frame (%fms)."), ThreadName, SecondsTook * 1000.f);
			#endif

			//*CurrentFrame = NextFrame;
		}
		
		ST_TimeUntilRefresh = RefreshRate;
	}
}

cv::Mat FFeatureDetector::GetNextFrame() const
{
	// Executed on worker thread.
	return CameraReader->GetFrame();
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
