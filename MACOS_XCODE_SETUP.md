# macOS Xcode Setup — Self-Contained App Bundle

## Overview

This project is configured to build a fully self-contained macOS `.app` bundle.
All cocos2d-x and dependent libraries are packaged inside the bundle so users
can download and run the game without installing any dependencies.

## Quick Start

```bash
# Generate Xcode project (auto-detects cocos2d-x)
./macos/generate_xcode.sh

# Or specify cocos2d-x path explicitly
./macos/generate_xcode.sh /path/to/cocos2d-x

# Open in Xcode
open build-xcode/MissileBatteryCommand.xcodeproj
```

## How Self-Contained Bundling Works

### CMakeLists.txt Configuration

When building on macOS with cocos2d-x found, the CMake build automatically:

1. **Creates a MACOSX_BUNDLE** — `add_executable(... MACOSX_BUNDLE ...)`
2. **Sets bundle metadata** via `Info.plist.in`:
   - Bundle ID: `com.imagerybusinesssystems.missilebatterycommand`
   - Version: 1.0.0
   - Min macOS: 10.13
   - Category: Games
   - High-DPI support enabled
3. **Configures runpath** for embedded frameworks:
   - `BUILD_RPATH = @executable_path/../Frameworks`
   - `INSTALL_RPATH = @executable_path/../Frameworks`
4. **Copies Resources** into `Contents/Resources/` inside the bundle
5. **Creates Frameworks directory** and embeds any cocos2d-x dylibs (if dynamic)

### Static Linking (Default — Recommended)

The `generate_xcode.sh` script passes `-DBUILD_SHARED_LIBS=OFF` by default.
This statically links cocos2d-x and all its dependencies (zlib, libpng, libjpeg,
freetype, glfw, etc.) directly into the executable.

Result: A single binary with zero external dependencies.

### Dynamic Linking (Optional)

If you set `-DBUILD_SHARED_LIBS=ON`, the CMake config automatically:
- Sets `@executable_path/../Frameworks` as the runtime library search path
- Creates a `Frameworks/` directory inside the `.app` bundle
- Copies `libcocos2d.dylib` (and dependencies) into that directory
- Libraries are found at runtime via the embedded runpath

## Xcode Build Settings

After opening the generated `.xcodeproj`, verify these settings on the
**MissileBatteryCommand** target:

### General Tab

| Setting | Value |
|---------|-------|
| Deployment Target | macOS 10.13+ |
| Category | Games |

### Build Settings Tab

| Setting | Value |
|---------|-------|
| **C++ Language Dialect** | C++17 (`-std=c++17`) |
| **Runpath Search Paths** | `@executable_path/../Frameworks` |
| **Dead Code Stripping** | Yes |
| **Enable Hardened Runtime** | Yes (required for notarization) |
| **Code Signing Identity** | Developer ID Application (for distribution) |
| **Product Bundle Identifier** | `com.imagerybusinesssystems.missilebatterycommand` |

### Build Phases Tab

1. **Compile Sources** — All `.cpp` files from `src/`
2. **Link Binary With Libraries** — `libcocos2d.a` (static) or `cocos2d` framework
3. **Copy Files** (Destination: Frameworks):
   - Only needed if using dynamic linking
   - Add any `.dylib` or `.framework` files from cocos2d-x
4. **Copy Bundle Resources**:
   - The `Resources/` directory with game assets

### Info.plist

The template at `macos/Info.plist.in` is used by CMake. Key entries:

| Key | Value |
|-----|-------|
| `CFBundleDisplayName` | Missile Battery Command |
| `CFBundleIdentifier` | com.imagerybusinesssystems.missilebatterycommand |
| `CFBundleVersion` | 1.0.0 |
| `LSMinimumSystemVersion` | 10.13 |
| `NSHighResolutionCapable` | YES |
| `LSApplicationCategoryType` | public.app-category.games |
| `NSHumanReadableCopyright` | © 2026 Imagery Business Systems, LLC |

## Building

### From Xcode

1. Select **MissileBatteryCommand** scheme
2. Choose **Product > Build** (Cmd+B) for debug
3. Choose **Product > Archive** for release distribution

### From Command Line

```bash
# Debug
cmake --build build-xcode --config Debug

# Release
cmake --build build-xcode --config Release

# Output location:
# build-xcode/Debug/MissileBatteryCommand.app
# build-xcode/Release/MissileBatteryCommand.app
```

### Without Xcode (Makefile)

```bash
mkdir -p build && cd build
cmake -DCOCOS2DX_ROOT=/path/to/cocos2d-x -DBUILD_SHARED_LIBS=OFF ..
make -j$(sysctl -n hw.ncpu)
```

## Distribution

### Code Signing

Required for distributing outside the App Store:

```bash
codesign --deep --force --options runtime \
    --sign "Developer ID Application: Your Name (TEAM_ID)" \
    build-xcode/Release/MissileBatteryCommand.app
```

### Notarization (required for macOS 10.15+)

```bash
# Create zip for upload
ditto -c -k --keepParent \
    build-xcode/Release/MissileBatteryCommand.app \
    MissileBatteryCommand.zip

# Submit for notarization
xcrun notarytool submit MissileBatteryCommand.zip \
    --apple-id "your@email.com" \
    --team-id "TEAM_ID" \
    --password "app-specific-password" \
    --wait

# Staple the notarization ticket to the app
xcrun stapler staple build-xcode/Release/MissileBatteryCommand.app
```

### Creating a DMG for Distribution

```bash
hdiutil create -volname "Missile Battery Command" \
    -srcfolder build-xcode/Release/MissileBatteryCommand.app \
    -ov -format UDZO \
    MissileBatteryCommand.dmg
```

## App Bundle Structure

```
MissileBatteryCommand.app/
└── Contents/
    ├── Info.plist                    (app metadata from Info.plist.in)
    ├── MacOS/
    │   └── MissileBatteryCommand    (executable — statically linked)
    ├── Resources/                   (game assets, textures, sounds)
    └── Frameworks/                  (only present with dynamic linking)
        ├── libcocos2d.dylib
        ├── libpng16.dylib
        ├── libfreetype.dylib
        └── ...
```

With static linking, the Frameworks directory is empty or absent — the single
executable contains everything. Users download the `.app` and double-click to run.

## Troubleshooting

### "Library not loaded" at runtime
The dynamic library path wasn't rewritten. Fix with:
```bash
install_name_tool -change /usr/local/lib/libcocos2d.dylib \
    @executable_path/../Frameworks/libcocos2d.dylib \
    MissileBatteryCommand.app/Contents/MacOS/MissileBatteryCommand
```
Or switch to static linking to avoid this entirely.

### "App is damaged and can't be opened"
The app needs code signing. See the Code Signing section above.

### Xcode can't find cocos2d-x headers
Re-run the generator with the correct path:
```bash
./macos/generate_xcode.sh /path/to/cocos2d-x
```
