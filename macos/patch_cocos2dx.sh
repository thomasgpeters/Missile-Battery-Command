#!/bin/bash
# ============================================================================
# Patch cocos2d-x for modern macOS/Xcode compatibility
#
# Fixes iconv_t type mismatch in CCFontAtlas (Xcode 15+/macOS 14+ SDK)
# where iconv_t changed from void* to __tag_iconv_t*
#
# Usage:
#   ./macos/patch_cocos2dx.sh [/path/to/cocos2d-x]
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
# Fix CCFontAtlas.h — change _iconv member from void* to iconv_t
# --------------------------------------------------------------------------
FONT_ATLAS_H="$COCOS_DIR/cocos/2d/CCFontAtlas.h"
if [ -f "$FONT_ATLAS_H" ]; then
    echo "[CHECK] CCFontAtlas.h"
    echo "  Before:"
    grep -n 'iconv\|void.*_iconv' "$FONT_ATLAS_H" 2>/dev/null || echo "  (no iconv references found)"
    echo ""

    # Add #include <iconv.h> if not present
    if ! grep -q '#include <iconv.h>' "$FONT_ATLAS_H"; then
        echo "[PATCH] Adding #include <iconv.h>"
        # Insert after the first #include block
        sed -i '' '1,/#include/{
            /^#include/a\
#include <iconv.h>
        }' "$FONT_ATLAS_H"
        PATCHED=$((PATCHED + 1))
    fi

    # Replace ANY variation of void* _iconv declaration
    if grep -q 'void' "$FONT_ATLAS_H" && grep -q '_iconv' "$FONT_ATLAS_H"; then
        # Use perl for more reliable cross-platform regex
        perl -pi -e 's/void\s*\*\s*_iconv/iconv_t _iconv/g' "$FONT_ATLAS_H"
        echo "[PATCH] Changed void* _iconv -> iconv_t _iconv"
        PATCHED=$((PATCHED + 1))
    fi

    echo "  After:"
    grep -n 'iconv\|_iconv' "$FONT_ATLAS_H" 2>/dev/null || echo "  (no matches)"
    echo ""
else
    echo "[SKIP]  CCFontAtlas.h not found"
fi

# --------------------------------------------------------------------------
# Fix CCFontAtlas.cpp — fix all iconv-related void* casts
# --------------------------------------------------------------------------
FONT_ATLAS_CPP="$COCOS_DIR/cocos/2d/CCFontAtlas.cpp"
if [ -f "$FONT_ATLAS_CPP" ]; then
    echo "[CHECK] CCFontAtlas.cpp"
    echo "  iconv-related lines before patch:"
    grep -n 'iconv' "$FONT_ATLAS_CPP" 2>/dev/null || echo "  (none)"
    echo ""

    # Remove (void*) cast from iconv_open
    perl -pi -e 's/\(void\s*\*\)\s*iconv_open/iconv_open/g' "$FONT_ATLAS_CPP"

    # Remove (iconv_t) cast from _iconv usage (no longer needed)
    perl -pi -e 's/\(iconv_t\)\s*_iconv/_iconv/g' "$FONT_ATLAS_CPP"

    echo "[PATCH] CCFontAtlas.cpp: Cleaned up iconv casts"
    PATCHED=$((PATCHED + 1))

    echo "  iconv-related lines after patch:"
    grep -n 'iconv' "$FONT_ATLAS_CPP" 2>/dev/null || echo "  (none)"
    echo ""
else
    echo "[SKIP]  CCFontAtlas.cpp not found"
fi

# --------------------------------------------------------------------------
# Fix CCFontFreeType.cpp — same iconv issue if present
# --------------------------------------------------------------------------
FONT_FT_CPP="$COCOS_DIR/cocos/2d/CCFontFreeType.cpp"
if [ -f "$FONT_FT_CPP" ]; then
    if grep -q 'iconv' "$FONT_FT_CPP" 2>/dev/null; then
        echo "[CHECK] CCFontFreeType.cpp"
        perl -pi -e 's/\(void\s*\*\)\s*iconv_open/iconv_open/g' "$FONT_FT_CPP"
        perl -pi -e 's/\(iconv_t\)\s*_iconv/_iconv/g' "$FONT_FT_CPP"
        echo "[PATCH] CCFontFreeType.cpp: Cleaned up iconv casts"
        PATCHED=$((PATCHED + 1))
    fi
fi

# --------------------------------------------------------------------------
# Cleanup backup files
# --------------------------------------------------------------------------
find "$COCOS_DIR/cocos/2d/" -name "*.bak" -delete 2>/dev/null

echo ""
echo "=========================================="
echo "  Patched $PATCHED item(s)."
echo ""
echo "  VERIFY the header was patched correctly:"
echo "    grep '_iconv' $FONT_ATLAS_H"
echo ""
echo "  Should show:  iconv_t _iconv"
echo "  NOT:          void *_iconv"
echo ""
echo "  Then rebuild:"
echo "    cd build && make clean && make -j\$(sysctl -n hw.ncpu)"
echo "=========================================="
