#include "EyeDetector.h"

FEyeDetector::FEyeDetector(FVideoReader* InVideoReader): FFeatureDetector(InVideoReader)
{
	ThreadName = TEXT("EyeDetectorThread");
	
	LastBlinkTime = MakeShared<double>();
	LastLeftWinkTime = MakeShared<double>();
	LastRightWinkTime = MakeShared<double>();
}
