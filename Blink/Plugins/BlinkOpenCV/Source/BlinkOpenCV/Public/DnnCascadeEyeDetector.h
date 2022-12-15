#pragma once
#include "DnnEyeDetector.h"

/**
 * @brief My third implementation of an eye detector, combining the DNN face detector with the Haar cascade
 * eye detector.
 */
class FDnnCascadeEyeDetector : public FEyeDetector
{
public:
	FDnnCascadeEyeDetector(FVideoReader* InVideoReader);
	
	virtual bool Init() override;
	virtual ~FDnnCascadeEyeDetector() override;

protected:
	virtual uint32 ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime) override;

	cv::Rect GetFace(const cv::Mat& Frame, cv::Mat& FoundFaces, OUT int32& BestFaceIndex) const;
	int32 CalculateBestFace(const cv::Mat& FoundFaces) const;
	
	cv::Rect GetFaceRect(const cv::Mat& Faces, int32 FaceIndex) const;
	cv::Point GetRightEyeApproxLocation(const cv::Mat& Faces, int32 FaceIndex) const;
	cv::Point GetLeftEyeApproxLocation(const cv::Mat& Faces, int32 FaceIndex) const;
	cv::Rect GetEyeApproxLocationArea(const cv::Rect& Face, cv::Point EyeApproxLocation) const;
	cv::Size GetMinEyeSize(const cv::Rect& EyeApproxLocationArea) const;
	std::vector<cv::Rect> GetRightEyesByCascade(const cv::Mat& Frame, const cv::Rect& RightEyeApproxArea) const;
	std::vector<cv::Rect> GetLeftEyesByCascade(const cv::Mat& Frame, const cv::Rect& LeftEyeApproxArea) const;

	bool IsEyeTooLarge(const cv::Rect& EyeApproxArea, const cv::Rect& Eye) const;
	bool IsEyeTooSmall(const cv::Rect& EyeApproxArea, const cv::Rect& Eye) const;
	
	void FilterEyes(std::vector<cv::Rect>& LeftEyes, std::vector<cv::Rect>& RightEyes, const cv::Rect& Face,
	                const cv::Rect& RightEyeApproxArea, const cv::Rect& LeftEyeApproxArea) const;
	void GetEyes(const cv::Mat& Frame, const cv::Rect& Face, const cv::Rect& RightEyeApproxArea,
	             const cv::Rect& LeftEyeApproxArea, cv::Rect& RightEye, cv::Rect& LeftEye) const;
	
	void DrawFace(const cv::Mat& Frame, const cv::Mat& Faces, int32 FaceIndex, bool bIncludeApproxEyes) const;
	void DrawEyeApproxArea(const cv::Mat& Frame, const cv::Rect& EyeApproxArea) const;
	void DrawPrefilteredEye(const cv::Mat& Frame, const cv::Rect& EyeApproxArea, const cv::Rect& Eye) const;
	void DrawEye(const cv::Mat& Frame, const cv::Rect& EyeApproxArea, const cv::Rect& Eye) const;

	TWeakPtr<cv::Ptr<cv::FaceDetectorYN>> GetFaceDetector() const { return LoadedFaceDetector; }
	TWeakPtr<cv::CascadeClassifier> GetRightEyeClassifier() const { return LoadedRightEyeClassifier; }
	TWeakPtr<cv::CascadeClassifier> GetLeftEyeClassifier() const { return LoadedLeftEyeClassifier; }

	virtual EEyeStatus GetEyeStatusFromFrame(const cv::Mat& Frame) const override;
	void UpdateEyeState(EEyeStatus FrameEyeStatus, const double& DeltaTime);
	EEyeStatus GetEyeStatusWithError(EEyeStatus FrameEyeStatus) const;

protected:
	float FaceConfidenceThreshold = .9f;
	float NmsThreshold = .3f;
	int32 TopKBoxes = 2500;
	float EyeToFaceProportionMin = .5f;
	float EyeToFaceProportionMax = .35f;
	
	float SampleRate = .4f;
	float ClosedEyeThreshold = .33f;
	float OpenEyeTimeMultiplier = 1.5f; // Should be strong enough to remove outliers and quickly reset.
	float WinkEyeTimeMultiplier = 1.f; // Should be weak enough to prevent outliers but strong enough to allow blinks mistaken as left and right winks.
	float BlinkEyeTimeMultiplier = 5.f; // Should be strong enough for two blinks to be registered.
	float ErrorTimeMultiplier = 2.5f;
	
	TSharedPtr<cv::Ptr<cv::FaceDetectorYN>> LoadedFaceDetector;
	TSharedPtr<cv::CascadeClassifier> LoadedRightEyeClassifier;
	TSharedPtr<cv::CascadeClassifier> LoadedLeftEyeClassifier;

	// State vars to take error into consideration.
	float TimeLeftEyeClosed = 0;
	float TimeRightEyeClosed = 0;
};
