@echo off
echo TroubleTanks - Phase 4 Build Script
echo ==================================

echo Compiling resources...
rc resources.rc
if %errorlevel% neq 0 (
    echo Error compiling resources!
    pause
    exit /b %errorlevel%
)

echo Compiling application...
cl /EHsc /Fe:TroubleTanks.exe main.cpp game.cpp network.cpp resources.res gdiplus.lib ws2_32.lib winmm.lib user32.lib gdi32.lib shell32.lib ole32.lib advapi32.lib
if %errorlevel% neq 0 (
    echo Error compiling application!
    pause
    exit /b %errorlevel%
)

echo Build successful!
echo Run TroubleTanks.exe to test the application.