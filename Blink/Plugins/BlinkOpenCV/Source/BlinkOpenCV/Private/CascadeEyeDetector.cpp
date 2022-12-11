#include "CascadeEyeDetector.h"

#include "PreOpenCVHeaders.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/core/cuda.hpp"
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaoptflow.hpp>
#include <opencv2/cudaobjdetect.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudafeatures2d.hpp>

#include "BlinkOpenCV.h"
#include "PostOpenCVHeaders.h"

FCascadeEyeDetector::FCascadeEyeDetector(FVideoReader* InVideoReader)
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

uint32 FCascadeEyeDetector::ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime)
{
	cv::cuda::GpuMat Src;
	Src.upload(Frame);
	cv::cuda::cvtColor(Src, Src, cv::COLOR_BGR2GRAY);
	Src.download(Frame);

	// Get the assumed eye status from frame.
	const EEyeStatus FrameEyeStatus = GetEyeStatusFromFrame(Frame);
	
	UpdateEyeState(FrameEyeStatus, DeltaTime);
	
	// Do additional processing to determine the actual eye status by taking errors into account.
	const EEyeStatus ErroredEyeStatus = GetEyeStatusWithError(FrameEyeStatus);

	UE_LOG(LogBlinkOpenCV, Warning, TEXT("State: %s"), *UEnum::GetValueAsString(FrameEyeStatus));
	UE_LOG(LogBlinkOpenCV, Error, TEXT("State: %s"), *UEnum::GetValueAsString(ErroredEyeStatus));
	return 0;
}

cv::Rect FCascadeEyeDetector::GetFace(const cv::Mat& Frame) const
{
	checkf(FaceClassifier, TEXT("The OpenCV Face cascade filter is null."));

	// Finds potential faces from frame.
	std::vector<cv::Rect> Faces;
	FaceClassifier->detectMultiScale(Frame, OUT Faces, 1.3f, 5,
		cv::CASCADE_FIND_BIGGEST_OBJECT,
		cv::Size(MinFaceSize, MinFaceSize));

	DrawPreFilteredFaces(Frame, Faces);

	FilterFaces(Frame, IN OUT Faces);

	// This will most likely be the real face.
	if (Faces.size() > 0)
	{
		DrawFace(Frame, Faces[0]);
		return Faces[0];
	}
	
	return cv::Rect();
}

void FCascadeEyeDetector::FilterFaces(const cv::Mat& Frame, std::vector<cv::Rect>& Faces) const
{
	if (Faces.size() < 2)
		return;
	
	// If multiple faces detected, choose the biggest face as this will most likely be the real one.
	int32 BiggestFaceIndex = 0;
	float BiggestArea = 0;
	for (int32 i = 0; i < Faces.size(); i++)
	{
		const float Area = Faces[i].area();
		if (Area > BiggestArea)
		{
			BiggestFaceIndex = i;
			BiggestArea = Area;
		}
	}

	cv::Rect BiggestFace = Faces[BiggestFaceIndex];
	Faces.clear();
	Faces.emplace_back(BiggestFace);
}

