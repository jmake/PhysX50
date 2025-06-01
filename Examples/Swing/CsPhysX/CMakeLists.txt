cmake_minimum_required(VERSION 3.16)

cmake_policy(SET CMP0122 NEW) ## ensures C# bindings use .NET-standard naming??
project(spicytech CXX C CUDA)

## Setting ... 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUERIRED ON)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") 

include_directories(${PROJECT_SOURCE_DIR})

message("[WRAPPER] CMAKE_SOURCE_DIR = '${CMAKE_SOURCE_DIR}' ")
message("[WRAPPER] CMAKE_CURRENT_SOURCE_DIR = '${CMAKE_CURRENT_SOURCE_DIR}' ")

find_package(CUDAToolkit REQUIRED)
get_target_property(cuda_runtime_location CUDA::cudart LOCATION)
message("[WRAPPER] CUDA runtime location: '${cuda_runtime_location}' ")
message("[WRAPPER] CUDAToolkit_INCLUDE_DIRS = '${CUDAToolkit_INCLUDE_DIRS}' ")


## PhysX ... 
message("[WRAPPER] PHYSX_ROOT_DIR='${PHYSX_ROOT_DIR}' ")

set(PHYSX_LIB_DIR "${PHYSX_ROOT_DIR}/bin/win.x86_64.vc143.mt/release")
file(GLOB PHYSX_STATIC_LIBS "${PHYSX_LIB_DIR}/*_64.lib") # Get only static libraries

message(STATUS "Using static PhysX libraries:")
foreach(lib ${PHYSX_STATIC_LIBS})
   message(STATUS " - ${lib}")
endforeach()

include_directories("${PHYSX_ROOT_DIR}/include")  


## Swig 
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set(CMAKE_SWIG_FLAGS "")
set(CMAKE_SWIG_OUTDIR ${CMAKE_BINARY_DIR}/Release)
set_source_files_properties(configuration.i PROPERTIES CPLUSPLUS ON)

set(MODULE_NAME "ModuleName")
swig_add_library(LibraryName 
                 LANGUAGE csharp
                 SOURCES configuration.i 
                 spicytech.cpp 
)
swig_link_libraries(LibraryName ${LIBRARIES})
target_include_directories(${SWIG_MODULE_LibraryName_REAL_NAME} PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
target_link_libraries(${SWIG_MODULE_LibraryName_REAL_NAME} PRIVATE CUDA::cudart)

## 'PROJEC_NAME\Assets\Plugins' structure 
add_custom_command(TARGET LibraryName POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/Assets/Plugins/SpicyTech"

    # Move the DLL
    COMMAND ${CMAKE_COMMAND} -E rename 
        "$<TARGET_FILE:LibraryName>" 
        "${CMAKE_SOURCE_DIR}/Assets/Plugins/LibraryName.dll"

    # Move the generated C# files
    COMMAND ${CMAKE_COMMAND} -E rename 
        "${CMAKE_SWIG_OUTDIR}/${MODULE_NAME}.cs"
        "${CMAKE_SOURCE_DIR}/Assets/Plugins/SpicyTech/${MODULE_NAME}.cs"

    COMMAND ${CMAKE_COMMAND} -E rename 
        "${CMAKE_SWIG_OUTDIR}/${MODULE_NAME}PINVOKE.cs"
        "${CMAKE_SOURCE_DIR}/Assets/Plugins/SpicyTech/${MODULE_NAME}PINVOKE.cs"
)
