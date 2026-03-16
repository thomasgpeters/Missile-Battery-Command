#!/bin/bash
# ============================================================================
# Patch cocos2d-x for modern macOS/Xcode compatibility (AppleClang 17+)
#
# Fixes:
#   1. iconv_t type mismatch in CCFontAtlas (void* -> iconv_t)
#   2. AudioCache missing _loadCallbacks/_playCallbacks members
#   3. AudioCache missing addLoadCallback declaration
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

# ==========================================================================
# Fix 1: CCFontAtlas.h — change _iconv member from void* to iconv_t
# ==========================================================================
FONT_ATLAS_H="$COCOS_DIR/cocos/2d/CCFontAtlas.h"
if [ -f "$FONT_ATLAS_H" ]; then
    echo "[CHECK] CCFontAtlas.h"

    # Add #include <iconv.h> if not present
    if ! grep -q '#include <iconv.h>' "$FONT_ATLAS_H"; then
        echo "[PATCH]   Adding #include <iconv.h>"
        sed -i '' '1,/#include/{
            /^#include/a\
#include <iconv.h>
        }' "$FONT_ATLAS_H"
        PATCHED=$((PATCHED + 1))
    fi

    # Replace void* _iconv with iconv_t _iconv
    if grep -q '_iconv' "$FONT_ATLAS_H"; then
        perl -pi -e 's/void\s*\*\s*_iconv/iconv_t _iconv/g' "$FONT_ATLAS_H"
        echo "[PATCH]   void* _iconv -> iconv_t _iconv"
        PATCHED=$((PATCHED + 1))
    fi
else
    echo "[SKIP]  CCFontAtlas.h not found"
fi

# ==========================================================================
# Fix 2: CCFontAtlas.cpp — remove void* casts around iconv calls
# ==========================================================================
FONT_ATLAS_CPP="$COCOS_DIR/cocos/2d/CCFontAtlas.cpp"
if [ -f "$FONT_ATLAS_CPP" ]; then
    echo "[CHECK] CCFontAtlas.cpp"
    perl -pi -e 's/\(void\s*\*\)\s*iconv_open/iconv_open/g' "$FONT_ATLAS_CPP"
    perl -pi -e 's/\(iconv_t\)\s*_iconv/_iconv/g' "$FONT_ATLAS_CPP"
    echo "[PATCH]   Cleaned up iconv casts"
    PATCHED=$((PATCHED + 1))
else
    echo "[SKIP]  CCFontAtlas.cpp not found"
fi

# ==========================================================================
# Fix 3: CCFontFreeType.cpp — same iconv casts
# ==========================================================================
FONT_FT_CPP="$COCOS_DIR/cocos/2d/CCFontFreeType.cpp"
if [ -f "$FONT_FT_CPP" ] && grep -q 'iconv' "$FONT_FT_CPP" 2>/dev/null; then
    echo "[CHECK] CCFontFreeType.cpp"
    perl -pi -e 's/\(void\s*\*\)\s*iconv_open/iconv_open/g' "$FONT_FT_CPP"
    perl -pi -e 's/\(iconv_t\)\s*_iconv/_iconv/g' "$FONT_FT_CPP"
    echo "[PATCH]   Cleaned up iconv casts"
    PATCHED=$((PATCHED + 1))
fi

# ==========================================================================
# Fix 4: AudioCache.h — add missing _loadCallbacks, _playCallbacks members
#         and addLoadCallback declaration
# ==========================================================================
AUDIO_CACHE_H="$COCOS_DIR/cocos/audio/include/AudioCache.h"
# Also check alternate location
if [ ! -f "$AUDIO_CACHE_H" ]; then
    AUDIO_CACHE_H="$COCOS_DIR/cocos/audio/AudioCache.h"
fi

if [ -f "$AUDIO_CACHE_H" ]; then
    echo "[CHECK] AudioCache.h"

    # Add _loadCallbacks if missing
    if ! grep -q '_loadCallbacks' "$AUDIO_CACHE_H"; then
        echo "[PATCH]   Adding _loadCallbacks member"
        # Insert before the closing }; of the class
        # Find _callbacks or any existing std::vector/std::function member, or the last private: section
        perl -0777 -pi -e '
            # Add members before the final closing brace of the class
            s/((\s*)(std::mutex\s+_callbackMutex;))/$1\n$2std::vector<std::function<void(bool)>> _loadCallbacks;\n$2std::vector<std::function<void()>> _playCallbacks;/s
                or
            # Fallback: add before closing brace
            s/(\n\};)(\s*\n\s*#endif)/\n    std::vector<std::function<void(bool)>> _loadCallbacks;\n    std::vector<std::function<void()>> _playCallbacks;$1$2/s;
        ' "$AUDIO_CACHE_H"
        PATCHED=$((PATCHED + 1))
    else
        echo "[OK]      _loadCallbacks already present"
    fi

    # Add _playCallbacks if missing (might have been added above)
    if ! grep -q '_playCallbacks' "$AUDIO_CACHE_H"; then
        echo "[PATCH]   Adding _playCallbacks member"
        perl -pi -e 's/(_loadCallbacks;)/$1\n    std::vector<std::function<void()>> _playCallbacks;/' "$AUDIO_CACHE_H"
        PATCHED=$((PATCHED + 1))
    fi

    # Add addLoadCallback declaration if missing
    if ! grep -q 'addLoadCallback' "$AUDIO_CACHE_H"; then
        echo "[PATCH]   Adding addLoadCallback declaration"
        # Add after addPlayCallback or any existing public method
        if grep -q 'addPlayCallback' "$AUDIO_CACHE_H"; then
            perl -pi -e 's/(void addPlayCallback.*?;)/$1\n    void addLoadCallback(const std::function<void(bool)>\& callback);/' "$AUDIO_CACHE_H"
        else
            # Add before first private: or protected:
            perl -pi -e 's/(private:|protected:)/void addLoadCallback(const std::function<void(bool)>\& callback);\n\n    $1/' "$AUDIO_CACHE_H"
        fi
        PATCHED=$((PATCHED + 1))
    else
        echo "[OK]      addLoadCallback already declared"
    fi

    echo "  Verify:"
    grep -n '_loadCallbacks\|_playCallbacks\|addLoadCallback' "$AUDIO_CACHE_H" 2>/dev/null || echo "  WARNING: members not found after patch"
else
    echo "[SKIP]  AudioCache.h not found at expected locations"
    echo "        Searching..."
    find "$COCOS_DIR" -name "AudioCache.h" -type f 2>/dev/null
fi

# ==========================================================================
# Cleanup .bak files
# ==========================================================================
find "$COCOS_DIR/cocos/" -name "*.bak" -delete 2>/dev/null

echo ""
echo "=========================================="
echo "  Patched $PATCHED item(s)."
echo ""
echo "  Rebuild:"
echo "    rm -rf build && mkdir build && cd build"
echo "    cmake .. && make -j\$(sysctl -n hw.ncpu)"
echo "=========================================="
