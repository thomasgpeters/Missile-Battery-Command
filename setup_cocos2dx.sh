#!/bin/bash
# ============================================================================
# Missile Battery Command — cocos2d-x Setup Script
#
# This script downloads cocos2d-x v4, installs system dependencies,
# and builds the game in GUI mode.
#
# Usage:
#   chmod +x setup_cocos2dx.sh
#   ./setup_cocos2dx.sh
#
# After setup, the game binary will be at:
#   build/MissileBatteryCommand
# ============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
COCOS_VERSION="4.0"
COCOS_DIR="$SCRIPT_DIR/cocos2d-x"

echo "============================================"
echo " Missile Battery Command — cocos2d-x Setup"
echo "============================================"
echo ""

# ============================================================================
# Step 1: Install system dependencies
# ============================================================================
echo "[1/5] Checking system dependencies..."

install_deps_debian() {
    echo "  Installing dependencies (apt)..."
    sudo apt-get update -qq
    sudo apt-get install -y -qq \
        build-essential cmake git \
        libx11-dev libxrandr-dev libxi-dev libxxf86vm-dev \
        libgl1-mesa-dev libglu1-mesa-dev \
        libglew-dev libglfw3-dev \
        libfreetype6-dev libfontconfig1-dev \
        libcurl4-openssl-dev libssl-dev \
        libjpeg-dev libpng-dev libwebp-dev \
        libtiff-dev libsqlite3-dev \
        libchipmunk-dev \
        zip unzip \
        python3
}

install_deps_fedora() {
    echo "  Installing dependencies (dnf)..."
    sudo dnf install -y \
        gcc-c++ cmake git \
        libX11-devel libXrandr-devel libXi-devel libXxf86vm-devel \
        mesa-libGL-devel mesa-libGLU-devel \
        glew-devel glfw-devel \
        freetype-devel fontconfig-devel \
        libcurl-devel openssl-devel \
        libjpeg-turbo-devel libpng-devel libwebp-devel \
        libtiff-devel sqlite-devel \
        zip unzip \
        python3
}

install_deps_macos() {
    echo "  Installing dependencies (brew)..."
    brew install cmake glew glfw freetype fontconfig curl openssl \
        libjpeg libpng webp libtiff sqlite python3 2>/dev/null || true
}

if [[ "$OSTYPE" == "linux"* ]]; then
    if command -v apt-get &>/dev/null; then
        install_deps_debian
    elif command -v dnf &>/dev/null; then
        install_deps_fedora
    else
        echo "  WARNING: Unknown Linux distro. Install OpenGL, GLEW, GLFW,"
        echo "  freetype, curl, and image libs manually."
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    install_deps_macos
else
    echo "  WARNING: Unsupported OS. Install dependencies manually."
fi

echo "  Done."
echo ""

# ============================================================================
# Step 2: Download cocos2d-x
# ============================================================================
echo "[2/5] Downloading cocos2d-x v${COCOS_VERSION}..."

if [ -d "$COCOS_DIR" ] && [ -f "$COCOS_DIR/CMakeLists.txt" ]; then
    echo "  cocos2d-x already exists at $COCOS_DIR — skipping download."
else
    echo "  Cloning cocos2d-x from GitHub..."
    git clone --depth 1 --branch v4 \
        https://github.com/cocos2d/cocos2d-x.git "$COCOS_DIR"

    echo "  Downloading cocos2d-x dependencies..."
    cd "$COCOS_DIR"
    python3 download-deps.py --remove-download yes 2>/dev/null || \
    python download-deps.py --remove-download yes 2>/dev/null || \
    echo "  NOTE: download-deps.py failed — you may need to run it manually."

    # NOTE: We intentionally skip 'git submodule update'. The submodules
    # (ccs-res, bindings-generator, etc.) are only needed for cocos2d-x's
    # own test suite and Python bindings — not for building the game.
    # They also use the deprecated git:// protocol which GitHub no longer
    # supports, causing "Operation timed out" errors.
    #
    # If you do need submodules for development, first rewrite the protocol:
    #   git config --global url."https://".insteadOf git://

    cd "$SCRIPT_DIR"
fi

echo "  Done."
echo ""

# ============================================================================
# Step 3: Configure build
# ============================================================================
echo "[3/5] Configuring CMake build..."

BUILD_DIR="$SCRIPT_DIR/build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake "$SCRIPT_DIR" \
    -DCOCOS2DX_ROOT="$COCOS_DIR" \
    -DCMAKE_BUILD_TYPE=Release

echo "  Done."
echo ""

# ============================================================================
# Step 4: Build
# ============================================================================
echo "[4/5] Building Missile Battery Command..."

make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "  Done."
echo ""

# ============================================================================
# Step 5: Report
# ============================================================================
echo "[5/5] Build complete!"
echo ""
echo "============================================"
echo " Missile Battery Command — Ready to Play"
echo "============================================"
echo ""
echo " Binary:  $BUILD_DIR/MissileBatteryCommand"
echo " Tests:   $BUILD_DIR/MissileBatteryCommandTests"
echo ""
echo " To run the game:"
echo "   cd $BUILD_DIR"
echo "   ./MissileBatteryCommand"
echo ""
echo " Controls:"
echo "   Click    — Select track on radar"
echo "   1-3      — Assign PATRIOT battery"
echo "   4-6      — Assign HAWK battery"
echo "   7-9      — Assign JAVELIN battery"
echo "   F        — Fire authorized"
echo "   A        — Abort engagement"
echo ""
echo "============================================"
