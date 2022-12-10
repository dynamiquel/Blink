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
	std::vector<cv::Rect> GetEyes(const cv::Mat& Frame, const cv::Rect& Face) const;

	void DrawFaces(const cv::Mat& Frame, const std::vector<cv::Rect>& Faces) const;
	void DrawEyes(const cv::Mat& Frame, const std::vector<cv::Rect>& Eyes) const;

	static FVector2D GetEyeAspectRatios(const std::vector<cv::Rect>& Eyes);
	int32 GetEyesUnderAspectRatioThreshold(const FVector2D& EyesAspectRatios) const;

	EEyeStatus GetEyeStatusFromFrame(const cv::Mat& Frame);
	
protected:
	float EyeClosedAspectRatioThreshold = 1.f;
	int MinEyeSize = 20; 
	
	cv::Ptr<cv::CascadeClassifier> FaceClassifier;
	cv::Ptr<cv::CascadeClassifier> EyeClassifier;
};
