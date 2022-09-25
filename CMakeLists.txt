cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0...3.18.4)

set(NAME OpenCLPathTracer)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

list(APPEND CMAKE_PREFIX_PATH "C:/src/vcpkg/vcpkg/installed/x64-windows")
file(GLOB src
     "*.h"
     "*.cpp"
     "./kernel/*.cl"
     "./shader/*.*"
)   

project(${NAME})

add_executable(${NAME} ${src})

target_link_libraries(${NAME} PRIVATE opengl32)
target_link_libraries(${NAME} PRIVATE OpenCL)
target_link_libraries(${NAME} PRIVATE glad)
target_link_libraries(${NAME} PRIVATE glfw3dll)

