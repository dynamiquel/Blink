#pragma once
#include "EyeDetector.h"

class BLINKOPENCV_API FCascadeEyeDetector : public FEyeDetector
{
public:
	FCascadeEyeDetector(FVideoReader* InVideoReader);
	virtual void Exit() override;
	
protected:
	virtual uint32 ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime) override;

	virtual cv::Rect GetFace(const cv::Mat& Frame) const;
	virtual void FilterFaces(const cv::Mat& Frame, std::vector<cv::Rect>& Faces) const;
	virtual void GetEyes(const cv::Mat& Frame, const cv::Rect& Face, cv::Rect& LeftEye, cv::Rect& RightEye) const;
	virtual void FilterEyes(std::vector<cv::Rect>& LeftEyes, std::vector<cv::Rect>& RightEyes, const cv::Rect& Face) const;

	static void TrimFaceToEyes(const cv::Rect& Face, cv::Rect& LeftEyeArea, cv::Rect& RightEyeArea);
	static bool IsEyeTooLarge(const cv::Rect& Eye, const cv::Rect& Face);

	void DrawPreFilteredFaces(const cv::Mat& Frame, const std::vector<cv::Rect>& Faces) const;
	void DrawFace(const cv::Mat& Frame, const cv::Rect& Face) const;
	void DrawEyeArea(const cv::Mat& Frame, const cv::Rect& EyeArea) const;
	void DrawPreFilteredEyes(const cv::Mat& Frame, const cv::Rect& EyeArea, const std::vector<cv::Rect>& Eyes) const;
	void DrawEye(const cv::Mat& Frame, const cv::Rect& EyeArea, const cv::Rect& Eye) const;

	EEyeStatus GetEyeStatusFromFrame(const cv::Mat& Frame) const;
	void UpdateEyeState(EEyeStatus FrameEyeStatus, const double& DeltaTime);
	EEyeStatus GetEyeStatusWithError(EEyeStatus FrameEyeStatus) const;
	
protected:
	int MinFaceSize = 200;
	int MinEyeSize = 40;
	float SampleRate = .4f;
	float ClosedEyeThreshold = .33f;
	float OpenEyeTimeMultiplier = 1.5f; // Should be strong enough to remove outliers and quickly reset.
	float WinkEyeTimeMultiplier = 1.f; // Should be weak enough to prevent outliers but strong enough to allow blinks mistaken as left and right winks.
	float BlinkEyeTimeMultiplier = 3.5f; // Should be strong enough for two blinks to be registered.
	float ErrorTimeMultiplier = 2.5f;
	
	TSharedPtr<cv::CascadeClassifier> FaceClassifier;
	TSharedPtr<cv::CascadeClassifier> EyeClassifier;

	// State vars to take error into consideration.
	float TimeLeftEyeClosed = 0;
	float TimeRightEyeClosed = 0;
};
