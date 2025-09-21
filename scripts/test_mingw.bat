@echo off
echo Testing MinGW Compilation Fixes
echo =============================

echo Creating a simple test file to check MinGW compatibility...
echo.

echo Testing if we can compile a minimal Windows program...
echo #include ^<windows.h^> > test_minimal.cpp
echo int main^(^) { return 0; } >> test_minimal.cpp

g++ -o test_minimal.exe test_minimal.cpp -lgdi32 -luser32
if %errorlevel% neq 0 (
    echo Error: Cannot compile even a minimal Windows program with MinGW
    echo You may need to install additional MinGW packages or use a different compiler
    del test_minimal.cpp
    pause
    exit /b 1
)

del test_minimal.cpp test_minimal.exe
echo MinGW Windows compilation test passed!
echo.

echo Now trying to compile TroubleTanks with MinGW...
call build_mingw.bat