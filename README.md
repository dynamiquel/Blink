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

Since this Plugin uses base classes, such as `FVideoReader` and `FFeatureDetector`, it should be relativley easy to expand this plugin to suit your own Unreal game needs.

### 3. Game
**Dir: /Source and /Content**

All the other game stuff, such as input, movement, player controller, assets, etc.
 
## Dependencies
The project uses Nvidia technology to get the most performance and so a Nvidia GPU is required to play this game.
1. OpenCV 4.5.5 with various additional modules (pre-installed with the forked OpenCV plugin)
2. Nvidia CUDA Runtime (pre-installed with Nvidia drivers)
3. GStreamer (currently an external dependency using the default packaged build settings)

I have not tested whether this project falls back to CPU processing when Nvidia CUDA cannot be used.
