cmake_minimum_required(VERSION 3.10)
project(MissileBatteryCommand)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# Cocos2d-x Integration
# ============================================================================
# Set COCOS2DX_ROOT to your cocos2d-x installation path
# Example: cmake -DCOCOS2DX_ROOT=/path/to/cocos2d-x ..
if(NOT DEFINED COCOS2DX_ROOT)
    if(DEFINED ENV{COCOS2DX_ROOT})
        set(COCOS2DX_ROOT $ENV{COCOS2DX_ROOT})
    else()
        # Default search paths
        set(COCOS2DX_SEARCH_PATHS
            "${CMAKE_SOURCE_DIR}/cocos2d-x"
            "${CMAKE_SOURCE_DIR}/external/cocos2d-x"
            "$ENV{HOME}/cocos2d-x"
            "/usr/local/lib/cocos2d-x"
        )
        foreach(SEARCH_PATH ${COCOS2DX_SEARCH_PATHS})
            if(EXISTS "${SEARCH_PATH}/CMakeLists.txt")
                set(COCOS2DX_ROOT "${SEARCH_PATH}")
                break()
            endif()
        endforeach()
    endif()
endif()

if(COCOS2DX_ROOT AND EXISTS "${COCOS2DX_ROOT}/CMakeLists.txt")
    message(STATUS "Found cocos2d-x at: ${COCOS2DX_ROOT}")
    set(BUILD_ENGINE_DONE ON)

    # Include cocos2d-x build
    add_subdirectory(${COCOS2DX_ROOT}/cocos ${ENGINE_BINARY_PATH}/cocos/core)
    set(COCOS2D_FOUND TRUE)
else()
    message(WARNING
        "cocos2d-x not found. Building in STUB mode for development.\n"
        "To build with cocos2d-x:\n"
        "  1. Download cocos2d-x from https://cocos2d-x.org/download\n"
        "  2. Extract to ${CMAKE_SOURCE_DIR}/cocos2d-x\n"
        "  3. Or set -DCOCOS2DX_ROOT=/path/to/cocos2d-x\n"
    )
    set(COCOS2D_FOUND FALSE)
endif()

# ============================================================================
# Source Files
# ============================================================================
set(GAME_SOURCES
    src/main.cpp
    src/AppDelegate.cpp
    src/RadarScene.cpp
    src/RadarDisplay.cpp
    src/Aircraft.cpp
    src/AircraftGenerator.cpp
    src/IFFSystem.cpp
    src/TrackManager.cpp
    src/MissileBattery.cpp
    src/FireControlSystem.cpp
    src/GameHUD.cpp
    src/GameConfig.cpp
)

set(GAME_HEADERS
    include/AppDelegate.h
    include/RadarScene.h
    include/RadarDisplay.h
    include/Aircraft.h
    include/AircraftGenerator.h
    include/IFFSystem.h
    include/TrackManager.h
    include/MissileBattery.h
    include/FireControlSystem.h
    include/GameHUD.h
    include/GameConfig.h
    include/GameTypes.h
)

# ============================================================================
# Build Target
# ============================================================================
add_executable(${PROJECT_NAME} ${GAME_SOURCES} ${GAME_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

if(COCOS2D_FOUND)
    target_link_libraries(${PROJECT_NAME} cocos2d)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_COCOS2DX=1)
else()
    # Stub mode: compile without cocos2d-x for code validation
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_COCOS2DX=0)
endif()

# ============================================================================
# Resource Copying
# ============================================================================
if(EXISTS "${CMAKE_SOURCE_DIR}/Resources")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources"
    )
endif()

message(STATUS "Missile Battery Command - Build configured")
message(STATUS "  cocos2d-x: ${COCOS2D_FOUND}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
