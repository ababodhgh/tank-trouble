# Docker Compilation Setup

This Docker setup allows you to compile the TroubleTanks project in a consistent environment regardless of your local system configuration.

## Prerequisites

- Docker installed on your system
- Basic understanding of Docker commands

## Dockerfile

Create a `Dockerfile` with the following content:

```dockerfile
# Use Windows Server Core with Visual Studio Build Tools
FROM mcr.microsoft.com/windows/servercore:ltsc2019

# Install Chocolatey
RUN powershell -NoProfile -ExecutionPolicy Bypass -Command \
    "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))"

# Install Visual Studio Build Tools
RUN choco install visualstudio2019buildtools -y

# Install Windows SDK
RUN choco install windows-sdk-10-version-2004-windbg -y

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Compile the project
RUN powershell -Command \
    "cd 'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\' && \
    vcvars64.bat && \
    cd C:\app && \
    rc resources.rc && \
    cl /EHsc /Fe:TroubleTanks.exe main.cpp game.cpp network.cpp resources.res \
    gdiplus.lib ws2_32.lib winmm.lib user32.lib gdi32.lib shell32.lib ole32.lib advapi32.lib"
```

## Docker Compose (Alternative)

Create a `docker-compose.yml` file:

```yaml
version: '3.8'
services:
  build:
    build: .
    volumes:
      - .:/app
    command: >
      powershell -Command "
      cd 'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\' && 
      vcvars64.bat && 
      cd C:\app && 
      rc resources.rc && 
      cl /EHsc /Fe:TroubleTanks.exe main.cpp game.cpp network.cpp resources.res 
      gdiplus.lib ws2_32.lib winmm.lib user32.lib gdi32.lib shell32.lib ole32.lib advapi32.lib"
```

## Building with Docker

### Using Dockerfile directly:

```bash
docker build -t troubletanks-builder .
docker run --rm -v ${PWD}:/app troubletanks-builder
```

### Using Docker Compose:

```bash
docker-compose up --build
```

## Cross-Platform Docker Alternative

For a lighter-weight cross-platform option, you can use MinGW in a Linux container:

```dockerfile
FROM debian:bullseye

# Install MinGW cross-compiler
RUN apt-get update && apt-get install -y \
    gcc-mingw-w64 \
    binutils-mingw-w64 \
    cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy project files
COPY . .

# Compile using MinGW
RUN x86_64-w64-mingw32-windres resources.rc -O coff -o resources.res && \
    x86_64-w64-mingw32-g++ -o TroubleTanks.exe main.cpp game.cpp network.cpp resources.res \
    -lgdiplus -lws2_32 -lwinmm -lgdi32 -luser32 -lkernel32 -lshell32 -lole32 -ladvapi32 \
    -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic
```

## Notes

1. Windows containers require Windows host systems
2. Cross-compilation from Linux to Windows produces larger binaries but is more portable
3. The resulting executable will be compatible with Windows systems
4. Docker containers provide a consistent build environment regardless of the host system