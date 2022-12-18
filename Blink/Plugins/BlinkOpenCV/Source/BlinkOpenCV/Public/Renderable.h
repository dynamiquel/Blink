// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#pragma once

class BLINKOPENCV_API FRenderable
{
public:
	virtual void Render() = 0;
	virtual void StopRendering() = 0;
	virtual ~FRenderable() = default;
};
