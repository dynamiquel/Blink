#include "DnnCascadeEyeDetector.h"

#include "BlinkOpenCV.h"
#include "opencv2/cudaimgproc.hpp"


FDnnCascadeEyeDetector::FDnnCascadeEyeDetector(FVideoReader* InVideoReader) : FEyeDetector(InVideoReader)
{
	ThreadName = TEXT("DnnCascadeEyeDetectorThread");

	CreateThread();
}

bool FDnnCascadeEyeDetector::Init()
{
	FString DnnDirectory = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("BlinkOpenCV"), TEXT("Content"), TEXT("DNN"));
	FString CascadeDirectory = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("BlinkOpenCV"), TEXT("Content"), TEXT("Cascades"));
	
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	FString Result;

	// Load the Face ONNX model.
	FString FilePath = FPaths::Combine(DnnDirectory, TEXT("face_detection_yunet_2022mar.onnx"));
	checkf(FileManager.FileExists(*FilePath), TEXT("The OpenCV Face model does not exist"));
	LoadedFaceDetector = MakeShared<cv::Ptr<cv::FaceDetectorYN>>(cv::FaceDetectorYN::create(TCHAR_TO_UTF8(*FilePath), "", {100, 100},
		FaceConfidenceThreshold, NmsThreshold, TopKBoxes, cv::dnn::DNN_BACKEND_CUDA, cv::dnn::DNN_TARGET_CUDA));
	checkf(LoadedFaceDetector.IsValid(), TEXT("The OpenCV Face model failed to load"));

	// Load the Right Eye Haar classifier.
	FilePath = FPaths::Combine(CascadeDirectory, TEXT("haarcascade_eye.xml"));
	checkf(FileManager.FileExists(*FilePath), TEXT("The OpenCV Right Eye cascade filter does not exist"));
	LoadedRightEyeClassifier = MakeShared<cv::CascadeClassifier>(TCHAR_TO_UTF8(*FilePath));

	// Load the Left Eye Haar classifier.
	FilePath = FPaths::Combine(CascadeDirectory, TEXT("haarcascade_eye.xml"));
	checkf(FileManager.FileExists(*FilePath), TEXT("The OpenCV Left Eye cascade filter does not exist"));
	LoadedLeftEyeClassifier = MakeShared<cv::CascadeClassifier>(TCHAR_TO_UTF8(*FilePath));

	
	return FEyeDetector::Init();
}

FDnnCascadeEyeDetector::~FDnnCascadeEyeDetector()
{
	LoadedFaceDetector.Reset();
	LoadedRightEyeClassifier.Reset();
	LoadedLeftEyeClassifier.Reset();
}

uint32 FDnnCascadeEyeDetector::ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime)
{
	cv::cuda::GpuMat CMat;
	CMat.upload(Frame);
	cv::cuda::resize(CMat, CMat, {1280, 720});
	CMat.download(Frame);
	
	const EEyeStatus FrameEyeStatus = GetEyeStatusFromFrame(Frame);

	UpdateEyeState(FrameEyeStatus, DeltaTime);
	
	// Do additional processing to determine the actual eye status by taking errors into account.
	const EEyeStatus ErroredEyeStatus = GetEyeStatusWithError(FrameEyeStatus);

	// Record the last eye(s) closed time so it can be used by external objects.
	const double CurrentTime = FPlatformTime::Seconds();
	switch (ErroredEyeStatus)
	{
	case EEyeStatus::WinkLeft:
		SetLastLeftWinkTime(CurrentTime);
		break;
	case EEyeStatus::WinkRight:
		SetLastRightWinkTime(CurrentTime);
		break;
	case EEyeStatus::Blink:
		SetLastBlinkTime(CurrentTime);
		break;
	}

	UE_LOG(LogBlinkOpenCV, Warning, TEXT("State: %s"), *UEnum::GetValueAsString(FrameEyeStatus));
	UE_LOG(LogBlinkOpenCV, Error, TEXT("State: %s"), *UEnum::GetValueAsString(ErroredEyeStatus));
	
	return 0;
}

