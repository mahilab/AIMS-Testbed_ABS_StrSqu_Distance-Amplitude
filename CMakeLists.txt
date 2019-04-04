cmake_minimum_required(VERSION 3.7)

#===============================================================================
# SETUP CROSS COMPILER (uncomment or remove as needed)
#===============================================================================

# NI X64 CROSS COMPILER

# # add compiler to path
# LIST(APPEND CMAKE_PROGRAM_PATH ${NI_X64_ROOT}/sysroots/i686-nilrtsdk-mingw32/usr/bin/x86_64-nilrt-linux)
# # set compiler (must be called before project/enable)
# set(CMAKE_C_COMPILER x86_64-nilrt-linux-gcc)
# set(CMAKE_CXX_COMPILER x86_64-nilrt-linux-g++)
# # set to cross compile
# set(CMAKE_SYSTEM_NAME Generic)
# set(CMAKE_CROSSCOMPILING 1)
# # setup sysroot (for NI RT cross compiler)
# set(CMAKE_SYSROOT ${NI_X64_ROOT}/sysroots/core2-64-nilrt-linux)

# NI ARM CROSS COMPILER

# # add compiler to path
# LIST(APPEND CMAKE_PROGRAM_PATH ${NI_ARM_ROOT}/sysroots/i686-nilrtsdk-mingw32/usr/bin/arm-nilrt-linux-gnueabi)
# # set compiler (must be called before project/enable)
# set(CMAKE_C_COMPILER arm-nilrt-linux-gnueabi-gcc)
# set(CMAKE_CXX_COMPILER arm-nilrt-linux-gnueabi-g++)
# # set to cross compile
# set(CMAKE_SYSTEM_NAME Generic)
# set(CMAKE_CROSSCOMPILING 1)
# # setup sysroot (for NI RT cross compiler)
# set(CMAKE_SYSROOT ${NI_ARM_ROOT}/sysroots/cortexa9-vfpv3-nilrt-linux-gnueabi)

#===============================================================================
# MY PROJECT
#===============================================================================

# enable C++11
enable_language(C)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)

# create project
project(AbsoluteThreshold_HAPEE)

# set binary output location to conventional "bin" (optional, but recommended)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# include directories
include_directories(
    "include"              # your include directory
    "C:/Git/MEL/include" # MEL's include directory
    "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/include" # NIDAQ's include directory
    "C:/Program Files (x86)/maxon motor ag/EPOS Positioning Controller/EPOS4/04 Programming/Windows DLL/Microsoft Visual C++/Definition Files" # EPOS4's directory
	"C:/Git/opencv/Install/opencv/include" # OpenCV's main include directory
)

# indicate where all used .lib are
link_directories(
	"C:/Git/MEL/lib/Release" # MEL's lib file
    "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/lib64/msvc" # NIDAQ's lib file
    "C:/Program Files (x86)/maxon motor ag/EPOS Positioning Controller/EPOS4/04 Programming/Windows DLL/Microsoft Visual C++/Definition Files" # EPOS4's directory
	"C:/Git/opencv/Install/opencv/x64/vc15/lib"
)

# remove or uncomment if linking MEL statically
# add_definitions(-DMEL_STATIC)

# create application
add_executable(
               HapEE-Control
               include/MaxonMotor.hpp
               include/TrialList.hpp
               include/DaqNI.hpp
               include/VideoCap.hpp
               src/MaxonMotor.cpp
               src/TrialList.cpp
               src/DaqNI.cpp
               src/VideoCap.cpp
               src/main.cpp)

# link MEL
target_link_libraries(HapEE-Control MEL NIDAQmx.lib EposCmd64.lib opencv_world401.lib)
