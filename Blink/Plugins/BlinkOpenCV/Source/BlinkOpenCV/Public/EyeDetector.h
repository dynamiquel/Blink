#pragma once
#include "FeatureDetector.h"

UENUM()
enum class EEyeStatus : uint8
{
	BothOpen,
	Wink,
	Blink,
	Error
};

class BLINKOPENCV_API FEyeDetector : public FFeatureDetector
{
public:
	FEyeDetector(FVideoReader* InVideoReader);
	
protected:
	virtual uint32 ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime) override;

	std::vector<cv::Rect> GetFaces(const cv::Mat& Frame) const;
	void GetEyes(const cv::Mat& Frame, const cv::Rect& Face, cv::Rect& LeftEye, cv::Rect& RightEye) const;
	static void TrimFaceToEyes(const cv::Rect& Face, cv::Rect& LeftEyeArea, cv::Rect& RightEyeArea);
	static void FilterEyes(std::vector<cv::Rect>& LeftEyes, std::vector<cv::Rect>& RightEyes, const cv::Rect& Face);
	static bool IsEyeTooLarge(const cv::Rect& Eye, const cv::Rect& Face);

	void DrawFaces(const cv::Mat& Frame, const std::vector<cv::Rect>& Faces) const;
	void DrawEyeArea(const cv::Mat& Frame, const cv::Rect& EyeArea) const;
	void DrawPreFilteredEyes(const cv::Mat& Frame, const cv::Rect& EyeArea, const std::vector<cv::Rect>& Eyes) const;
	void DrawEye(const cv::Mat& Frame, const cv::Rect& EyeArea, const cv::Rect& Eye) const;
	
	static FVector2D GetEyeAspectRatios(const std::vector<cv::Rect>& Eyes);
	int32 GetEyesUnderAspectRatioThreshold(const FVector2D& EyesAspectRatios) const;

	EEyeStatus GetEyeStatusFromFrame(const cv::Mat& Frame);
	
protected:
	float EyeClosedAspectRatioThreshold = 1.f;
	int MinFaceSize = 200;
	int MinEyeSize = 40; 
	
	cv::Ptr<cv::CascadeClassifier> FaceClassifier;
	cv::Ptr<cv::CascadeClassifier> EyeClassifier;
};
