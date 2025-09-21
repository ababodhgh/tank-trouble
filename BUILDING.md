# Building TroubleTanks

This document provides simplified build instructions for getting TroubleTanks up and running quickly.

## Prerequisites

- Windows development environment
- One of the following:
  - Visual Studio (2019 or later recommended)
  - MinGW with g++ (7.0 or later)
  - CMake (3.10 or later)
  - Docker (for containerized builds)

## Quick Build Options

### Option 1: Visual Studio (Recommended for Windows)

1. Open Visual Studio
2. Create a new "Windows Desktop Application" project
3. Replace the generated files with the source files from this repository
4. Follow the detailed instructions in [VISUAL_STUDIO_SETUP.md](VISUAL_STUDIO_SETUP.md)
5. Build the solution (Ctrl+Shift+B)

### Option 2: MinGW

```cmd
# Using the provided batch script
build_mingw.bat

# Or manually:
windres resources.rc -O coff -o resources.res
g++ -o TroubleTanks.exe main.cpp game.cpp network.cpp resources.res -lgdiplus -lws2_32 -lwinmm -lgdi32 -luser32 -lkernel32 -lshell32 -lole32 -ladvapi32
```

### Option 3: CMake

```cmd
cmake -B build
cmake --build build
```

### Option 4: Docker

```bash
docker build -t troubletanks .
docker run --rm -v ${PWD}:/app troubletanks
```

## Troubleshooting

If you encounter build issues:

1. Ensure all dependencies are installed
2. Check that the PNG asset files are in the correct location
3. Verify your compiler supports C++17
4. Refer to the detailed setup guides for your specific build environment

For more detailed instructions, see:
- [VISUAL_STUDIO_SETUP.md](VISUAL_STUDIO_SETUP.md) for Visual Studio setup
- [DOCKER_BUILD.md](DOCKER_BUILD.md) for Docker builds