project(papify C)
cmake_minimum_required(VERSION 2.8)

# Simple config: I want 'papify' binary to be built in the build folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Check if papi is properly installed on the system
find_library(papilib NAMES papi REQUIRED)
if(NOT papilib)
	message(FATAL_ERROR "The installation of Papi library is required to compile Papify. Please refer to http://icl.cs.utk.edu/papi/ to get more information on how to compile and install Papi.")
endif()

# Ensure extra libraries headers will be available for all the compilation process
include_directories(
	libs/roxml/include
)

add_subdirectory(libs)
add_subdirectory(src)
