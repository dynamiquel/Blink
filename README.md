# Blink

Unreal Engine 5 comes pre-installed with an OpenCV plugin, which contains a stripped-down version of OpenCV 4.5.5 that cannot be used to read camera or video input.
I had to fork this plugin so I could make the necessary adjustments to make OpenCV work the way I want. This required building OpenCV 4.5.5 from source to include the necessary modules.

For whatever reason, I could never get DirectShow or Microsoft Media Foundation to work on my setup with Unreal Engine 5, so I decided to build and use the OpenCV GStreamer module.

## Design
The project is split into three main parts:

### 1. OpenCV Plugin
**Dir: /Plugins/OpenCV**

This is my own forked version of the Unreal Engine 5 OpenCV plugin, as mentioned earlier. 
It contains the necessary logic to get the OpenCV libraries running in the engine.

### 2. BlinkOpenCV Plugin
**Dir: /Plugins/BlinkOpenCV**

This is my own plugin, which contains all the OpenCV-related logic, such as opening the camera, reading the camera, face detection, etc. 
It uses multithreading by using Unreal's FRunnables and hardware acceleration using Nvidia CUDA.

Since this Plugin uses base classes, such as `FVideoReader` and `FFeatureDetector`, it should be relatively easy to expand this plugin to suit your own Unreal game needs.

`FCascadeEyeDetector` is the class which was featured in the tech demo and offers the best eye detection. See `FDnnCascadeEyeDetector` for an alternative implementation using deep neural networks.

### 3. Game
**Dir: /Source and /Content**

All the other game stuff, such as input, movement, player controller, assets, etc.
 
## Dependencies
The project uses Nvidia technology to get the most performance and so a Nvidia GPU is required to play this game.
1. OpenCV 4.5.5 with various additional modules (pre-installed with the forked OpenCV plugin)
2. Nvidia CUDA Runtime (pre-installed with Nvidia drivers)
3. GStreamer (currently an external dependency using the complete Windows binary installer, found [here](https://gstreamer.freedesktop.org/data/pkg/windows/1.20.4/msvc/gstreamer-1.0-msvc-x86_64-1.20.4.msi))

I have not tested whether this project falls back to CPU processing when Nvidia CUDA cannot be used.

## Testing
All testing was done with the provided **positive_test.mp4**, **negative_test.mp4**, **positive_light_test.mp4** and **negative_light_test.mp4** test files.

### Blink detector
|Metric	|Expected result	|Actual result	|
|---	|---	|---	|
|Accuracy (dark / light)   	|90%  	|100% / 37.5%   	|
|Latency (from camera to game)   	|200ms   	|11ms   	|
|False positives (dark / light)  	|1 every 60 seconds   	|0 / 4 every 15 seconds   	|

### Wink detector
|Metric	|Expected result	|Actual result	|
|---	|---	|---	|
|Accuracy (dark / light)   	|90%  	|N/A   	|
|Latency (from camera to game)   	|200ms   	|11ms   	|
|False positives (dark / light)  	|1 every 60 seconds   	|0 / 4 every 15 seconds   	|

### Other game feature tests
|Metric	|Expected result	|Actual result	|
|---	|---	|---	|
|Frames-per-second   	|60  	|73 - performance cost of camera features (including two video feed windows) is 2 FPS   	|
|Enemies   	|Should move towards the player and attempt to kill   	|Enemies are stationary targets which get smaller overtime   	|
|Player character and gun  	|The player character and gun should be working completely and responsive.    	|The player character and gun is working completely and responsive.   	|