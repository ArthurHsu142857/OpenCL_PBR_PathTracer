cmake_minimum_required(VERSION 3.15)

set(NAME OpenCLPathTracer)
project(${NAME})

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use the vcpkg toolchain file if available
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}\\installed\\x64-windows")
else()
	message("VCPKG_ROOT is not defined")
endif()

# Find required packages
find_package(OpenCL REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)

# Collect all source and header files
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h")

# Collect all kernels and shader files
file(GLOB OPENCL_FILES "kernel/*.cl")
file(GLOB SHADER_FILES "shader/*.vs" "shader/*.fs")

source_group("kernels" FILES ${OPENCL_FILES})
source_group("shaders" FILES ${SHADER_FILES})

# Add executable with source and header files
add_executable(${NAME} ${SOURCES} ${HEADERS} ${OPENCL_FILES} ${SHADER_FILES})

# Link libraries
target_link_libraries(${NAME}
    PRIVATE
        OpenCL::OpenCL
		opengl32
        glad::glad
        glfw
)

# Include directories
target_include_directories(${NAME}
    PRIVATE
		${OpenCL_INCLUDE_DIR}
        ${GLAD_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIR}
        include
)