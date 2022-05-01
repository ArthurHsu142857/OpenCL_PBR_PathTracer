cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0...3.18.4)

set(NAME OpenCLPathTracer)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
#list(APPEND CMAKE_PREFIX_PATH "C:/Users/arthu/Desktop/OpenCLHeader/OpenCL-CLHPP/build;C:/Users/arthu/Desktop/OpenCLHeader/OpenCL-Headers/install;C:/Users/arthu/Desktop/OpenCLHeader/OpenCL-ICD-Loader/install")
list(APPEND CMAKE_PREFIX_PATH "C:/src/vcpkg/vcpkg/installed/x86-windows;C:/src/vcpkg/vcpkg/installed/x64-windows")

project(${NAME})
add_executable(${NAME} main.cpp)

find_package(OpenCL REQUIRED)
find_package(glfw3 3.3.7 REQUIRED)
find_package(glad REQUIRED)

target_link_libraries(${NAME} PRIVATE OpenCL::OpenCL glfw glad::glad)

#find_package(OpenCLHeaders REQUIRED)
#find_package(OpenCLICDLoader REQUIRED)
#find_package(OpenCLHeadersCpp REQUIRED)

#target_link_libraries(${NAME} PRIVATE OpenCL::Headers OpenCL::OpenCL OpenCL::HeadersCpp)


#cmake_minimum_required(VERSION 3.10)

#set(NAME OpenCLPathTracer)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

#set_property(GLOBAL PROPERTY USE_FOLDERS ON) 

#set(SOURCE_FILE main.cpp algebra3.cpp bvh.cpp Material.cpp matLoader.cpp objLoader.cpp Triangle.cpp algebra3.h bvh.h Material.h matLoader.h objLoader.h Shader.h Triangle.h)

#add_executable(${NAME} ${SOURCE_FILE})
#target_link_directories(${NAME} )
#add_library()

#cmake_minimum_required(VERSION 3.7)

#set(NAME OpenCLPathTracer)
#set(SOURCE_FILE main.cpp algebra3.cpp bvh.cpp Material.cpp matLoader.cpp objLoader.cpp Triangle.cpp algebra3.h bvh.h Material.h matLoader.h objLoader.h Shader.h Triangle.h)

#project(${NAME})

#find_package(OpenCL REQUIRED)
#add_executable(${NAME} ${SOURCE_FILE})
#target_link_libraries(${NAME} OpenCL::OpenCL)
