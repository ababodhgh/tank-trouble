@echo off
echo Quick MinGW Fix Test
echo ===================

echo Checking for duplicate g_soundEnabled definitions...
findstr /C:"bool g_soundEnabled" main.cpp
echo.

echo Checking for StartHosting function declarations...
findstr /C:"StartHosting" main.cpp network.h
echo.

echo Checking for Tank::JustShot declaration...
findstr /C:"JustShot" game.h game.cpp
echo.

echo Test completed. If no errors above, the fixes should work.