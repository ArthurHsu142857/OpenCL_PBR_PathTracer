cmake_minimum_required(VERSION 3.0)

set(NAME OpenCLPathTracer)
set(CMAKE_PREFIX_PATH "C:/source/vcpkg/installed/x64-windows")

project(${NAME})

find_package(OpenCL 2.2 REQUIRED)
find_path(GLAD_INCLUDE_DIR glad/glad.h)
find_path(GLFW3_INCLUDE_DIR GLFW/glfw3.h)
find_library(GLAD_LIBRARY glad)
find_library(GLFW3_LIBRARY glfw3)
find_library(GLFW3DLL_LIBRARY glfw3dll)

set(CMAKE_CXX_STANDARD 17)

include_directories(${OpenCL_INCLUDE_DIR})
include_directories(${GLAD_INCLUDE_DIR})
include_directories(${GLFW3_INCLUDE_DIR})

link_directories(${OpenCL_LIBRARY})
link_directories(${GLAD_LIBRARY})
link_directories(${GLFW3_LIBRARY})
link_directories(${GLFW3DLL_LIBRARY})

file(GLOB src
     "*.h"
     "*.cpp"
     "./kernel/*.cl"
     "./shader/*.*"
)   

add_executable(${NAME} ${src})

target_link_libraries(${NAME} 
	PRIVATE
		${OpenCL_LIBRARY}
		opengl32
		${GLAD_LIBRARY}
		${GLFW3_LIBRARY}
		${GLFW3DLL_LIBRARY}
)

