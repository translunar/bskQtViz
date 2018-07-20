# AVS Lab Visualization Tool

## Installation Instructions
The following describes how to set up an IDE to develop and run the spacecraft 
visualization code on
- PC computers using **Microsoft Windows** operating system and **Microsoft Visual
    Studio 2013 Express**
    - If you would like to use a different version of Visual Studio you will
      have to build the boost libraries for your version of VS and modify the
      CMakeLists.txt file
- **Intel Mac** computers using **Mac OS-X** operating system and **XCode**

It is assumed you have downloaded an updated copy of the code from the GIT repository
<git@schaub-dt.colorado.edu:/Repository.git/Visualization> into a directory on your
machine (SOURCE_DIR).

Instructions are provided to set up the code using CMake. Additional instructions 
are included for how to properly compile boost libraries to run on both Windows 
and Mac OS X machines, and for how to compile CSPICE.

To run the code you will also have to install the QT framework, which can be 
downloaded from: <http://www.qt.io/download-open-source/>

The application takes a single input parameter as the full path to the visualization
data directory. The parameter can be defined through the IDE or at the command line.

### Compiling Visualization Tool with CMake
- CMakeList.txt files are included so that the project can be built with CMake
- Install [CMake](http://www.cmake.org)
- Run CMake
    - Select generator based on your system
        - Visual Studio 13
        - XCode
    - Enter in the fields:
        - Where is the source code: SOURCE_DIR
        - Where to build the binaries: SOURCE_DIR/build
    - Click Configure
    - Click Configure
    - Click Generate
- Note: CMake will create a project and executable with the same name as the directory in which it was run.
To change this name, modify the property EXE_NAME before configuring the second time
    
Special instructions for users of Visual Studio:
- Open project
- In Solution Explorer right click {DirectoryName} and select "Set as Startup Project"
- Properties -> Configuration Properties -> Debugging
    - Working Directory = $(SolutionDir)bin\$(Configuration)\

### Compiling Boost libraries

- Download the latest version of [boost](http://www.boost.org)

#### Windows
- Start the Visual Studio Developer Command Prompt
- Navigate to the boost directory
- Run: .\bootstrap.bat
- Run: .\b2 --toolset=msvc-XX.0 --with-NAME --build-type=complete stage
    - where XX = 10 for Visual Studio 2010, 11 for Visual Studio 2012, 12 for Visual Studio 2013
    - where NAME = system, filesystem, serialization, thread, date_time, chrono
- Open Stage folder
- Copy libboost_NAME-vcXX0-mt-1_YY.lib to External\boost_1_XX_0

#### Mac
- Open terminal window
- Navigate to the boost directory
- Run: ./bootstrap.sh
- Run: ./b2 --toolset=clang --cxxflags="-std=c++11 -stdlib=libc++" --linkflags="-stdlib=libc++" --with-NAME stage
    - where NAME = system, filesystem, serialization, thread
- Open Stage folder
- Copy libboost_system.a and libboost_program_options.a to External/boost_1_XX_0

### Compiling CSPICE

The following special instructions should be followed in addition to the default CSPICE compilation instructions.

#### Windows
- Modify line 23 of external/cspice/mkprodct.bat to specify the type of build you want (/MD vs /MT)
- Copy mkprodct to {CSPICE_DIR}/src/cspice where {CSPICE} is the directory where CSPICE has been downloaded to
- Run .\mkprodct.bat
- Copy cspice.dll and cspice.lib to external/cspice/lib
- Rename to match boost convention