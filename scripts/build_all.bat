@echo off
echo TroubleTanks - Universal Build Script
echo ===================================

echo Detecting available compilers...

set BUILD_METHOD=none

echo Checking for Visual Studio compiler...
where cl >nul 2>nul
if %errorlevel% equ 0 (
    set BUILD_METHOD=msvc
    goto build
)

echo Checking for MinGW...
where g++ >nul 2>nul
if %errorlevel% equ 0 (
    set BUILD_METHOD=mingw
    goto build
)

echo Checking for CMake...
where cmake >nul 2>nul
if %errorlevel% equ 0 (
    set BUILD_METHOD=cmake
    goto build
)

echo No compatible compiler found!
echo Please install one of the following:
echo  - Visual Studio Build Tools
echo  - MinGW
echo  - CMake
pause
exit /b 1

:build
echo.
echo Building with %BUILD_METHOD%...

if "%BUILD_METHOD%"=="msvc" (
    call build.bat
) else if "%BUILD_METHOD%"=="mingw" (
    call build_mingw.bat
) else if "%BUILD_METHOD%"=="cmake" (
    echo Creating build directory...
    cmake -B build
    if %errorlevel% neq 0 (
        echo Error creating CMake build directory!
        pause
        exit /b %errorlevel%
    )
    
    echo Building with CMake...
    cmake --build build
    if %errorlevel% neq 0 (
        echo Error building with CMake!
        pause
        exit /b %errorlevel%
    )
    
    echo Build successful! Executable is in the build directory.
)

echo.
echo Build process completed!
pause