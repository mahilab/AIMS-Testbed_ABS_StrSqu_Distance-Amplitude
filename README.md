# HapEE-Control

This project contains code necessary to run the core of the HapEE system (Haptic Experience Emulator system), the data saved from this experiment as well as the code used to analyze the data. The required external libraries to build the code are:

- Maxon EPOS Controller Command Library - EposCmd64
- National Instruments Daq Library - NIDAQ
- OpenCV's library - opencv_world401

Use Cmake V3.7 or more recent to build this project. 

Steps to build this project in Microsoft Visual Studio are as follows:

1. Open CMakeLists.txt in main project folder and adjust the include and link directories to the correct sources for all external libraries

2. Using cmd window, navigate to the directory housing the project

3. Enter the following commands:

   ```powershell
   cd HapEE-Control     # change directory to ./HapEE-Control
   mkdir build  		# make new directory for our out-of-place build
   cd build    		# change directory to ./HapEE-Control/build
   cmake .. -G "Visual Studio 15 2017 Win64" # generates all the build files
   ```

4. Assuming no errors have been thrown this far and all library linking is correct, once the build files are generated, use CMake to compile the project initially using the following command

   ```powershell
   cmake --build . --config Release # compiles the build in ./bin/Release
   ```

5. Copy all .dll files for external libraries (if being used as a dynamic rather than static library) into the ./bin/Release folder and run the program from the executable there

6. Compiling can now be done within Visual Studio and then run from the release executable. To run from Visual Studio, the .dlls must be copied into the main project build folder as well.