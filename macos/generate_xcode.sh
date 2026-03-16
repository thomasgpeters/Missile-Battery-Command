#!/bin/bash
# ============================================================================
# Generate Xcode project for Missile Battery Command
#
# Usage:
#   ./macos/generate_xcode.sh [/path/to/cocos2d-x]
#
# This generates a self-contained Xcode project that builds a .app bundle
# with all dependencies embedded. Users can download the .app and run it
# without installing cocos2d-x or any other libraries.
# ============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build-xcode"

# Determine cocos2d-x path
COCOS2DX_ROOT="${1:-}"
if [ -z "$COCOS2DX_ROOT" ]; then
    # Auto-detect from common locations
    for SEARCH_PATH in \
        "${PROJECT_DIR}/cocos2d-x" \
        "${PROJECT_DIR}/external/cocos2d-x" \
        "$HOME/cocos2d-x" \
        "$HOME/Developer/cocos2d-x" \
        "/usr/local/lib/cocos2d-x"; do
        if [ -f "${SEARCH_PATH}/CMakeLists.txt" ]; then
            COCOS2DX_ROOT="$SEARCH_PATH"
            break
        fi
    done
fi

echo "=========================================="
echo "  Missile Battery Command — Xcode Setup"
echo "=========================================="
echo ""

if [ -n "$COCOS2DX_ROOT" ] && [ -f "${COCOS2DX_ROOT}/CMakeLists.txt" ]; then
    echo "  cocos2d-x: ${COCOS2DX_ROOT}"
    COCOS_FLAG="-DCOCOS2DX_ROOT=${COCOS2DX_ROOT}"
else
    echo "  cocos2d-x: NOT FOUND (building in stub mode)"
    echo ""
    echo "  To build with cocos2d-x, run:"
    echo "    $0 /path/to/cocos2d-x"
    echo ""
    COCOS_FLAG=""
fi

# Prefer static linking for self-contained bundles
STATIC_FLAG="-DBUILD_SHARED_LIBS=OFF"

echo "  Build dir:  ${BUILD_DIR}"
echo "  Static:     YES (self-contained app bundle)"
echo ""

# Create build directory
mkdir -p "$BUILD_DIR"

# Generate Xcode project
cmake -G Xcode \
    -S "$PROJECT_DIR" \
    -B "$BUILD_DIR" \
    ${COCOS_FLAG} \
    ${STATIC_FLAG} \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13

echo ""
echo "=========================================="
echo "  Xcode project generated!"
echo "=========================================="
echo ""
echo "  Open in Xcode:"
echo "    open ${BUILD_DIR}/MissileBatteryCommand.xcodeproj"
echo ""
echo "  Xcode build settings to verify:"
echo "    1. Select MissileBatteryCommand target"
echo "    2. Build Settings > Runpath Search Paths:"
echo "       @executable_path/../Frameworks"
echo "    3. Build Settings > Deployment Target: 10.13+"
echo "    4. Build Phases > Copy Files: Frameworks/"
echo ""
echo "  To build from command line:"
echo "    cmake --build ${BUILD_DIR} --config Release"
echo ""
echo "  The .app bundle will be at:"
echo "    ${BUILD_DIR}/Release/MissileBatteryCommand.app"
echo ""
