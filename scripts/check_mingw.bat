@echo off
echo Checking MinGW Installation
echo =========================

echo Checking for g++...
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo g++ not found!
    echo Please install MinGW or add it to your PATH.
) else (
    echo g++ found:
    for /f "tokens=*" %%i in ('where g++') do echo   %%i
    g++ --version | head -n 1
)

echo.
echo Checking for windres...
where windres >nul 2>nul
if %errorlevel% neq 0 (
    echo windres not found!
    echo Please ensure MinGW bin directory is in your PATH.
) else (
    echo windres found:
    for /f "tokens=*" %%i in ('where windres') do echo   %%i
)

echo.
echo Checking for mingw32-make...
where mingw32-make >nul 2>nul
if %errorlevel% neq 0 (
    echo mingw32-make not found!
) else (
    echo mingw32-make found:
    for /f "tokens=*" %%i in ('where mingw32-make') do echo   %%i
)

echo.
echo If all tools are found, you can compile the project by running build_mingw.bat
pause
exit