@echo off
echo Testing MinGW Compilation
echo ====================

echo Compiling resources...
windres resources.rc -O coff -o resources.res
if %errorlevel% neq 0 (
    echo Error compiling resources!
    exit /b %errorlevel%
)

echo Compiling source files...
g++ -c main.cpp -D_WIN32_WINNT=0x0600 -DUNICODE -D_UNICODE
if %errorlevel% neq 0 (
    echo Error compiling main.cpp!
    exit /b %errorlevel%
)

g++ -c game.cpp
if %errorlevel% neq 0 (
    echo Error compiling game.cpp!
    exit /b %errorlevel%
)

g++ -c network.cpp
if %errorlevel% neq 0 (
    echo Error compiling network.cpp!
    exit /b %errorlevel%
)

echo All source files compiled successfully!
echo Object files created: main.o game.o network.o
dir *.o
del *.o *.res