cv::Rect FDnnCascadeEyeDetector::GetFace(const cv::Mat& Frame, cv::Mat& FoundFaces, int32& BestFaceIndex) const
{
	BestFaceIndex = -1;

	// Downscale frame since the face detector performs very poorly at high resolution, with no improvement to
	// accuracy.
	cv::Mat DownscaledFrame = Frame;
	cv::cuda::GpuMat CMat;
	CMat.upload(DownscaledFrame);
	cv::cuda::resize(CMat, CMat, {640, 360});
	CMat.download(DownscaledFrame);
	
	if (const auto FaceDetector = GetFaceDetector().Pin(); FaceDetector.IsValid())
	{
		FaceDetector->get()->setInputSize({DownscaledFrame.cols, DownscaledFrame.rows});
		FaceDetector->get()->detect(DownscaledFrame, OUT FoundFaces);
	}

	// False if no faces were found.
	// Each row in this Mat is a different face, with the column specifying the location of face landmarks,
	// such as eyes, mouth and nose.
	if (FoundFaces.rows < 1)
		return cv::Rect();
	
	BestFaceIndex = CalculateBestFace(FoundFaces);
	return GetFaceRect(FoundFaces, BestFaceIndex);
}

int32 FDnnCascadeEyeDetector::CalculateBestFace(const cv::Mat& FoundFaces) const
{
	// Basic algorithm that determines best face by choosing the biggest face by area.
	int32 BiggestFaceArea = 0;
	int32 BiggestFaceIndex = 0;
	for (int32 i = 0; i < FoundFaces.rows; i++)
	{
		cv::Rect FaceRect = GetFaceRect(FoundFaces, i);
		if (const int32 FaceArea = FaceRect.area(); FaceArea > BiggestFaceArea)
		{
			BiggestFaceArea = FaceArea;
			BiggestFaceIndex = i;
		}
	}

	return BiggestFaceIndex;
}

void FDnnCascadeEyeDetector::DrawFace(const cv::Mat& Frame, const cv::Mat& Faces, int32 FaceIndex,
	bool bIncludeApproxEyes) const
{
	// Face box.
	cv::rectangle(Frame,
			GetFaceRect(Faces, FaceIndex),
			{175, 255, 0}, 2);

	// Right eye (from person pov)
	cv::circle(Frame, GetRightEyeApproxLocation(Faces, FaceIndex), 3, {150, 255, 255}, 1);

	// Left eye (from person pov)
	cv::circle(Frame, GetLeftEyeApproxLocation(Faces, FaceIndex), 3, {150, 255, 255}, 1);
}

void FDnnCascadeEyeDetector::DrawEyeApproxArea(const cv::Mat& Frame, const cv::Rect& EyeApproxArea) const
{
	cv::rectangle(Frame, EyeApproxArea, {0, 255, 0}, 1);
}

void FDnnCascadeEyeDetector::DrawPrefilteredEye(const cv::Mat& Frame, const cv::Rect& EyeApproxArea,
	const cv::Rect& Eye) const
{
	cv::rectangle(Frame(EyeApproxArea), Eye, {50, 50, 50}, 1);
}

void FDnnCascadeEyeDetector::DrawEye(const cv::Mat& Frame, const cv::Rect& EyeApproxArea, const cv::Rect& Eye) const
{
	if (Eye.area() == 0)
		return;
	
	const int32 Radius = Eye.width / 2;
	const auto EyeCentre = cv::Point(Eye.x + Radius, Eye.y + Radius);
	cv::circle(Frame(EyeApproxArea), EyeCentre, Radius, {0, 0, 255}, 1);
}


