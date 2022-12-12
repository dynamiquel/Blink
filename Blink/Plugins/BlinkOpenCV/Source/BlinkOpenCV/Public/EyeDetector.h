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
	
	const TSharedPtr<double> GetLastBlinkTime() const { return LastBlinkTime; }
	const TSharedPtr<double> GetLastLeftWinkTime() const { return LastLeftWinkTime; }
	const TSharedPtr<double> GetLastRightWinkTime() const { return LastRightWinkTime; }

protected:
	void SetLastBlinkTime(double NewBlinkTime) const { *LastBlinkTime = NewBlinkTime; }
	void SetLastLeftWinkTime(double NewLeftWinkTime) const { *LastLeftWinkTime = NewLeftWinkTime; }
	void SetLastRightWinkTime(double NewRightWinkTime) const { *LastRightWinkTime = NewRightWinkTime; }

private:
	// Thread-safe pointers to last eye closed times.
	TSharedPtr<double> LastBlinkTime;
	TSharedPtr<double> LastLeftWinkTime;
	TSharedPtr<double> LastRightWinkTime;
};
