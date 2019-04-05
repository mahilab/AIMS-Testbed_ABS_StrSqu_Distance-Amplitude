cmake_minimum_required(VERSION 3.7)

#===============================================================================
# MY PROJECT
#===============================================================================

# create project
project(AbsoluteThreshold_HAPEE VERSION 0.1.0 LANGUAGES CXX)

# find MEL::MEL and all available MEL::xxx modules
find_package(MEL REQUIRED)

# include directories
include_directories(
    "include"              # your include directory
    "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/include" # NIDAQ's include directory
    "C:/Program Files (x86)/maxon motor ag/EPOS Positioning Controller/EPOS4/04 Programming/Windows DLL/Microsoft Visual C++/Definition Files" # EPOS4's directory
	"C:/Git/opencv/Install/opencv/include" # OpenCV's main include directory
)

# indicate where all used .lib are
link_directories(
	"C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/lib64/msvc" # NIDAQ's lib file
    "C:/Program Files (x86)/maxon motor ag/EPOS Positioning Controller/EPOS4/04 Programming/Windows DLL/Microsoft Visual C++/Definition Files" # EPOS4's directory
	"C:/Git/opencv/Install/opencv/x64/vc15/lib"
)

# remove or uncomment if linking MEL statically
# add_definitions(-DMEL_STATIC)

# create application
add_executable(Threshold_Tests
    include/MaxonMotor.hpp
    include/TrialList.hpp
    include/DaqNI.hpp
    src/MaxonMotor.cpp
    src/TrialList.cpp
    src/DaqNI.cpp
    src/Threshold_Tests.cpp
)

# link MEL
target_link_libraries(Threshold_Tests
    MEL::MEL
    NIDAQmx.lib
    EposCmd64.lib
    opencv_world401.lib
)