EEyeStatus FDnnCascadeEyeDetector::GetEyeStatusFromFrame(const cv::Mat& Frame) const
{	
	cv::Mat FoundFaces;
	int32 BestFaceIndex;
	cv::Rect FaceRect = GetFace(Frame, OUT FoundFaces, OUT BestFaceIndex);
	
	if (BestFaceIndex >= 0)
	{
		DrawFace(Frame, FoundFaces, BestFaceIndex, true);

		// Get the approximate eye location using the eye landmarks from the face detection model.
		const cv::Point RightEyeApproxLocation = GetRightEyeApproxLocation(FoundFaces, BestFaceIndex);
		const cv::Point LeftEyeApproxLocation = GetLeftEyeApproxLocation(FoundFaces, BestFaceIndex);

		// Convert the approximate eye location from a point to a rectangle proportional to the face width.
		const cv::Rect RightEyeApproxArea = GetEyeApproxLocationArea(FaceRect, RightEyeApproxLocation);
		const cv::Rect LeftEyeApproxArea = GetEyeApproxLocationArea(FaceRect, LeftEyeApproxLocation);

		DrawEyeApproxArea(Frame, RightEyeApproxArea);
		DrawEyeApproxArea(Frame, LeftEyeApproxArea);

		// Find and retrieve the actual eyes from the approximate eye location.
		cv::Rect RightEye, LeftEye;
		GetEyes(Frame, FaceRect, RightEyeApproxArea, LeftEyeApproxArea, OUT RightEye, OUT LeftEye);

		DrawEye(Frame, RightEyeApproxArea, RightEye);
		DrawEye(Frame, LeftEyeApproxArea, LeftEye);
		
		if (RightEye.empty() && LeftEye.empty())
			return EEyeStatus::Blink;
		if (RightEye.empty())
			return EEyeStatus::WinkRight;
		if (LeftEye.empty())
			return EEyeStatus::WinkLeft;
		
		return EEyeStatus::BothOpen;
	}

	return EEyeStatus::Error;
}

void FDnnCascadeEyeDetector::UpdateEyeState(EEyeStatus FrameEyeStatus, const double& DeltaTime)
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

EEyeStatus FDnnCascadeEyeDetector::GetEyeStatusWithError(EEyeStatus FrameEyeStatus) const
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

cv::Rect FDnnCascadeEyeDetector::GetFaceRect(const cv::Mat& Faces, int32 FaceIndex) const
{
	// Multiply by 2 since face detection was done at half-scale.
	return cv::Rect((int)Faces.at<float>(FaceIndex, 0) * 2, (int)Faces.at<float>(FaceIndex, 1) * 2,
	                (int)Faces.at<float>(FaceIndex, 2) * 2, (int)Faces.at<float>(FaceIndex, 3) * 2);
}

cv::Point FDnnCascadeEyeDetector::GetRightEyeApproxLocation(const cv::Mat& Faces, int32 FaceIndex) const
{
	return cv::Point((int)Faces.at<float>(FaceIndex, 4) * 2, (int)Faces.at<float>(FaceIndex, 5) * 2);;
}

cv::Point FDnnCascadeEyeDetector::GetLeftEyeApproxLocation(const cv::Mat& Faces, int32 FaceIndex) const
{
	return cv::Point((int)Faces.at<float>(FaceIndex, 6) * 2, (int)Faces.at<float>(FaceIndex, 7) * 2);
}

cv::Rect FDnnCascadeEyeDetector::GetEyeApproxLocationArea(const cv::Rect& Face, cv::Point EyeApproxLocation) const
{
	const int32 EyeWidth = (float)Face.width * EyeToFaceProportionMax;
	return cv::Rect(FMath::Max(0, EyeApproxLocation.x - EyeWidth / 2), FMath::Max(0, EyeApproxLocation.y - EyeWidth / 2),
	                EyeWidth, EyeWidth);
}

cv::Size FDnnCascadeEyeDetector::GetMinEyeSize(const cv::Rect& EyeApproxLocationArea) const
{
	// Calculate min eye size based on proportion to face.

	return {
		int32((float)EyeApproxLocationArea.width * EyeToFaceProportionMin),
		int32((float)EyeApproxLocationArea.height * EyeToFaceProportionMin)
	};
}

