#include "EyeDetector.h"

#include "PreOpenCVHeaders.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/core/cuda.hpp"
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaoptflow.hpp>
#include <opencv2/cudaobjdetect.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include "PostOpenCVHeaders.h"

FEyeDetector::FEyeDetector(FVideoReader* InVideoReader)
	: FFeatureDetector(InVideoReader)
{
	ThreadName = TEXT("EyeDetectorThread");
	
	FString CascadeDirectory =
    FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("BlinkOpenCV"), TEXT("Content"), TEXT("Cascades"));
    
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	FString Result;

	// Load the Face cascade filter.
	FString CascadeFilePath = FPaths::Combine(CascadeDirectory, TEXT("haarcascade_frontalface_default.xml"));
	checkf(FileManager.FileExists(*CascadeFilePath), TEXT("The OpenCV Face cascade filter does not exist"));
		
	//FaceClassifier = cv::cuda::CascadeClassifier::create(TCHAR_TO_UTF8(*CascadeFilePath));
	FaceClassifier = cv::makePtr<cv::CascadeClassifier>(TCHAR_TO_UTF8(*CascadeFilePath));
	
	// Load the Eye cascade filter.
	CascadeFilePath = FPaths::Combine(CascadeDirectory, TEXT("haarcascade_eye.xml"));
	checkf(FileManager.FileExists(*CascadeFilePath), TEXT("The OpenCV Eye cascade filter does not exist"));

	EyeClassifier = cv::makePtr<cv::CascadeClassifier>(TCHAR_TO_UTF8(*CascadeFilePath));
}

uint32 FEyeDetector::ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime)
{
	cv::cuda::GpuMat Src;
	Src.upload(Frame);
	
	cv::cuda::cvtColor(Src, Src, cv::COLOR_BGR2GRAY);

	EEyeStatus EyeStatus = GetEyeStatusFromFrame(Frame);
	
	const auto Canny = cv::cuda::createCannyEdgeDetector(100, 200);
	Canny->detect(Src, Src);
	
	Src.download(Frame);

	return 0;
}

std::vector<cv::Rect> FEyeDetector::GetFaces(const cv::Mat& Frame) const
{
	checkf(FaceClassifier, TEXT("The OpenCV Face cascade filter is null."));
	
	std::vector<cv::Rect> Faces;
	FaceClassifier->detectMultiScale(Frame, OUT Faces, 1.3f, 5);
	return Faces;
}

std::vector<cv::Rect> FEyeDetector::GetEyes(const cv::Mat& Frame, const cv::Rect& Face) const
{
	checkf(EyeClassifier, TEXT("The OpenCV Eye cascade filter is null."));
	
	const auto FaceRoi = Frame(Face);

	std::vector<cv::Rect> Eyes;
	EyeClassifier->detectMultiScale(
		FaceRoi,
		OUT Eyes,
		1.1,
		2,
		cv::CASCADE_SCALE_IMAGE,
		cv::Size(MinEyeSize, MinEyeSize));

	return Eyes;
}

void FEyeDetector::DrawFaces(const cv::Mat& Frame, const std::vector<cv::Rect>& Faces) const
{
	// Show faces on frame as a rectangle.
	for (auto Face : Faces)
	{
		cv::rectangle(Frame,
			{Face.x, Face.y}, {Face.x + Face.width, Face.y + Face.height},
			{255, 255, 0}, 2);
	}
}

void FEyeDetector::DrawEyes(const cv::Mat& Frame, const std::vector<cv::Rect>& Eyes) const
{
	// Show eyes on frame as a rectangle.
	for (auto Eye : Eyes)
	{
		cv::rectangle(Frame,
			{Eye.x, Eye.y}, {Eye.x + Eye.width, Eye.y + Eye.height},
			{0, 255, 255}, 1);
	}
}

FVector2D FEyeDetector::GetEyeAspectRatios(const std::vector<cv::Rect>& Eyes)
{
	FVector2D EyeAspectRatios;
	
	if (Eyes.size() > 0)
		EyeAspectRatios.X = (float)Eyes[0].width / (float)Eyes[0].height;
	if (Eyes.size() > 1)
		EyeAspectRatios.Y = (float)Eyes[1].width / (float)Eyes[1].height;

	return EyeAspectRatios;
}

int32 FEyeDetector::GetEyesUnderAspectRatioThreshold(const FVector2D& EyesAspectRatios) const
{
	int32 UnderAspectRatioThreshold = 0;

	if (EyesAspectRatios.X < EyeClosedAspectRatioThreshold)
		UnderAspectRatioThreshold++;
	if (EyesAspectRatios.Y < EyeClosedAspectRatioThreshold)
		UnderAspectRatioThreshold++;

	return UnderAspectRatioThreshold;
}

EEyeStatus FEyeDetector::GetEyeStatusFromFrame(const cv::Mat& Frame)
{
	// Get faces.
	/*const std::vector<cv::Rect> Faces = GetFaces(Frame);
	DrawFaces(Frame, Faces);
	
	// Treat more or less than one face found as an error.
	// Could be further developed to deal with multiple faces.
	if (Faces.size() != 1)
		return EEyeStatus::Error;
	
	// Get eyes.
	const std::vector<cv::Rect> Eyes = GetEyes(Frame, Faces[0]);
	DrawEyes(Frame, Eyes);
	
	// Treat no eyes found as a blink.
	if (Eyes.size() == 0)
		return EEyeStatus::Blink;
	
	// Get the aspect ratio of the eyes and determine if its a closed eye.
	const FVector2D EyeAspectRatios = GetEyeAspectRatios(Eyes);
	int32 NumClosedEyes = GetEyesUnderAspectRatioThreshold(EyeAspectRatios);
	
	switch (NumClosedEyes)
	{
		// If both eyes are found and above the closed eye threshold, don't do anything.
		case 0:
			return EEyeStatus::BothOpen;
		// If one eye cannot be found or is under the closed eye threshold, treat as a wink.
		case 1:
			return EEyeStatus::Wink;
		// Else, treat as a blink.
		default:
			return EEyeStatus::Blink;
	}*/
	return EEyeStatus::BothOpen;
}
