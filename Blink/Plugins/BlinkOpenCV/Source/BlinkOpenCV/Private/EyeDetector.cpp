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
	Src.download(Frame);
	
	EEyeStatus EyeStatus = GetEyeStatusFromFrame(Frame);

	return 0;
}

std::vector<cv::Rect> FEyeDetector::GetFaces(const cv::Mat& Frame) const
{
	checkf(FaceClassifier, TEXT("The OpenCV Face cascade filter is null."));
	
	std::vector<cv::Rect> Faces;
	FaceClassifier->detectMultiScale(Frame, OUT Faces, 1.3f, 5,
		cv::CASCADE_FIND_BIGGEST_OBJECT,
		cv::Size(MinFaceSize, MinFaceSize));
	return Faces;
}

void FEyeDetector::GetEyes(const cv::Mat& Frame, const cv::Rect& Face, cv::Rect& LeftEye, cv::Rect& RightEye) const
{
	checkf(EyeClassifier, TEXT("The OpenCV Eye cascade filter is null."));

	// Trim the Face rectangle to a small part where the eyes are typically located.
	// Saves processing time and reduces false positives.
	cv::Rect LeftEyeArea, RightEyeArea;
	TrimFaceToEyes(Face, OUT LeftEyeArea, OUT RightEyeArea);

	DrawEyeArea(Frame, LeftEyeArea);
	DrawEyeArea(Frame, RightEyeArea);
	
	auto FaceRoi = Frame(LeftEyeArea);
	std::vector<cv::Rect> LeftEyes;
	EyeClassifier->detectMultiScale(
		FaceRoi,
		OUT LeftEyes,
		1.1,
		2,
		cv::CASCADE_SCALE_IMAGE,
		cv::Size(MinEyeSize, MinEyeSize));

	FaceRoi = Frame(RightEyeArea);
	std::vector<cv::Rect> RightEyes;
	EyeClassifier->detectMultiScale(
		FaceRoi,
		OUT RightEyes,
		1.1,
		2,
		cv::CASCADE_SCALE_IMAGE,
		cv::Size(MinEyeSize, MinEyeSize));

	DrawPreFilteredEyes(Frame, LeftEyeArea, LeftEyes);
	DrawPreFilteredEyes(Frame, RightEyeArea, RightEyes);
	
	FilterEyes(IN OUT LeftEyes, IN OUT RightEyes, Face);

	if (LeftEyes.size() > 0)
		LeftEye = LeftEyes[0];
	if (RightEyes.size() > 0)
		RightEye = RightEyes[0];

	DrawEye(Frame, LeftEyeArea, LeftEye);
	DrawEye(Frame, RightEyeArea, RightEye);
}

void FEyeDetector::TrimFaceToEyes(const cv::Rect& Face, cv::Rect& LeftEyeArea, cv::Rect& RightEyeArea)
{
	// Trim the Face rect to roughly only include the eyes area.
	
	LeftEyeArea = Face;
	// Move top bar down a little.
	LeftEyeArea.y += LeftEyeArea.height * .25f;
	// Move bottom bar up a lot.
	LeftEyeArea.height *= .25f;
	// Move left bar right a little.
	LeftEyeArea.x += LeftEyeArea.width * .17f;
	// Move right bar left a lot.
	LeftEyeArea.width *= .3f;

	RightEyeArea = LeftEyeArea;
	// Move left bar right a lot.
	RightEyeArea.x = Face.x + (Face.width * .83f) - RightEyeArea.width;
}

