# Visual Studio Project Setup Guide

## Creating a New Project

1. Open Visual Studio
2. Select "Create a new project"
3. Choose "Windows Desktop Application" or "Win32 Project"
4. Name the project "TroubleTanks" and choose a location
5. Click "Create"

## Project Configuration

### 1. Project Properties Setup

Right-click on the project in Solution Explorer and select "Properties":

#### Configuration Properties → C/C++ → General
- Additional Include Directories: Add any necessary paths

#### Configuration Properties → C/C++ → Preprocessor
- Preprocessor Definitions: Add `UNICODE` and `_UNICODE`

#### Configuration Properties → Linker → General
- Additional Library Directories: Add paths if needed

#### Configuration Properties → Linker → Input
- Additional Dependencies: Add these libraries:
  - `gdiplus.lib`
  - `ws2_32.lib`
  - `winmm.lib`
  - `gdi32.lib`
  - `user32.lib`
  - `kernel32.lib`
  - `shell32.lib`
  - `ole32.lib`
  - `advapi32.lib`

### 2. Adding Source Files

1. Remove any generated files from the project
2. Add the following files to the project:
   - `main.cpp`
   - `game.cpp`
   - `network.cpp`
   - `game.h`
   - `network.h`
   - `main.h`
   - `resource.h`
   - `resources.rc`

### 3. Resource Configuration

The `resources.rc` file should automatically be recognized as a resource file. Make sure:
1. The PNG files (`TANK1.png`, `TANK2.png`, `TANK1_BULLET.png`, `TANK2_BULLET.png`, `WALL.png`) are in the project directory
2. The resource compiler will embed these files into the executable

### 4. Build Configuration

#### For x86 (32-bit):
- Set Platform to "x86" in the configuration manager

#### For x64 (64-bit):
- Set Platform to "x64" in the configuration manager

## Building the Project

1. Select "Build" → "Build Solution" or press F7
2. The executable `TroubleTanks.exe` will be generated in the output directory

## Running the Application

After building, you can run the application directly from Visual Studio by pressing F5 or selecting "Debug" → "Start Without Debugging".

## Troubleshooting

### Common Issues:

1. **Missing Libraries**: Make sure all required libraries are added to the linker dependencies
2. **Resource Compilation Errors**: Ensure all PNG files are in the project directory
3. **Windows SDK Issues**: Make sure you have the Windows SDK installed
4. **C++ Standard**: Ensure C++17 or later is selected in the project properties

### Debugging Tips:

1. Use breakpoints in `main.cpp` to step through the code
2. Check the Output window for compilation errors
3. Verify that the resource file compiles without errors