std::vector<cv::Rect> FDnnCascadeEyeDetector::GetRightEyesByCascade(const cv::Mat& Frame,
                                                                    const cv::Rect& RightEyeApproxArea) const
{
	if (const auto RightEyeClassifier = GetRightEyeClassifier().Pin(); RightEyeClassifier.IsValid())
	{
		cv::Mat EyeRoi = Frame(RightEyeApproxArea);

		// Cascade appears to work better in greyscale.
		cv::cuda::GpuMat CMat;
		CMat.upload(EyeRoi);
		cv::cuda::cvtColor(CMat, CMat, cv::COLOR_BGR2GRAY);
		CMat.download(EyeRoi);
	
		// Search for eyes in the calculated Right Eye Area.
		std::vector<cv::Rect> RightEyes;
		RightEyeClassifier->detectMultiScale(
			EyeRoi,
			OUT RightEyes,
			1.3,
			2,
			cv::CASCADE_SCALE_IMAGE,
			GetMinEyeSize(RightEyeApproxArea),
			RightEyeApproxArea.size());

		return RightEyes;
	}

	return std::vector<cv::Rect>();
}

std::vector<cv::Rect> FDnnCascadeEyeDetector::GetLeftEyesByCascade(const cv::Mat& Frame,
	const cv::Rect& LeftEyeApproxArea) const
{
	if (const auto LeftEyeClassifier = GetLeftEyeClassifier().Pin(); LeftEyeClassifier.IsValid())
	{
		cv::Mat EyeRoi = Frame(LeftEyeApproxArea);
	
		// Cascade appears to work better in greyscale.
		cv::cuda::GpuMat CMat;
		CMat.upload(EyeRoi);
		cv::cuda::cvtColor(CMat, CMat, cv::COLOR_BGR2GRAY);
		CMat.download(EyeRoi);
	
		// Search for eyes in the calculated Left Eye Area.
		std::vector<cv::Rect> LeftEyes;
		LeftEyeClassifier->detectMultiScale(
			EyeRoi,
			OUT LeftEyes,
			1.3,
			2,
			cv::CASCADE_SCALE_IMAGE,
			GetMinEyeSize(LeftEyeApproxArea),
			LeftEyeApproxArea.size());

		return LeftEyes;
	}
	
	return std::vector<cv::Rect>();
}

bool FDnnCascadeEyeDetector::IsEyeTooLarge(const cv::Rect& EyeApproxArea, const cv::Rect& Eye) const
{
	return Eye.width > EyeApproxArea.width || Eye.height > EyeApproxArea.height;
}

bool FDnnCascadeEyeDetector::IsEyeTooSmall(const cv::Rect& EyeApproxArea, const cv::Rect& Eye) const
{
	cv::Size MinSize = GetMinEyeSize(EyeApproxArea);
	return Eye.width < MinSize.width || Eye.height < MinSize.height;
}

void FDnnCascadeEyeDetector::FilterEyes(std::vector<cv::Rect>& LeftEyes, std::vector<cv::Rect>& RightEyes,
                                        const cv::Rect& Face, const cv::Rect& RightEyeApproxArea,
                                        const cv::Rect& LeftEyeApproxArea) const
{
}

void FDnnCascadeEyeDetector::GetEyes(const cv::Mat& Frame, const cv::Rect& Face, const cv::Rect& RightEyeApproxArea,
                                     const cv::Rect& LeftEyeApproxArea, cv::Rect& RightEye, cv::Rect& LeftEye) const
{	
	auto RightEyes = GetRightEyesByCascade(Frame, RightEyeApproxArea);
	auto LeftEyes = GetLeftEyesByCascade(Frame, LeftEyeApproxArea);

	for (const auto& CurrentRightEye : RightEyes)
		DrawPrefilteredEye(Frame, RightEyeApproxArea, CurrentRightEye);

	for (const auto& CurrentLeftEye : LeftEyes)
		DrawPrefilteredEye(Frame, LeftEyeApproxArea, CurrentLeftEye);

	// Remove likely incorrect eyes.
	FilterEyes(IN OUT LeftEyes, IN OUT RightEyes, Face, RightEyeApproxArea, LeftEyeApproxArea);

	// Return the first right and left eye.
	if (RightEyes.size() > 0)
		RightEye = RightEyes[0];
	if (LeftEyes.size() > 0)
		LeftEye = LeftEyes[0];
}