void FEyeDetector::FilterEyes(std::vector<cv::Rect>& LeftEyes, std::vector<cv::Rect>& RightEyes, const cv::Rect& Face)
{
	// Basic algorithm to determine which eyes are the real ones.

	// Reverse loop so array can be modified directly.
	for (int32 i = LeftEyes.size() - 1; i >= 0; i--)
	{
		// If eye size is abnormally too large compared to face, discard it.
		if (IsEyeTooLarge(LeftEyes[i], Face))
		{
			LeftEyes.pop_back();
			continue;
		}
	}
	
	for (int32 i = RightEyes.size() - 1; i >= 0; i--)
	{
		// If eye size is abnormally too large compared to face, discard it.
		if (IsEyeTooLarge(RightEyes[i], Face))
		{
			RightEyes.pop_back();
			continue;
		}
	}

	// If more than two eyes in one area, find which one is more likely to be the real one by comparing it so the size
	// of the eyes in the other eye area.
	// (Could compare histogram as eyes are typically symmetrical).
	// (Could compare using previous left/right eye central position as it should be relatively close).
	if ((LeftEyes.size() > 0 && RightEyes.size() > 1) || (LeftEyes.size() > 1 && RightEyes.size() > 0))
	{
		int32 BestLeftIndex = 0, BestRightIndex = 0;
		float ClosestMatch = FLT_MAX;

		for (int32 Left = 0; Left < LeftEyes.size(); Left++)
		{
			for (int32 Right = 0; Right < RightEyes.size(); Right++)
			{
				float Diff = FMath::Abs((float)LeftEyes[Left].area() - (float)RightEyes[Right].area());
				
				if (Diff < ClosestMatch)
				{
					ClosestMatch = Diff;
					BestLeftIndex = Left;
					BestRightIndex = Right;
				}
			}
		}

		// Create copy of best eyes, empty the entire eyes arrays, then add back the best eyes.
		auto BestLeftEye = LeftEyes[BestLeftIndex];
		auto BestRightEye = RightEyes[BestRightIndex];
		LeftEyes.clear();
		LeftEyes.emplace_back(BestLeftEye);
		RightEyes.clear();
		RightEyes.emplace_back(BestRightEye);
	}
}

bool FEyeDetector::IsEyeTooLarge(const cv::Rect& Eye, const cv::Rect& Face)
{
	const float EyeProportion = (float)Eye.area() / (float)Face.area();
	return EyeProportion >= .1f;
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

void FEyeDetector::DrawEyeArea(const cv::Mat& Frame, const cv::Rect& EyeArea) const
{
	cv::rectangle(Frame,
			{EyeArea.x, EyeArea.y}, {EyeArea.x + EyeArea.width, EyeArea.y + EyeArea.height},
			{125, 255, 0}, 2);
}

void FEyeDetector::DrawPreFilteredEyes(const cv::Mat& Frame, const cv::Rect& Face, const std::vector<cv::Rect>& Eyes) const
{
	// Show eyes on frame as a rectangle.
	for (auto Eye : Eyes)
	{
		cv::rectangle(Frame(Face),
			{Eye.x, Eye.y}, {Eye.x + Eye.width, Eye.y + Eye.height},
			{0, 255, 255}, 1);
	}
}

void FEyeDetector::DrawEye(const cv::Mat& Frame, const cv::Rect& EyeArea, const cv::Rect& Eye) const
{
	const auto EyeCentre = cv::Point(Eye.x + Eye.width / 2, Eye.y + Eye.height / 2);
	const int32 Radius = FMath::RoundToInt((Eye.width + Eye.height) * .15f);
	cv::circle(Frame(EyeArea), EyeCentre, Radius, {150, 255, 255}, 1);
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
	const std::vector<cv::Rect> Faces = GetFaces(Frame);
	DrawFaces(Frame, Faces);
	
	// Treat more or less than one face found as an error.
	// Could be further developed to deal with multiple faces.
	if (Faces.size() != 1)
		return EEyeStatus::Error;
	
	// Get eyes.
	cv::Rect LeftEye, RightEye;
	GetEyes(Frame, Faces[0], OUT LeftEye, OUT RightEye);
	//DrawEyes(Frame, Faces[0], Eyes);
	
	// Treat no eyes found as a blink.
	/*if (Eyes.size() == 0)
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
