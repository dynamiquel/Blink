#pragma once
#include "EyeDetector.h"

/**
 * @brief My second implementation of an eye detector. It uses DNN instead of cascades, by using OpenCV's YuNet, which
 * can be found here: https://github.com/opencv/opencv_zoo/tree/master/models/face_detection_yunet.
 *
 * This DNN is a lot more accurate (almost perfect) than the cascade implementation at the cost of more processing time.
 * The DNN model itself features landmarks, which can be used to get eye position. Sadly, this landmark feature is
 * not very precise (but good enough), does not provide width/height and appears even if the eyes are closed.
 * Therefore, this model cannot be used to determine whether eyes are closed or open.
 */
class FDnnEyeDetector : public FEyeDetector
{
public:
	FDnnEyeDetector(FVideoReader* VideoReader);

	virtual bool Init() override;
	virtual void Exit() override;

protected:
	virtual uint32 ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime) override;

	void GetFace(const cv::Mat& Frame, cv::Mat& FoundFaces, OUT int32& BestFaceIndex);
	int32 CalculateBestFace(cv::Mat& FoundFaces);
	void DrawFace(const cv::Mat& Frame, const cv::Mat& Faces, int32 FaceIndex) const;

	static cv::Rect GetFaceRect(const cv::Mat& Faces, int32 FaceIndex);
	static cv::Size GetRightEye(const cv::Mat& Faces, int32 FaceIndex);
	static cv::Size GetLeftEye(const cv::Mat& Faces, int32 FaceIndex);

protected:
	float FaceConfidenceThreshold = .9f;
	cv::Point FaceSize = cv::Point(320, 320);
	float NmsThreshold = .3f;
	int32 TopKBoxes = 2500;
	
	cv::Ptr<cv::FaceDetectorYN> FaceDetector;
};
