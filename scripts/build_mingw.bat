@echo off
echo TroubleTanks - MinGW Build Script
echo ===============================

echo Checking for MinGW...
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: MinGW g++ not found in PATH!
    echo Please install MinGW or add it to your PATH environment variable.
    echo You can download MinGW from: http://www.mingw.org/
    pause
    exit /b 1
)

echo Checking for windres...
where windres >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: windres not found in PATH!
    echo Please ensure MinGW bin directory is in your PATH.
    pause
    exit /b 1
)

echo Compiling resources...
windres resources.rc -O coff -o resources.res
if %errorlevel% neq 0 (
    echo Error compiling resources!
    pause
    exit /b %errorlevel%
)

echo Compiling application with MinGW (Windows GUI version)...
g++ -o TroubleTanks.exe main.cpp game.cpp network.cpp resources.res -lgdiplus -lws2_32 -lwinmm -lgdi32 -luser32 -lkernel32 -lshell32 -lole32 -ladvapi32 -static-libgcc -static-libstdc++ -mwindows -D_WIN32_WINNT=0x0600 -DUNICODE -D_UNICODE
if %errorlevel% neq 0 (
    echo Error compiling Windows GUI version!
    echo Trying console version...
    
    g++ -o TroubleTanks.exe main.cpp game.cpp network.cpp resources.res -lgdiplus -lws2_32 -lwinmm -lgdi32 -luser32 -lkernel32 -lshell32 -lole32 -ladvapi32 -static-libgcc -static-libstdc++ -D_WIN32_WINNT=0x0600 -DUNICODE -D_UNICODE
    if %errorlevel% neq 0 (
        echo Error compiling console version!
        echo Creating object files and linking manually...
        
        g++ -c main.cpp -D_WIN32_WINNT=0x0600 -DUNICODE -D_UNICODE
        if %errorlevel% neq 0 (
            echo Error compiling main.cpp!
            pause
            exit /b %errorlevel%
        )
        
        g++ -c game.cpp
        if %errorlevel% neq 0 (
            echo Error compiling game.cpp!
            pause
            exit /b %errorlevel%
        )
        
        g++ -c network.cpp
        if %errorlevel% neq 0 (
            echo Error compiling network.cpp!
            pause
            exit /b %errorlevel%
        )
        
        g++ -o TroubleTanks.exe main.o game.o network.o resources.res -lgdiplus -lws2_32 -lwinmm -lgdi32 -luser32 -lkernel32 -lshell32 -lole32 -ladvapi32 -static-libgcc -static-libstdc++
        if %errorlevel% neq 0 (
            echo Error linking object files!
            echo You may need to use Visual Studio or install additional MinGW libraries.
            del *.o *.res >nul 2>nul
            pause
            exit /b %errorlevel%
        )
        
        del *.o *.res >nul 2>nul
    )
)

echo Build successful!
echo Run TroubleTanks.exe to test the application.
pause