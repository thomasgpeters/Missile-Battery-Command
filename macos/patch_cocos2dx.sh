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
# Search all possible locations for the apple AudioCache.h
AUDIO_CACHE_H=""
for CANDIDATE in \
    "$COCOS_DIR/cocos/audio/apple/AudioCache.h" \
    "$COCOS_DIR/cocos/audio/include/AudioCache.h" \
    "$COCOS_DIR/cocos/audio/AudioCache.h"; do
    if [ -f "$CANDIDATE" ]; then
        AUDIO_CACHE_H="$CANDIDATE"
        break
    fi
done

# Last resort: find it
if [ -z "$AUDIO_CACHE_H" ]; then
    AUDIO_CACHE_H=$(find "$COCOS_DIR" -path "*/audio/apple/AudioCache.h" -type f 2>/dev/null | head -1)
fi

if [ -f "$AUDIO_CACHE_H" ]; then
    echo "[CHECK] AudioCache.h ($AUDIO_CACHE_H)"

    # Add _loadCallbacks if missing — append before final }; of class
    if ! grep -q '_loadCallbacks' "$AUDIO_CACHE_H"; then
        echo "[PATCH]   Adding _loadCallbacks and _playCallbacks members"
        # Use a simple approach: find the last }; and insert before it
        perl -0777 -pi -e '
            s/(\n\s*\}\s*;?\s*\n(?:#endif|\n|$))/\n    std::vector<std::function<void(bool)>> _loadCallbacks;\n    std::vector<std::function<void()>> _playCallbacks;\n$1/s;
        ' "$AUDIO_CACHE_H"

        # Verify it was added
        if grep -q '_loadCallbacks' "$AUDIO_CACHE_H"; then
            echo "          -> Success"
        else
            # Absolute fallback: just append before the last line containing };
            echo "          -> Perl failed, trying sed fallback"
            # Find the last }; line number and insert before it
            LAST_BRACE=$(grep -n '};' "$AUDIO_CACHE_H" | tail -1 | cut -d: -f1)
            if [ -n "$LAST_BRACE" ]; then
                sed -i '' "${LAST_BRACE}i\\
    std::vector<std::function<void(bool)>> _loadCallbacks;\\
    std::vector<std::function<void()>> _playCallbacks;
" "$AUDIO_CACHE_H"
                echo "          -> Inserted at line $LAST_BRACE"
            fi
        fi
        PATCHED=$((PATCHED + 1))
    else
        echo "[OK]      _loadCallbacks already present"
    fi

    # Add addLoadCallback declaration if missing
    if ! grep -q 'addLoadCallback' "$AUDIO_CACHE_H"; then
        echo "[PATCH]   Adding addLoadCallback declaration"
        if grep -q 'addPlayCallback' "$AUDIO_CACHE_H"; then
            # Insert after addPlayCallback line
            sed -i '' '/addPlayCallback/a\
    void addLoadCallback(const std::function<void(bool)>\& callback);
' "$AUDIO_CACHE_H"
        else
            # Insert before the _loadCallbacks member we just added
            sed -i '' '/_loadCallbacks/i\
    void addLoadCallback(const std::function<void(bool)>\& callback);
' "$AUDIO_CACHE_H"
        fi
        PATCHED=$((PATCHED + 1))
    else
        echo "[OK]      addLoadCallback already declared"
    fi

    # Ensure <functional> and <vector> are included
    if ! grep -q '#include <functional>' "$AUDIO_CACHE_H"; then
        sed -i '' '1,/#include/{
            /^#include/a\
#include <functional>\
#include <vector>
        }' "$AUDIO_CACHE_H"
        echo "[PATCH]   Added #include <functional> and <vector>"
        PATCHED=$((PATCHED + 1))
    fi

    echo "  Verify:"
    grep -n '_loadCallbacks\|_playCallbacks\|addLoadCallback' "$AUDIO_CACHE_H" 2>/dev/null || echo "  WARNING: members not found after patch"
else
    echo "[SKIP]  AudioCache.h not found anywhere"
    echo "        Searched: $COCOS_DIR/cocos/audio/"
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
