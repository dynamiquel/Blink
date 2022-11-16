// Copyright Epic Games, Inc. All Rights Reserved.
using System;
using System.IO;
using UnrealBuildTool;

public class OpenCV : ModuleRules
{
	public OpenCV(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		string PlatformDir = Target.Platform.ToString();
		string IncPath = Path.Combine(ModuleDirectory, "include");
        string BinaryPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Binaries/ThirdParty", PlatformDir));
		
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Link CUDA.
			string CudaPath = "D:/Program Files/CUDA/v11.8";
			string CudaIncludePath = Path.Combine(CudaPath, "include");
			string CudaLibPath = Path.Combine(CudaPath, "lib/x64");
			PublicIncludePaths.Add(CudaIncludePath);
			PublicAdditionalLibraries.Add(Path.Combine(CudaLibPath, "cudart_static.lib"));

			PublicSystemIncludePaths.Add(IncPath);

			string LibPath = Path.Combine(ModuleDirectory, "lib", PlatformDir);
			string LibName = "opencv_world455";
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, LibName + ".lib"));
			
			string DLLName = LibName + ".dll";
			string FFMPEGDDLName = "opencv_videoio_ffmpeg455_64.dll";
			string MSMFDDLName = "opencv_videoio_msmf455_64.dll";
			string HashDDLName = "opencv_img_hash455.dll";

			PublicDelayLoadDLLs.AddRange(new []
			{
				DLLName,
				FFMPEGDDLName,
				MSMFDDLName,
				HashDDLName
			});
			
			RuntimeDependencies.Add(Path.Combine(BinaryPath, DLLName));
			RuntimeDependencies.Add(Path.Combine(BinaryPath, FFMPEGDDLName));
			RuntimeDependencies.Add(Path.Combine(BinaryPath, MSMFDDLName));
			RuntimeDependencies.Add(Path.Combine(BinaryPath, HashDDLName));
			
			PublicDefinitions.AddRange(new []
			{
				"WITH_OPENCV=1",
				"OPENCV_PLATFORM_PATH=Binaries/ThirdParty/" + PlatformDir,
				"OPENCV_DLL_NAME=" + DLLName,
				"OPENCV_FFMPEG_DLL_NAME=" + FFMPEGDDLName,
				"OPENCV_MSMF_DLL_NAME=" + MSMFDDLName,
				"OPENCV_HASH_DLL_NAME=" + HashDDLName,
			});
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicSystemIncludePaths.Add(IncPath);
			
			string LibName = "libopencv_world.so";
			PublicAdditionalLibraries.Add(Path.Combine(BinaryPath, LibName));
			PublicRuntimeLibraryPaths.Add(BinaryPath);
			RuntimeDependencies.Add(Path.Combine(BinaryPath, LibName));
			RuntimeDependencies.Add(Path.Combine(BinaryPath, "libopencv_world.so.405"));
			PublicDefinitions.Add("WITH_OPENCV=1");
		}
		else // unsupported platform
		{
            PublicDefinitions.Add("WITH_OPENCV=0");
		}
	}
}
