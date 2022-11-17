// Copyright Epic Games, Inc. All Rights Reserved.

#include "IOpenCVHelperModule.h"
#include "Containers/Set.h"
#include "Misc/ScopeLock.h"
#include "Modules/ModuleManager.h" // for IMPLEMENT_MODULE()
#include "Interfaces/IPluginManager.h"
#include "HAL/PlatformProcess.h"

#if WITH_OPENCV
#include "OpenCVHelper.h"

#include "PreOpenCVHeaders.h"
#include "opencv2/unreal.hpp"
#include "PostOpenCVHeaders.h"

#endif

namespace OpenCVHelperModule
{
#if WITH_OPENCV

	static TSet<void*> UnrealAllocationsSet;
	static FCriticalSection UnrealAllocationsSetMutex;

	static void* UnrealMalloc(size_t Count, uint32_t Alignment)
	{
		void* Address = FMemory::Malloc(static_cast<SIZE_T>(Count), static_cast<uint32>(Alignment));

		{
			FScopeLock Lock(&UnrealAllocationsSetMutex);
			UnrealAllocationsSet.Add(Address);
		}

		return Address;
	}

	static void UnrealFree(void* Original)
	{
		// Only free allocations made by Unreal. Any allocations made before new/delete was overridden will have to leak.
		{
			FScopeLock Lock(&UnrealAllocationsSetMutex);

			if (!UnrealAllocationsSet.Contains(Original))
			{
				return;
			}

			UnrealAllocationsSet.Remove(Original);
		}

		FMemory::Free(Original);
	}

#endif //WITH_OPENCV
}

class FOpenCVHelperModule : public IOpenCVHelperModule
{
public:
	FOpenCVHelperModule();

public:
	//~ IModuleInterface interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void* OpenCvDllHandle;
	void* OpenCvFfmpegDllHandle;
	void* OpenCvMsmfDllHandle;
	void* OpenCvHashDllHandle;
};

FOpenCVHelperModule::FOpenCVHelperModule()
	: OpenCvDllHandle(nullptr), OpenCvFfmpegDllHandle(nullptr), OpenCvMsmfDllHandle(nullptr),
OpenCvHashDllHandle(nullptr)
{}

void FOpenCVHelperModule::StartupModule()
{
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("OpenCV"))->GetBaseDir();

#if WITH_OPENCV

#if defined(OPENCV_DLL_NAME)
	const FString OpenCvBinPath = PluginDir / TEXT(PREPROCESSOR_TO_STRING(OPENCV_PLATFORM_PATH));

	FPlatformProcess::PushDllDirectory(*OpenCvBinPath);	

#if defined(OPENCV_FFMPEG_DLL_NAME)
	const FString FfmpegDLLPath = OpenCvBinPath / TEXT(PREPROCESSOR_TO_STRING(OPENCV_FFMPEG_DLL_NAME));
	OpenCvFfmpegDllHandle = FPlatformProcess::GetDllHandle(*FfmpegDLLPath);
#endif
#if defined(OPENCV_MSMF_DLL_NAME)
	const FString MsmfDLLPath = OpenCvBinPath / TEXT(PREPROCESSOR_TO_STRING(OPENCV_MSMF_DLL_NAME));
	OpenCvMsmfDllHandle = FPlatformProcess::GetDllHandle(*MsmfDLLPath);
#endif
#if defined(OPENCV_HASH_DLL_NAME)
	const FString HashDLLPath = OpenCvBinPath / TEXT(PREPROCESSOR_TO_STRING(OPENCV_HASH_DLL_NAME));
	OpenCvHashDllHandle = FPlatformProcess::GetDllHandle(*HashDLLPath);
#endif

	const FString DLLPath = OpenCvBinPath / TEXT(PREPROCESSOR_TO_STRING(OPENCV_DLL_NAME));
	OpenCvDllHandle = FPlatformProcess::GetDllHandle(*DLLPath);

	
	FPlatformProcess::PopDllDirectory(*OpenCvBinPath);
	
#endif

	// We need to tell OpenCV to use Unreal's memory allocator to avoid crashes.
	// These may happen when Unreal passes a container to OpenCV, then OpenCV allocates memory for that container
	// and then Unreal tries to release the memory in it.
	cv::unreal::SetMallocAndFree(&OpenCVHelperModule::UnrealMalloc, &OpenCVHelperModule::UnrealFree);

#endif
}

void FOpenCVHelperModule::ShutdownModule()
{
#if WITH_OPENCV
	if (OpenCvDllHandle)
	{
		FPlatformProcess::FreeDllHandle(OpenCvDllHandle);
		OpenCvDllHandle = nullptr;
	}
	if (OpenCvFfmpegDllHandle)
	{
		FPlatformProcess::FreeDllHandle(OpenCvFfmpegDllHandle);
		OpenCvFfmpegDllHandle = nullptr;
	}
	if (OpenCvMsmfDllHandle)
	{
		FPlatformProcess::FreeDllHandle(OpenCvMsmfDllHandle);
		OpenCvMsmfDllHandle = nullptr;
	}
	if (OpenCvHashDllHandle)
	{
		FPlatformProcess::FreeDllHandle(OpenCvHashDllHandle);
		OpenCvHashDllHandle = nullptr;
	}

	// Note: Seems safer to not put back the original new/delete in OpenCV and keep Unreal's versions even after this module unloads.

#endif
}

IMPLEMENT_MODULE(FOpenCVHelperModule, OpenCVHelper);
