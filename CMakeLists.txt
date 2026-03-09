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
    src/AirspaceManager.cpp
    src/DataCard.cpp
    src/ThreatBoard.cpp
    src/BattalionHQ.cpp
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
    include/AirspaceManager.h
    include/DataCard.h
    include/ThreatBoard.h
    include/BattalionHQ.h
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

# ============================================================================
# Test Suite
# ============================================================================
enable_testing()

# Testable game library (everything except main.cpp)
set(GAME_LIB_SOURCES
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
    src/AirspaceManager.cpp
    src/DataCard.cpp
    src/ThreatBoard.cpp
    src/BattalionHQ.cpp
)

add_library(MissileBatteryCommandLib STATIC ${GAME_LIB_SOURCES} ${GAME_HEADERS})

target_include_directories(MissileBatteryCommandLib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

if(COCOS2D_FOUND)
    target_link_libraries(MissileBatteryCommandLib cocos2d)
    target_compile_definitions(MissileBatteryCommandLib PUBLIC USE_COCOS2DX=1)
else()
    target_compile_definitions(MissileBatteryCommandLib PUBLIC USE_COCOS2DX=0)
endif()

# Test executable
set(TEST_SOURCES
    tests/test_main.cpp
    tests/test_cocos2dx_validation.cpp
    tests/test_game_types.cpp
    tests/test_aircraft.cpp
    tests/test_iff_system.cpp
    tests/test_track_manager.cpp
    tests/test_missile_battery.cpp
    tests/test_aircraft_generator.cpp
    tests/test_fire_control.cpp
    tests/test_game_config.cpp
    tests/test_airspace_manager.cpp
    tests/test_data_card.cpp
    tests/test_threat_board.cpp
    tests/test_battalion_hq.cpp
)

add_executable(MissileBatteryCommandTests ${TEST_SOURCES})

target_include_directories(MissileBatteryCommandTests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests
)

target_link_libraries(MissileBatteryCommandTests MissileBatteryCommandLib)

# Register with CTest
add_test(NAME AllTests COMMAND MissileBatteryCommandTests)

# Interactive test harness (ncurses)
find_package(Curses QUIET)
if(CURSES_FOUND)
    set(HARNESS_TEST_SOURCES
        tests/test_harness.cpp
        tests/test_cocos2dx_validation.cpp
        tests/test_game_types.cpp
        tests/test_aircraft.cpp
        tests/test_iff_system.cpp
        tests/test_track_manager.cpp
        tests/test_missile_battery.cpp
        tests/test_aircraft_generator.cpp
        tests/test_fire_control.cpp
        tests/test_game_config.cpp
        tests/test_airspace_manager.cpp
        tests/test_data_card.cpp
        tests/test_threat_board.cpp
        tests/test_battalion_hq.cpp
    )

    add_executable(TestHarness ${HARNESS_TEST_SOURCES})

    target_include_directories(TestHarness PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/tests
        ${CURSES_INCLUDE_DIRS}
    )

    target_link_libraries(TestHarness MissileBatteryCommandLib ${CURSES_LIBRARIES})
    message(STATUS "  Test Harness: ON (ncurses found)")
else()
    message(STATUS "  Test Harness: OFF (ncurses not found)")
endif()

message(STATUS "Missile Battery Command - Build configured")
message(STATUS "  cocos2d-x: ${COCOS2D_FOUND}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Tests: ON")
