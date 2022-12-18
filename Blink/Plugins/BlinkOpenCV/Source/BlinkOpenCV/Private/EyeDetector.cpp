// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#include "EyeDetector.h"

FEyeDetector::FEyeDetector(FVideoReader* InVideoReader): FFeatureDetector(InVideoReader)
{
	ThreadName = TEXT("EyeDetectorThread");
	
	LastBlinkTime = MakeShared<double>();
	LastLeftWinkTime = MakeShared<double>();
	LastRightWinkTime = MakeShared<double>();
}
