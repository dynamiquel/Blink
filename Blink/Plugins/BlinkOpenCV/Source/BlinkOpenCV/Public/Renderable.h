#pragma once

class BLINKOPENCV_API FRenderable
{
public:
	virtual void Render() = 0;
	virtual void StopRendering() = 0;
	virtual ~FRenderable() = default;
};
