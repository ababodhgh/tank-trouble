@echo off
echo Debugging TroubleTanks Project
echo ===========================

echo Checking for secure string functions...
echo Searching for swprintf_s and wcscpy_s in main.cpp:
findstr "swprintf_s\|wcscpy_s" main.cpp >nul
if %errorlevel% equ 0 (
    echo ERROR: Found unhandled secure string functions!
    findstr /n "swprintf_s\|wcscpy_s" main.cpp
) else (
    echo OK: No unhandled secure string functions found.
)

echo.
echo Checking conditional compilation blocks...
echo Searching for __MINGW32__ blocks:
findstr "__MINGW32__" main.cpp >nul
if %errorlevel% equ 0 (
    echo OK: Found MinGW conditional compilation blocks.
) else (
    echo WARNING: No MinGW conditional compilation blocks found.
)

echo.
echo Checking function declarations...
echo Checking StartHosting declaration:
findstr "bool StartHosting" main.cpp network.h >nul
if %errorlevel% equ 0 (
    echo OK: StartHosting declarations match.
) else (
    echo WARNING: StartHosting declarations may not match.
)

echo.
echo Checking variable definitions...
echo Checking for duplicate g_soundEnabled:
findstr /c:"bool g_soundEnabled" main.cpp | find /c /v ""
set /p count=<nul
for /f %%i in ('findstr /c:"bool g_soundEnabled" main.cpp ^| find /c /v ""') do set count=%%i
if %count% gtr 1 (
    echo ERROR: Found %count% definitions of g_soundEnabled!
) else (
    echo OK: Single definition of g_soundEnabled found.
)

echo.
echo Debug check completed.