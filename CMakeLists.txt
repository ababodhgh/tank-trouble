cmake_minimum_required(VERSION 3.10)
project(TroubleTanks)

set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(PkgConfig REQUIRED)

# Add executable
add_executable(TroubleTanks
    src/main.cpp
    src/game.cpp
    src/network.cpp
    src/resources.rc
)

# Set Windows-specific properties
if(WIN32)
    # Enable Windows subsystem
    set_target_properties(TroubleTanks PROPERTIES
        WIN32_EXECUTABLE ON
    )
    
    # Link Windows libraries
    target_link_libraries(TroubleTanks
        gdiplus
        ws2_32
        winmm
        gdi32
        user32
        kernel32
        shell32
        ole32
        advapi32
    )
    
    # Set subsystem for GUI application
    target_link_options(TroubleTanks PRIVATE "/SUBSYSTEM:WINDOWS")
endif()

# Include directories
target_include_directories(TroubleTanks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Set compiler definitions
target_compile_definitions(TroubleTanks PRIVATE UNICODE _UNICODE)

# Copy PNG files to build directory if needed
configure_file(assets/TANK1.png TANK1.png COPYONLY)
configure_file(assets/TANK2.png TANK2.png COPYONLY)
configure_file(assets/TANK1_BULLET.png TANK1_BULLET.png COPYONLY)
configure_file(assets/TANK2_BULLET.png TANK2_BULLET.png COPYONLY)
configure_file(assets/WALL.png WALL.png COPYONLY)