void FCascadeEyeDetector::GetEyes(const cv::Mat& Frame, const cv::Rect& Face, cv::Rect& LeftEye, cv::Rect& RightEye) const
{
	checkf(EyeClassifier, TEXT("The OpenCV Eye cascade filter is null."));

	// Trim the Face rectangle to a small part where the eyes are typically located.
	// Saves processing time and reduces false positives.
	cv::Rect LeftEyeArea, RightEyeArea;
	TrimFaceToEyes(Face, OUT LeftEyeArea, OUT RightEyeArea);

	DrawEyeArea(Frame, LeftEyeArea);
	DrawEyeArea(Frame, RightEyeArea);

	// Search for eyes in the calculated Left Eye Area.
	auto FaceRoi = Frame(LeftEyeArea);
	std::vector<cv::Rect> LeftEyes;
	EyeClassifier->detectMultiScale(
		FaceRoi,
		OUT LeftEyes,
		1.3,
		2,
		cv::CASCADE_SCALE_IMAGE,
		cv::Size(MinEyeSize, MinEyeSize));

	// Search for eyes in the calculated Right Eye Area.
	FaceRoi = Frame(RightEyeArea);
	std::vector<cv::Rect> RightEyes;
	EyeClassifier->detectMultiScale(
		FaceRoi,
		OUT RightEyes,
		1.3,
		2,
		cv::CASCADE_SCALE_IMAGE,
		cv::Size(MinEyeSize, MinEyeSize));

	DrawPreFilteredEyes(Frame, LeftEyeArea, LeftEyes);
	DrawPreFilteredEyes(Frame, RightEyeArea, RightEyes);

	// Remove eyes that are likely false positives.
	FilterEyes(IN OUT LeftEyes, IN OUT RightEyes, Face);

	if (LeftEyes.size() > 0)
		LeftEye = LeftEyes[0];
	if (RightEyes.size() > 0)
		RightEye = RightEyes[0];

	DrawEye(Frame, LeftEyeArea, LeftEye);
	DrawEye(Frame, RightEyeArea, RightEye);
}

void FCascadeEyeDetector::TrimFaceToEyes(const cv::Rect& Face, cv::Rect& LeftEyeArea, cv::Rect& RightEyeArea)
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

void FCascadeEyeDetector::FilterEyes(std::vector<cv::Rect>& LeftEyes, std::vector<cv::Rect>& RightEyes, const cv::Rect& Face) const
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

bool FCascadeEyeDetector::IsEyeTooLarge(const cv::Rect& Eye, const cv::Rect& Face)
{
	const float EyeProportion = (float)Eye.area() / (float)Face.area();
	return EyeProportion >= .1f;
}

void FCascadeEyeDetector::DrawPreFilteredFaces(const cv::Mat& Frame, const std::vector<cv::Rect>& Faces) const
{
	// Show faces on frame as a rectangle.
	for (auto Face : Faces)
	{
		cv::rectangle(Frame,
			{Face.x, Face.y}, {Face.x + Face.width, Face.y + Face.height},
			{255, 255, 0}, 2);
	}
}

void FCascadeEyeDetector::DrawFace(const cv::Mat& Frame, const cv::Rect& Face) const
{
	cv::rectangle(Frame,
			{Face.x, Face.y}, {Face.x + Face.width, Face.y + Face.height},
			{175, 255, 0}, 2);
}

void FCascadeEyeDetector::DrawEyeArea(const cv::Mat& Frame, const cv::Rect& EyeArea) const
{
	cv::rectangle(Frame,
			{EyeArea.x, EyeArea.y}, {EyeArea.x + EyeArea.width, EyeArea.y + EyeArea.height},
			{125, 255, 0}, 2);
}

void FCascadeEyeDetector::DrawPreFilteredEyes(const cv::Mat& Frame, const cv::Rect& Face, const std::vector<cv::Rect>& Eyes) const
{
	// Show eyes on frame as a rectangle.
	for (auto Eye : Eyes)
	{
		cv::rectangle(Frame(Face),
			{Eye.x, Eye.y}, {Eye.x + Eye.width, Eye.y + Eye.height},
			{0, 255, 255}, 1);
	}
}

void FCascadeEyeDetector::DrawEye(const cv::Mat& Frame, const cv::Rect& EyeArea, const cv::Rect& Eye) const
{
	const auto EyeCentre = cv::Point(Eye.x + Eye.width / 2, Eye.y + Eye.height / 2);
	const int32 Radius = FMath::RoundToInt((Eye.width + Eye.height) * .15f);
	cv::circle(Frame(EyeArea), EyeCentre, Radius, {150, 255, 255}, 1);
}

