#include "CameraReader.h"
#include "BlinkOpenCV.h"
#include "OpenCVHelper.h"
#include "PreOpenCVHeaders.h"
#include "opencv2/highgui.hpp"
#include "opencv2/core/cuda.hpp"
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudacodec.hpp>
#include "PostOpenCVHeaders.h"

UCameraReader::UCameraReader()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.f / 60.f;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bUseCamera = true;
	CameraIndex = 0;
	VideoFileLocation = TEXT("C:/Users/Liamk/Downloads/destiny2.mp4");
	bResize = true;
	ResizeDimensions = FVector2D(1280, 720);
	bShowInSeparateWindow = true;
	WindowName = TEXT("Camera");
	CurrentFrame = MakeShared<cv::Mat>();
}

void UCameraReader::BeginPlay()
{
	Super::BeginPlay();

	PrintOpenCVBuildInfo();
}

void UCameraReader::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// Called one per Tick as specified by the PrimaryComponentTick.
	// Tick is only called while this Component is Active.
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// VideoStream not active, attempt to initialise it.
	if (!VideoActive)
	{
		InitialiseVideoStream();
	}
	else
	{
		// Attempt to read the current frame in the VideoStream.
		cv::Mat TmpFrame;
		if (VideoStream.read(OUT TmpFrame))
		{
			// Frame retrieved, process it.
			ProcessNextFrame(TmpFrame);

			// Show the frame in the external window, if enabled.
			if (bShowInSeparateWindow)
				cv::imshow(TCHAR_TO_UTF8(*WindowName), TmpFrame);
		}
		else
		{
			UE_LOG(LogBlinkOpenCV, Error, TEXT("CameraReader: VideoStream could not be read"));
		}

		// Setting after ensures any thread that wants access to the video frame, only gets FULLY processed frames
		// from the CameraReader. Otherwise, it is possible for other threads to get partially processed frames.
		TmpFrame.copyTo(*CurrentFrame);
	}
}

void UCameraReader::BeginDestroy()
{
	Super::BeginDestroy();
	Stop();
}

void UCameraReader::Deactivate()
{
	Super::Deactivate();
	Stop();
}

void UCameraReader::Stop()
{
	UE_LOG(LogBlinkOpenCV, Display, TEXT("CameraReader: Stopping"));
	if (VideoActive)
	{
		VideoActive = false;
		VideoStream.release();

		if (bShowInSeparateWindow)
			cv::destroyWindow(TCHAR_TO_UTF8(*WindowName));
	}
}

void UCameraReader::InitialiseVideoStream()
{
	UE_LOG(LogBlinkOpenCV, Display, TEXT("CameraReader: Initialising VideoStream"));
	
	// Attempts to open the VideoStream with the desired video input device/file.
	// CPU decodes better than GPU for FFMPEG.
	// NVENC, which I would have liked to use, is apparently broken for the new version of OpenCV.
	bool bOpened;
	if (bUseCamera)
	{
		// Works but very low frame rate. 
		std::string GPipeline = 
		std::basic_string("autovideosrc")
		+ " ! videoconvert"
		+ " ! appsink";
		bOpened = VideoStream.open(GPipeline, cv::CAP_GSTREAMER);
	}
	else
	{
		// Works well but CPU only.
		/*std::string GPipeline = 
		"filesrc location=" + std::basic_string(TCHAR_TO_UTF8(*VideoFileLocation))
		+ " ! decodebin"
		+ " ! videoconvert"
		+ " ! appsink";*/

		// Experiment.
		std::string GPipeline = 
		"filesrc location=" + std::basic_string(TCHAR_TO_UTF8(*VideoFileLocation))
		+ " ! qtdemux name=demux demux.video_0"
		+ " ! queue"
		+ " ! h264parse"
		+ " ! omxh264dec"
		+ " ! appsink";
		
		bOpened = VideoStream.open(GPipeline, cv::CAP_GSTREAMER);

	}

	PrintVideoStreamProperties();
	
	if (bOpened)
	{
		if (!VideoStream.grab())
		{
			UE_LOG(LogBlinkOpenCV, Error, TEXT("CameraReader: VideoStream could not be grabbed"));
		}
		
		cv::Mat Frame;
		if (VideoStream.retrieve(Frame))
		{
			UE_LOG(LogBlinkOpenCV, Display, TEXT("CameraReader: VideoStream can be read"));
		}
		else
		{
			UE_LOG(LogBlinkOpenCV, Error, TEXT("CameraReader: VideoStream cannot be read"));
		}
		
		Start();
	}
	else
	{
		UE_LOG(LogBlinkOpenCV, Error,
			TEXT("CameraReader: VideoStream could not be initialised. Does it exist or is it being used by another program?"));
	}
}

void UCameraReader::PrintOpenCVBuildInfo()
{
	UE_LOG(LogBlinkOpenCV, Display, TEXT("%s"), *FString(cv::getBuildInformation().c_str()));
}

void UCameraReader::PrintVideoStreamProperties() const
{
	FString Output = FString::Printf(TEXT("Backend: %s"), *FString(VideoStream.getBackendName().c_str()));
	Output.Appendf(TEXT("\nHeight: %d"), (int32)VideoStream.get(cv::CAP_PROP_FRAME_HEIGHT));
	Output.Appendf(TEXT("\nWidth: %d"), (int32)VideoStream.get(cv::CAP_PROP_FRAME_WIDTH));
	Output.Appendf(TEXT("\nFPS: %f"), VideoStream.get(cv::CAP_PROP_FPS));
	Output.Appendf(TEXT("\nFormat: %d"), (int32)VideoStream.get(cv::CAP_PROP_FORMAT));
	Output.Appendf(TEXT("\nGPU Acceleration: %d"), (int32)VideoStream.get(cv::CAP_PROP_HW_ACCELERATION));
	
	// Convert double to 4 char array.
	int32 Codec = VideoStream.get(cv::CAP_PROP_FOURCC);
	//char* CodecCharPtr[4];
	//std::memcpy(CodecCharPtr, &Codec, 4);
	//const char* CodecCharPtr = (reinterpret_cast<char*>(&Codec));
	Output.Appendf(TEXT("\nCodec: %d %d %d %d"), Codec & 255, (Codec >> 8) & 255, (Codec >> 16) & 255, (Codec >> 24) & 255);
	//delete CodecCharPtr[4];
	UE_LOG(LogBlinkOpenCV, Display, TEXT("%s"), *Output);
}

void UCameraReader::ProcessNextFrame(cv::Mat& Frame)
{
	if (bResize && ResizeDimensions.X > 2 && ResizeDimensions.Y > 2)
	{
		const auto Resolution = cv::Size(ResizeDimensions.X, ResizeDimensions.Y);
		cv::cuda::GpuMat CMat;
		CMat.upload(Frame);
		cv::cuda::resize(CMat, CMat, Resolution);
		CMat.download(Frame);
	}
}

void UCameraReader::Start()
{
	UE_LOG(LogBlinkOpenCV, Display, TEXT("CameraReader: Started (camera initialised)"));
	
	if (bShowInSeparateWindow)
	{
		const std::string NativeWindowName = TCHAR_TO_UTF8(*WindowName);
		cv::namedWindow(NativeWindowName);
		cv::resizeWindow(NativeWindowName, 1280, 720);
	}
	
	VideoActive = true;
}
