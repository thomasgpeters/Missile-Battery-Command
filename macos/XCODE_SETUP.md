# Xcode Setup for Self-Contained App Bundle

## Quick Start

```bash
# Generate Xcode project (auto-detects cocos2d-x)
./macos/generate_xcode.sh

# Or specify cocos2d-x path explicitly
./macos/generate_xcode.sh /path/to/cocos2d-x

# Open in Xcode
open build-xcode/MissileBatteryCommand.xcodeproj
```

## Xcode Build Settings Configuration

After opening the project in Xcode, verify these settings on the
**MissileBatteryCommand** target:

### General Tab
- **Deployment Target**: macOS 10.13+
- **Category**: Games

### Build Settings Tab

| Setting | Value |
|---------|-------|
| **Runpath Search Paths** | `@executable_path/../Frameworks` |
| **Always Embed Swift Standard Libraries** | No (pure C++ project) |
| **Dead Code Stripping** | Yes |
| **Enable Hardened Runtime** | Yes (for notarization) |
| **Code Signing Identity** | Your Developer ID (for distribution) |

### Build Phases Tab

1. **Copy Files Phase** (for dynamic libraries):
   - Destination: Frameworks
   - Add any `.dylib` or `.framework` files from cocos2d-x if using dynamic linking

2. **Copy Bundle Resources**:
   - Ensure the `Resources/` directory is included

## Static vs Dynamic Linking

### Static Linking (Recommended)
The CMake configuration defaults to static linking (`BUILD_SHARED_LIBS=OFF`).
This produces a single executable with all cocos2d-x code compiled in.
No Frameworks directory or dylib copying needed.

### Dynamic Linking
If you need dynamic linking, the CMake config automatically:
- Sets `@executable_path/../Frameworks` as the runpath
- Creates a Frameworks directory in the bundle
- Copies cocos2d-x dylibs into the bundle

## Building for Distribution

### Archive for Distribution
1. In Xcode: Product > Archive
2. In the Organizer: Distribute App
3. Choose "Developer ID" for direct distribution, or "App Store" for Mac App Store

### Command Line Build
```bash
# Debug build
cmake --build build-xcode --config Debug

# Release build
cmake --build build-xcode --config Release

# The .app bundle location:
# build-xcode/Release/MissileBatteryCommand.app
```

### Code Signing (for distribution outside App Store)
```bash
codesign --deep --force --options runtime \
    --sign "Developer ID Application: Your Name (TEAM_ID)" \
    build-xcode/Release/MissileBatteryCommand.app
```

### Notarization (required for macOS 10.15+)
```bash
# Create a zip for notarization
ditto -c -k --keepParent \
    build-xcode/Release/MissileBatteryCommand.app \
    MissileBatteryCommand.zip

# Submit for notarization
xcrun notarytool submit MissileBatteryCommand.zip \
    --apple-id "your@email.com" \
    --team-id "TEAM_ID" \
    --password "app-specific-password" \
    --wait

# Staple the ticket
xcrun stapler staple build-xcode/Release/MissileBatteryCommand.app
```

## App Bundle Structure

After building, the self-contained `.app` bundle looks like:

```
MissileBatteryCommand.app/
  Contents/
    Info.plist              (app metadata)
    MacOS/
      MissileBatteryCommand (executable, statically linked)
    Resources/              (game assets)
    Frameworks/             (only if using dynamic linking)
      libcocos2d.dylib
      ...
```

Users simply download the `.app` and double-click to run. No dependencies needed.
