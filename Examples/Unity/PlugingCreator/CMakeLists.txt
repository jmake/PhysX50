cmake_minimum_required(VERSION 3.16)

cmake_policy(SET CMP0122 NEW) ## ensures C# bindings use .NET-standard naming??
project(example CXX C)

message("[WRAPPER] PHYSX_ROOT_DIR='${PHYSX_ROOT_DIR}' ")
include_directories(${PROJECT_SOURCE_DIR})

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

# Set language
set(CMAKE_SWIG_FLAGS "")

# Add the interface
set_source_files_properties(example.i PROPERTIES CPLUSPLUS ON)

# Set output directory
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_SWIG_OUTDIR ${CMAKE_BINARY_DIR}/Release)

# Add C# wrapper
swig_add_library(LibraryName 
  LANGUAGE csharp
  SOURCES example.i example.cpp 
)