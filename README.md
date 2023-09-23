## Overview
A Physically Based Rendering renderer using Ray Tracing on OpenCL.  

![acdb](https://user-images.githubusercontent.com/43297470/198681895-33ac1737-1166-481a-a2bd-19b8b6834052.png)

## Build
Use [vcpkg](https://github.com/microsoft/vcpkg) download libraries we need.  

```sh
> vcpkg install glfw3:x64-windows
> vcpkg install glad:x64-windows
> vcpkg install opencl:x64-windows
> vcpkg install glm:x64-windows
```
Download [GLFW3](https://www.glfw.org/download.html) x64 binaries for your Visual C++ version and copy all file in `./lib-vcxxxx` to `your_vcpkg_path\installed\x64-windows\lib`.  
Alter CMAKE_PREFIX_PATH in cmake list to your vcpkg path, default path is `C:/source/vcpkg/installed/x64-windows`.\  

## Note
This project built in **AMD OpenCL2.0** environment at first.  
Now porting to **Nvidia OpenCL1.2**, still have some work to do.  

## Related Efforts
The OpenCL initialize and basic structure functions are based on  
[OpenCL-path-tracing-tutorial-2-Part-2-Path-tracing-spheres](https://github.com/straaljager/OpenCL-path-tracing-tutorial-2-Part-2-Path-tracing-spheres).