EEyeStatus FCascadeEyeDetector::GetEyeStatusFromFrame(const cv::Mat& Frame) const
{
	// Get faces.
	const cv::Rect Face = GetFace(Frame);
	if (Face.empty())
		return EEyeStatus::Error;
	
	// Get eyes.
	cv::Rect LeftEye, RightEye;
	GetEyes(Frame, Face, OUT LeftEye, OUT RightEye);
	
	// Treat no eyes found as a blink.
	if (LeftEye.empty() && RightEye.empty())
		return EEyeStatus::Blink;
	if (LeftEye.empty())
		return EEyeStatus::WinkLeft;
	if (RightEye.empty())
		return EEyeStatus::WinkRight;

	return EEyeStatus::BothOpen;
}

void FCascadeEyeDetector::UpdateEyeState(EEyeStatus FrameEyeStatus, const double& DeltaTime)
{
	// Keeps track of frame changes so we can figure out which events were likely errors.
	switch (FrameEyeStatus)
	{
		case EEyeStatus::BothOpen:
			TimeLeftEyeClosed = FMath::Clamp(TimeLeftEyeClosed - DeltaTime * OpenEyeTimeMultiplier, 0, SampleRate);
			TimeRightEyeClosed = FMath::Clamp(TimeRightEyeClosed - DeltaTime * OpenEyeTimeMultiplier, 0, SampleRate);
			break;
		case EEyeStatus::WinkLeft:
			TimeLeftEyeClosed = FMath::Clamp(TimeLeftEyeClosed + DeltaTime * WinkEyeTimeMultiplier, 0, SampleRate);
		    TimeRightEyeClosed = FMath::Clamp(TimeRightEyeClosed - DeltaTime * OpenEyeTimeMultiplier, 0, SampleRate);
			break;
		case EEyeStatus::WinkRight:
			TimeLeftEyeClosed = FMath::Clamp(TimeLeftEyeClosed - DeltaTime * OpenEyeTimeMultiplier, 0, SampleRate);
			TimeRightEyeClosed = FMath::Clamp(TimeRightEyeClosed + DeltaTime * WinkEyeTimeMultiplier, 0, SampleRate);
			break;
		case EEyeStatus::Blink:
			TimeLeftEyeClosed = FMath::Clamp(TimeLeftEyeClosed + DeltaTime * BlinkEyeTimeMultiplier, 0, SampleRate);
			TimeRightEyeClosed = FMath::Clamp(TimeRightEyeClosed + DeltaTime * BlinkEyeTimeMultiplier, 0, SampleRate);
			break;
		case EEyeStatus::Error:
			TimeLeftEyeClosed = FMath::Clamp(TimeLeftEyeClosed + DeltaTime * ErrorTimeMultiplier, 0, SampleRate);
		    TimeRightEyeClosed = FMath::Clamp(TimeRightEyeClosed + DeltaTime * ErrorTimeMultiplier, 0, SampleRate);
			break;
	}
}

EEyeStatus FCascadeEyeDetector::GetEyeStatusWithError(EEyeStatus FrameEyeStatus) const
{
	const bool bLeftEyeOpen = TimeLeftEyeClosed / SampleRate < ClosedEyeThreshold;
	const bool bRightEyeOpen = TimeRightEyeClosed / SampleRate < ClosedEyeThreshold;
	
	// Both eyes have been recently closed.
	if (!bLeftEyeOpen && !bRightEyeOpen)
		return EEyeStatus::Blink;
	// Both eyes have been open for long or not closed long enough.
	if (bLeftEyeOpen && bRightEyeOpen)
		return EEyeStatus::BothOpen;
	// Right eye has been closed for long but left not.
	if (bLeftEyeOpen && !bRightEyeOpen)
		return EEyeStatus::WinkRight;
	// Left eye has been closed for long but right not. 
	if (!bLeftEyeOpen && bRightEyeOpen)
		return EEyeStatus::WinkLeft;
	
	return FrameEyeStatus;
}
