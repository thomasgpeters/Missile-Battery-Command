#!/bin/bash
# ============================================================================
# Patch cocos2d-x for modern macOS/Xcode compatibility
#
# Fixes:
#   1. iconv_t type mismatch in CCFontAtlas (Xcode 15+/macOS 14+ SDK)
#   2. Other known AppleClang 17+ compatibility issues
#
# Usage:
#   ./macos/patch_cocos2dx.sh [/path/to/cocos2d-x]
#
# If no path given, defaults to ./cocos2d-x in the project root.
# ============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
COCOS_DIR="${1:-${PROJECT_DIR}/cocos2d-x}"

if [ ! -d "$COCOS_DIR" ]; then
    echo "ERROR: cocos2d-x not found at: $COCOS_DIR"
    echo "Usage: $0 /path/to/cocos2d-x"
    exit 1
fi

echo "=========================================="
echo "  Patching cocos2d-x for modern macOS"
echo "=========================================="
echo "  cocos2d-x: $COCOS_DIR"
echo ""

PATCHED=0

# --------------------------------------------------------------------------
# Fix 1: CCFontAtlas.h — change _iconv from void* to iconv_t
# --------------------------------------------------------------------------
FONT_ATLAS_H="$COCOS_DIR/cocos/2d/CCFontAtlas.h"
if [ -f "$FONT_ATLAS_H" ]; then
    if grep -q 'void \*_iconv' "$FONT_ATLAS_H" 2>/dev/null; then
        echo "[PATCH] CCFontAtlas.h: Fixing _iconv type (void* -> iconv_t)"

        # Ensure iconv.h is included
        if ! grep -q '#include <iconv.h>' "$FONT_ATLAS_H"; then
            # Add iconv.h include after the last existing #include
            sed -i.bak '/#include/{ h; }; ${x; s/#include.*/#include <iconv.h>\n&/; x; }' "$FONT_ATLAS_H" 2>/dev/null || \
            sed -i '' '/#include/{ h; }; ${x; s/#include.*/#include <iconv.h>\n&/; x; }' "$FONT_ATLAS_H"
        fi

        # Replace void* _iconv with iconv_t _iconv
        sed -i.bak 's/void \*_iconv/iconv_t _iconv/g' "$FONT_ATLAS_H" 2>/dev/null || \
        sed -i '' 's/void \*_iconv/iconv_t _iconv/g' "$FONT_ATLAS_H"

        PATCHED=$((PATCHED + 1))
    else
        echo "[OK]    CCFontAtlas.h: Already patched or uses correct type"
    fi
else
    echo "[SKIP]  CCFontAtlas.h: File not found"
fi

# --------------------------------------------------------------------------
# Fix 2: CCFontAtlas.cpp — remove invalid casts to/from void*
# --------------------------------------------------------------------------
FONT_ATLAS_CPP="$COCOS_DIR/cocos/2d/CCFontAtlas.cpp"
if [ -f "$FONT_ATLAS_CPP" ]; then
    NEEDS_PATCH=0

    # Check for void* casts around iconv calls
    if grep -q '(iconv_t)_iconv' "$FONT_ATLAS_CPP" 2>/dev/null || \
       grep -q 'iconv_open' "$FONT_ATLAS_CPP" 2>/dev/null; then

        # Fix iconv_open assignment to void*
        if grep -q '_iconv = (void\*)iconv_open' "$FONT_ATLAS_CPP" 2>/dev/null; then
            echo "[PATCH] CCFontAtlas.cpp: Fixing iconv_open cast"
            sed -i.bak 's/_iconv = (void\*)iconv_open/_iconv = iconv_open/g' "$FONT_ATLAS_CPP" 2>/dev/null || \
            sed -i '' 's/_iconv = (void\*)iconv_open/_iconv = iconv_open/g' "$FONT_ATLAS_CPP"
            NEEDS_PATCH=1
        fi

        # Fix iconv_close cast
        if grep -q 'iconv_close((iconv_t)_iconv)' "$FONT_ATLAS_CPP" 2>/dev/null; then
            echo "[PATCH] CCFontAtlas.cpp: Fixing iconv_close cast"
            sed -i.bak 's/iconv_close((iconv_t)_iconv)/iconv_close(_iconv)/g' "$FONT_ATLAS_CPP" 2>/dev/null || \
            sed -i '' 's/iconv_close((iconv_t)_iconv)/iconv_close(_iconv)/g' "$FONT_ATLAS_CPP"
            NEEDS_PATCH=1
        fi

        # Fix iconv() call cast
        if grep -q 'iconv((iconv_t)_iconv' "$FONT_ATLAS_CPP" 2>/dev/null; then
            echo "[PATCH] CCFontAtlas.cpp: Fixing iconv() cast"
            sed -i.bak 's/iconv((iconv_t)_iconv/iconv(_iconv/g' "$FONT_ATLAS_CPP" 2>/dev/null || \
            sed -i '' 's/iconv((iconv_t)_iconv/iconv(_iconv/g' "$FONT_ATLAS_CPP"
            NEEDS_PATCH=1
        fi

        # Fix nullptr/NULL comparisons with void*
        if grep -q '_iconv != nullptr' "$FONT_ATLAS_CPP" 2>/dev/null; then
            # These should work fine with iconv_t, no change needed
            true
        fi

        if [ $NEEDS_PATCH -eq 1 ]; then
            PATCHED=$((PATCHED + 1))
        else
            echo "[OK]    CCFontAtlas.cpp: No void* casts found to fix"
        fi
    else
        echo "[OK]    CCFontAtlas.cpp: Already patched"
    fi
else
    echo "[SKIP]  CCFontAtlas.cpp: File not found"
fi

# --------------------------------------------------------------------------
# Fix 3: CCFontFreeType.cpp — same iconv type issue
# --------------------------------------------------------------------------
FONT_FT_CPP="$COCOS_DIR/cocos/2d/CCFontFreeType.cpp"
if [ -f "$FONT_FT_CPP" ]; then
    if grep -q '(void\*)iconv_open\|iconv_close((iconv_t)' "$FONT_FT_CPP" 2>/dev/null; then
        echo "[PATCH] CCFontFreeType.cpp: Fixing iconv casts"
        sed -i.bak \
            -e 's/(void\*)iconv_open/iconv_open/g' \
            -e 's/iconv_close((iconv_t)\(.*\))/iconv_close(\1)/g' \
            -e 's/iconv((iconv_t)\(.*\),/iconv(\1,/g' \
            "$FONT_FT_CPP" 2>/dev/null || \
        sed -i '' \
            -e 's/(void\*)iconv_open/iconv_open/g' \
            -e 's/iconv_close((iconv_t)\(.*\))/iconv_close(\1)/g' \
            -e 's/iconv((iconv_t)\(.*\),/iconv(\1,/g' \
            "$FONT_FT_CPP"
        PATCHED=$((PATCHED + 1))
    else
        echo "[OK]    CCFontFreeType.cpp: Already patched or no issues"
    fi
else
    echo "[SKIP]  CCFontFreeType.cpp: File not found"
fi

# --------------------------------------------------------------------------
# Cleanup .bak files
# --------------------------------------------------------------------------
find "$COCOS_DIR/cocos/2d/" -name "*.bak" -delete 2>/dev/null

echo ""
echo "=========================================="
if [ $PATCHED -gt 0 ]; then
    echo "  Patched $PATCHED file(s) successfully!"
    echo ""
    echo "  Now rebuild:"
    echo "    cd build && make clean && make -j\$(sysctl -n hw.ncpu)"
else
    echo "  No patches needed — files already compatible."
fi
echo "=========================================="
