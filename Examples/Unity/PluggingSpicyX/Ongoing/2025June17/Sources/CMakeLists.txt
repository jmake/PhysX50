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


## 
file(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp") 
message(STATUS "Files:")
foreach(file ${SRCS})
   message(STATUS " - ${file}")
endforeach()

add_library(LibrarySnippet STATIC ${SRCS})

## Swig 
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set(CMAKE_SWIG_OUTDIR ${CMAKE_BINARY_DIR}/Release)

set(MODULE_NAME "SpicyModule") # -> see '%module' in the '.i' file 
set(LIBRARY_NAME "SpicyLibrary")

set(CMAKE_SWIG_FLAGS -namespace SpicyTech) 
set_source_files_properties(configuration.i PROPERTIES CPLUSPLUS ON)

swig_add_library(${LIBRARY_NAME} 
                 LANGUAGE csharp
                 SOURCES configuration.i 
                 spicytech.cpp 
)
swig_link_libraries(${LIBRARY_NAME} ${PHYSX_STATIC_LIBS} LibrarySnippet) ## ${LIBRARIES} 
target_include_directories(${SWIG_MODULE_${LIBRARY_NAME}_REAL_NAME} PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
target_link_libraries(${SWIG_MODULE_${LIBRARY_NAME}_REAL_NAME} CUDA::cudart)

## 'PROJEC_NAME\Assets\Plugins' structure 
add_custom_command(TARGET ${LIBRARY_NAME} POST_BUILD
    # Create target directories
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/Assets/Plugins"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/Assets/Plugins/SpicyTech"

    # Move the DLL
    COMMAND ${CMAKE_COMMAND} -E rename 
        "$<TARGET_FILE:${LIBRARY_NAME}>" 
        "${CMAKE_SOURCE_DIR}/Assets/Plugins/${LIBRARY_NAME}.dll"

    # Move Unity test file
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
        "${CMAKE_SOURCE_DIR}/Unity" 
        "${CMAKE_SOURCE_DIR}/Assets/Plugins/Scripts"

    # Move all generated .cs files (${MODULE_NAME}.cs", ${MODULE_NAME}PINVOKE.cs", MyClass.cs, etc.)
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
        "${CMAKE_SWIG_OUTDIR}" 
        "${CMAKE_SOURCE_DIR}/Assets/Plugins/SpicyTech"

)
