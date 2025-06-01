cmake_minimum_required(VERSION 3.16)
project(simplest CXX)

## 0.0. Setting... 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUERIRED ON)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") 


## 1.0. HEADERS 
include_directories("${PHYSX_ROOT_DIR}/include")  


## 2.0. LIBRARIES 
set(PHYSX_LIB_DIR "${PHYSX_ROOT_DIR}/bin/win.x86_64.vc143.mt/release")
file(GLOB PHYSX_STATIC_LIBS "${PHYSX_LIB_DIR}/*_64.lib") # Get only static libraries

message(STATUS "Using static PhysX libraries:")
foreach(lib ${PHYSX_STATIC_LIBS})
   message(STATUS " - ${lib}")
endforeach()

## 3.0. EXECUTABLE 
##set(SRCS "../SnippetHelloWorld.cpp")
message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp") 
message(STATUS "Files:")
foreach(file ${SRCS})
   message(STATUS " - ${file}")
endforeach()

set(EXECUTABLE "snippetHelloWorld")
add_executable(${EXECUTABLE} ${SRCS})
##set_target_properties(${EXECUTABLE} PROPERTIES SUFFIX ".x")

target_link_libraries(${EXECUTABLE} ${PHYSX_STATIC_LIBS})

## 4.0. TESTING  
enable_testing()
add_test(NAME ${EXECUTABLE} COMMAND ${EXECUTABLE})

