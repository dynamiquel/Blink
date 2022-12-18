// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#pragma once
#include "FeatureDetector.h"

UENUM()
enum class EEyeStatus : uint8
{
	BothOpen,
	WinkLeft,
	WinkRight,
	Blink,
	Error
};

class FEyeDetector : public FFeatureDetector
{
public:
	FEyeDetector(FVideoReader* InVideoReader);
	
	const TWeakPtr<double> GetLastBlinkTime() const { return LastBlinkTime; }
	const TWeakPtr<double> GetLastLeftWinkTime() const { return LastLeftWinkTime; }
	const TWeakPtr<double> GetLastRightWinkTime() const { return LastRightWinkTime; }

protected:
	void SetLastBlinkTime(double NewBlinkTime) { *LastBlinkTime = NewBlinkTime; }
	void SetLastLeftWinkTime(double NewLeftWinkTime) { *LastLeftWinkTime = NewLeftWinkTime; }
	void SetLastRightWinkTime(double NewRightWinkTime) { *LastRightWinkTime = NewRightWinkTime; }

protected:
	virtual EEyeStatus GetEyeStatusFromFrame(const cv::Mat& Frame) const = 0;

private:
	// Thread-safe pointers to last eye closed times.
	TSharedPtr<double> LastBlinkTime;
	TSharedPtr<double> LastLeftWinkTime;
	TSharedPtr<double> LastRightWinkTime;
};
