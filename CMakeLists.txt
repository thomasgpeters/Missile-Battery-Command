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
        message(STATUS "Using COCOS2DX_ROOT from environment: ${COCOS2DX_ROOT}")
    else()
        # Search common macOS installation locations
        set(COCOS2DX_SEARCH_PATHS
            "${CMAKE_SOURCE_DIR}/cocos2d-x"
            "${CMAKE_SOURCE_DIR}/external/cocos2d-x"
            "${CMAKE_SOURCE_DIR}/../cocos2d-x"
            "$ENV{HOME}/cocos2d-x"
            "$ENV{HOME}/Developer/cocos2d-x"
            "$ENV{HOME}/Developer/C-Projects/cocos2d-x"
            "$ENV{HOME}/Projects/cocos2d-x"
            "$ENV{HOME}/src/cocos2d-x"
            "/usr/local/lib/cocos2d-x"
            "/usr/local/cocos2d-x"
            "/opt/cocos2d-x"
            "/opt/homebrew/opt/cocos2d-x"
        )
        message(STATUS "COCOS2DX_ROOT not set. Searching common locations...")
        foreach(SEARCH_PATH ${COCOS2DX_SEARCH_PATHS})
            if(EXISTS "${SEARCH_PATH}/CMakeLists.txt")
                set(COCOS2DX_ROOT "${SEARCH_PATH}")
                message(STATUS "  Found cocos2d-x at: ${SEARCH_PATH}")
                break()
            else()
                message(STATUS "  Not found at: ${SEARCH_PATH}")
            endif()
        endforeach()
    endif()
endif()

if(COCOS2DX_ROOT AND EXISTS "${COCOS2DX_ROOT}/CMakeLists.txt")
    message(STATUS "Found cocos2d-x at: ${COCOS2DX_ROOT}")
    set(BUILD_ENGINE_DONE ON)

    # Disable precompiled headers — cocos2d-x PCH is built as C++ but
    # .mm (Objective-C++) files need ObjC enabled, causing a mismatch
    # on modern Xcode/AppleClang.
    set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)

    # Skip building cocos2d-x test/template projects — we only need
    # the cocos2d library itself. The actual variable is BUILD_TESTS.
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_LUA_LIBS OFF CACHE BOOL "" FORCE)
    set(BUILD_JS_LIBS OFF CACHE BOOL "" FORCE)

    # Set internal variables that cocos2d-x's sub-CMakeLists expect.
    set(COCOS2DX_ROOT_PATH "${COCOS2DX_ROOT}")
    set(ENGINE_BINARY_PATH "${CMAKE_BINARY_DIR}/engine")
    set(EXTERNAL_DIR "${COCOS2DX_ROOT}/external")

    # Include cocos2d-x from its root. EXCLUDE_FROM_ALL ensures only
    # targets our game depends on (cocos2d lib) are built.
    add_subdirectory(${COCOS2DX_ROOT} ${ENGINE_BINARY_PATH}/cocos2d-x EXCLUDE_FROM_ALL)

    # Disable PCH on cocos2d target directly (belt and suspenders)
    if(TARGET cocos2d)
        set_target_properties(cocos2d PROPERTIES DISABLE_PRECOMPILE_HEADERS ON)
    endif()

    # Link system zlib on macOS (needed for ZipUtils, arm64)
    if(APPLE)
        find_package(ZLIB REQUIRED)
    endif()

    set(COCOS2D_FOUND TRUE)
else()
    message(WARNING
        "cocos2d-x NOT FOUND — Building in STUB mode (no GUI).\n"
        "\n"
        "To build with GUI mode, do ONE of the following:\n"
        "\n"
        "  Option 1: Pass the path directly to CMake:\n"
        "    cmake -DCOCOS2DX_ROOT=/path/to/cocos2d-x ..\n"
        "\n"
        "  Option 2: Set the environment variable:\n"
        "    export COCOS2DX_ROOT=/path/to/cocos2d-x\n"
        "    cmake ..\n"
        "\n"
        "  Option 3: Symlink into the project:\n"
        "    ln -s /path/to/cocos2d-x ${CMAKE_SOURCE_DIR}/cocos2d-x\n"
        "    cmake ..\n"
        "\n"
        "  To find your cocos2d-x installation:\n"
        "    find ~ -name 'cocos2d-x' -type d -maxdepth 4 2>/dev/null\n"
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
    src/RadarBlip.cpp
    src/ConsoleFrame.cpp
    src/IntegratedConsoleScene.cpp
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
    src/DrawerBook.cpp
    src/WelcomeScreen.cpp
)

set(GAME_HEADERS
    include/AppDelegate.h
    include/RadarScene.h
    include/RadarDisplay.h
    include/RadarBlip.h
    include/ConsoleFrame.h
    include/IntegratedConsoleScene.h
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
    include/DrawerBook.h
    include/WelcomeScreen.h
)

# ============================================================================
# Build Target — macOS App Bundle when cocos2d-x is available
# ============================================================================
if(APPLE AND COCOS2D_FOUND)
    # Build as a macOS .app bundle for self-contained distribution
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${GAME_SOURCES} ${GAME_HEADERS})

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/macos/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "Missile Battery Command"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.imagerybusinesssystems.missilebatterycommand"
        MACOSX_BUNDLE_COPYRIGHT "© 2026 Imagery Business Systems, LLC"
        MACOSX_BUNDLE_ICON_FILE "AppIcon"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.imagerybusinesssystems.missilebatterycommand"
        XCODE_ATTRIBUTE_INFOPLIST_KEY_CFBundleDisplayName "Missile Battery Command"
    )

    # Embed dynamic libraries inside the app bundle Frameworks directory
    # This makes the app self-contained — users don't need to install dependencies
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "@executable_path/../Frameworks"
        INSTALL_RPATH "@executable_path/../Frameworks"
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../Frameworks"
    )

    # Copy Resources into the app bundle
    if(EXISTS "${CMAKE_SOURCE_DIR}/Resources")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Resources"
            "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/Resources"
        )
    endif()

    # Post-build: copy any cocos2d-x dylibs into Frameworks and fix install names
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/Frameworks"
        COMMENT "Creating Frameworks directory in app bundle"
    )

    # If cocos2d-x builds shared libraries, embed them
    if(TARGET cocos2d AND BUILD_SHARED_LIBS)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:cocos2d>"
                "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/Frameworks/"
            COMMENT "Embedding cocos2d dylib in app bundle"
        )
    endif()
else()
    # Non-macOS or stub mode: plain executable
    add_executable(${PROJECT_NAME} ${GAME_SOURCES} ${GAME_HEADERS})

    if(EXISTS "${CMAKE_SOURCE_DIR}/Resources")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Resources"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources"
        )
    endif()
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

if(COCOS2D_FOUND)
    # Prefer static linking for self-contained app bundle
    target_link_libraries(${PROJECT_NAME} cocos2d)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_COCOS2DX=1)
    # Link system zlib on macOS (uncompress symbol needed by ZipUtils)
    if(APPLE AND ZLIB_FOUND)
        target_link_libraries(${PROJECT_NAME} ZLIB::ZLIB)
    endif()
else()
    # Stub mode: compile without cocos2d-x for code validation
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_COCOS2DX=0)
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
    src/RadarBlip.cpp
    src/ConsoleFrame.cpp
    src/IntegratedConsoleScene.cpp
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
    src/DrawerBook.cpp
    src/WelcomeScreen.cpp
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
