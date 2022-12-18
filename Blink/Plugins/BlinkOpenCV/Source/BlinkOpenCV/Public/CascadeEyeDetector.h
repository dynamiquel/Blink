﻿// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#pragma once
#include "EyeDetector.h"

/**
 * @brief My first implementation of an eye detector using Haar cascades.
 * It is highly inaccurate as it leads to many false positives and few false negatives.
 * A lot of tolerance features have been implemented to increase accuracy at the cost of latency but it remains
 * insufficient, especially for winks.
 * It is surprisingly very accurate with low light.
 */
class BLINKOPENCV_API FCascadeEyeDetector : public FEyeDetector
{
public:
	FCascadeEyeDetector(FVideoReader* InVideoReader);
	virtual ~FCascadeEyeDetector() override;
	
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

	virtual EEyeStatus GetEyeStatusFromFrame(const cv::Mat& Frame) const override;
	void UpdateEyeState(EEyeStatus FrameEyeStatus, const double& DeltaTime);
	EEyeStatus GetEyeStatusWithError(EEyeStatus FrameEyeStatus) const;

	TWeakPtr<cv::CascadeClassifier> GetFaceClassifier() const { return FaceClassifier; }
	TWeakPtr<cv::CascadeClassifier> GetEyeClassifier() const { return EyeClassifier; }
	TWeakPtr<cv::Ptr<cv::cuda::Filter>> GetBlurFilter() const { return BlurFilter; }
	TWeakPtr<cv::Ptr<cv::cuda::CannyEdgeDetector>> GetEdgeFilter() const { return EdgeFilter; }
	
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
	TSharedPtr<cv::Ptr<cv::cuda::Filter>> BlurFilter;
	TSharedPtr<cv::Ptr<cv::cuda::CannyEdgeDetector>> EdgeFilter;

	// State vars to take error into consideration.
	float TimeLeftEyeClosed = 0;
	float TimeRightEyeClosed = 0;
};
