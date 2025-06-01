cmake_minimum_required(VERSION 3.16)

cmake_policy(SET CMP0122 NEW) ## ensures C# bindings use .NET-standard naming??
project(simplest)

message("[WRAPPER] PHYSX_ROOT_DIR='${PHYSX_ROOT_DIR}' ")


## https://cmake.org/cmake/help/latest/module/FindSWIG.html 
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
#include (UseSWIG) ## remove!

include_directories(${PROJECT_SOURCE_DIR})
message("[WRAPPER] PROJECT_SOURCE_DIR='${PROJECT_SOURCE_DIR}' ")

#set_property(SOURCE example.i PROPERTY CPLUSPLUS ON) # remove??
swig_add_library(SWIG_fileName LANGUAGE csharp SOURCES example.i)

#add_library(LibraryName SHARED "example.c") # LINK : fatal error LNK1181: cannot open input file 'LibraryName.lib'
add_library(LibraryName STATIC example.c)
##set_target_properties(LibraryName PROPERTIES OUTPUT_NAME "LibraryName")

## 0.0. Setting... 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

swig_link_libraries(SWIG_fileName LibraryName ${LIBRARIES})
