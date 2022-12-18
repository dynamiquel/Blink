// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#include "DnnEyeDetector.h"

FDnnEyeDetector::FDnnEyeDetector(FVideoReader* VideoReader) : FEyeDetector(VideoReader)
{
	ThreadName = TEXT("DnnEyeDetectorThread");

	CreateThread();
}

bool FDnnEyeDetector::Init()
{
	FString DnnDirectory = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("BlinkOpenCV"), TEXT("Content"), TEXT("DNN"));
    
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	FString Result;

	// Load the Face ONNX model.
	FString FilePath = FPaths::Combine(DnnDirectory, TEXT("face_detection_yunet_2022mar.onnx"));
	checkf(FileManager.FileExists(*FilePath), TEXT("The OpenCV Face model does not exist"));

	FaceDetector = cv::FaceDetectorYN::create(TCHAR_TO_UTF8(*FilePath), "", FaceSize,
		FaceConfidenceThreshold, NmsThreshold, TopKBoxes, cv::dnn::DNN_BACKEND_CUDA, cv::dnn::DNN_TARGET_CUDA);

	checkf(FaceDetector && FaceDetector.get(), TEXT("The OpenCV Face model failed to load"));
	
	return FEyeDetector::Init();
}

void FDnnEyeDetector::Exit()
{
	FEyeDetector::Exit();
	FaceDetector.reset();
}

uint32 FDnnEyeDetector::ProcessNextFrame(cv::Mat& Frame, const double& DeltaTime)
{
	// The DNN model is really really slow. Resizing the frame to a smaller size, dramatically decreases processing times
	// while retaining accuracy.
	cv::cuda::GpuMat CMat;
	CMat.upload(Frame);
	cv::cuda::resize(CMat, CMat, {320, 180});
	CMat.download(Frame);
	
	FaceDetector->setInputSize({Frame.cols, Frame.rows});
	
	cv::Mat FoundFaces;
	int32 BestFaceIndex;
	GetFace(Frame, OUT FoundFaces, OUT BestFaceIndex);
	
	if (BestFaceIndex >= 0)
	{
		DrawFace(Frame, FoundFaces, BestFaceIndex);
	}
	
	return 0;
}

void FDnnEyeDetector::GetFace(const cv::Mat& Frame, cv::Mat& FoundFaces, int32& BestFaceIndex)
{
	BestFaceIndex = -1;
	FaceDetector->detect(Frame, OUT FoundFaces);

	// False if no faces were found.
	// Each row in this Mat is a different face, with the column specifying the location of face landmarks,
	// such as eyes, mouth and nose.
	if (FoundFaces.rows < 1)
		return;
	
	BestFaceIndex = CalculateBestFace(FoundFaces);
}

int32 FDnnEyeDetector::CalculateBestFace(cv::Mat& FoundFaces)
{
	// Basic algorithm that determines best face by choosing the biggest face by area.
	int32 BiggestFaceArea = 0;
	int32 BiggestFaceIndex = 0;
	for (int32 i = 0; i < FoundFaces.rows; i++)
	{
		cv::Rect FaceRect = GetFaceRect(FoundFaces, i);
		int32 FaceArea = FaceRect.area();
		if (FaceArea > BiggestFaceArea)
		{
			BiggestFaceArea = FaceArea;
			BiggestFaceIndex = i;
		}
	}

	return BiggestFaceIndex;
}

void FDnnEyeDetector::DrawFace(const cv::Mat& Frame, const cv::Mat& Faces, int32 FaceIndex) const
{	
	// Face box.
	cv::rectangle(Frame,
			GetFaceRect(Faces, FaceIndex),
			{175, 255, 0}, 2);

	// Right eye (from person pov)
	cv::circle(Frame, GetRightEye(Faces, FaceIndex), 10, {150, 255, 255}, 1);

	// Left eye (from person pov)
	cv::circle(Frame, GetLeftEye(Faces, FaceIndex), 10, {150, 255, 255}, 1);
}

cv::Rect FDnnEyeDetector::GetFaceRect(const cv::Mat& Faces, int32 FaceIndex)
{
	// Landmark indices:
	// 0 = top-left of face
	// 1 = bottom right of face.
	// 2 = width of face.
	// 3 = height of face.
	// 4 = right eye x
	// 5 = right eye y
	// 6 = left eye x
	// 7 = right eye y
	// 8 = nose x
	// 9 = nose y
	// 10 = right mouth corner x
	// 11 = right mouth corner y
	// 12 = left mouth corner x
	// 13 = left mouth corner y
	
	return cv::Rect((int)Faces.at<float>(FaceIndex, 0), (int)Faces.at<float>(FaceIndex, 1), (int)Faces.at<float>(FaceIndex, 2), (int)Faces.at<float>(FaceIndex, 3));
}

cv::Size FDnnEyeDetector::GetRightEye(const cv::Mat& Faces, int32 FaceIndex)
{
	return cv::Point((int)Faces.at<float>(FaceIndex, 4), (int)Faces.at<float>(FaceIndex, 5));;
}

cv::Size FDnnEyeDetector::GetLeftEye(const cv::Mat& Faces, int32 FaceIndex)
{
	return cv::Point((int)Faces.at<float>(FaceIndex, 6), (int)Faces.at<float>(FaceIndex, 7));
}
