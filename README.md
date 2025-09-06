# ESPOTAUpdater Library

A comprehensive ESP32 Over-The-Air (OTA) update library that integrates with GitHub releases for seamless firmware distribution and updates.

## Features

- 🚀 **GitHub Integration**: Automatically fetch updates from GitHub releases
- 🎯 **Multi-Board Support**: Board-specific firmware binaries
- 📊 **Progress Callbacks**: Real-time update progress monitoring
- 🔄 **Auto-Update**: Configurable automatic updates
- 🛡️ **Safe Updates**: Validation and rollback protection
- 📱 **Version Management**: Semantic version parsing and comparison
- ⚙️ **Configurable**: Customizable update intervals and behaviors

## Quick Start

### Installation

#### PlatformIO
Add to your `platformio.ini`:
```ini
lib_deps = 
    https://github.com/stevennolte/ESPOTAUpdater.git
```

#### Arduino IDE
1. Download or clone this repository
2. Copy to your Arduino libraries folder
3. Restart Arduino IDE

### Basic Usage

```cpp
#include <ESPOTAUpdater.h>

// Initialize with your GitHub repository and current firmware version
ESPOTAUpdater otaUpdater("username/repository", 100); // version 1.00

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi first
    // ... your WiFi connection code ...
    
    // Set board type (optional - auto-detected from build flags)
    otaUpdater.setBoardType("ESP32_DEVKIT");
    
    // Check for updates
    otaUpdater.checkForUpdates();
}

void loop() {
    // Check for updates every 5 minutes (default)
    if (otaUpdater.shouldCheckForUpdates(millis())) {
        otaUpdater.checkForUpdates();
        otaUpdater.updateLastCheckTime(millis());
    }
    
    delay(1000);
}
```

### Advanced Usage with Callbacks

```cpp
#include <ESPOTAUpdater.h>

ESPOTAUpdater otaUpdater("username/repository", 100);

void onUpdateAvailable(int currentVersion, int newVersion, const String& downloadUrl) {
    Serial.printf("Update available: v%d -> v%d\n", currentVersion, newVersion);
    Serial.println("Download URL: " + downloadUrl);
    
    // Custom logic - ask user, check conditions, etc.
    // The library will auto-update if enabled, or you can call:
    // otaUpdater.performUpdate(downloadUrl.c_str());
}

void onUpdateProgress(size_t progress, size_t total) {
    float percentage = (float)progress / total * 100;
    Serial.printf("Update progress: %.1f%% (%zu/%zu bytes)\n", percentage, progress, total);
}

void onUpdateComplete(bool success, const String& message) {
    if (success) {
        Serial.println("Update completed successfully!");
    } else {
        Serial.println("Update failed: " + message);
    }
}

void setup() {
    Serial.begin(115200);
    
    // Set up callbacks
    otaUpdater.setUpdateAvailableCallback(onUpdateAvailable);
    otaUpdater.setUpdateProgressCallback(onUpdateProgress);
    otaUpdater.setUpdateCompleteCallback(onUpdateComplete);
    
    // Configure update behavior
    otaUpdater.enableAutoUpdate(true);
    otaUpdater.setUpdateInterval(10 * 60 * 1000); // Check every 10 minutes
    
    // Connect to WiFi
    // ... your WiFi connection code ...
    
    otaUpdater.checkForUpdates();
}
```

## GitHub Release Setup

### Firmware Naming Convention

For multi-board support, name your firmware binaries according to your board type:

- `firmware-esp32-devkit.bin` - For ESP32 DevKit boards
- `firmware-xiao-esp32s3.bin` - For Seeed Xiao ESP32S3
- `firmware-esp32s3-devkitc.bin` - For ESP32-S3 DevKit C
- `firmware.bin` - Generic fallback firmware

### Version Tagging

Use semantic versioning for your GitHub release tags:
- `v1.0` - Major.Minor format
- `v1.23` - Supports multi-digit minor versions
- `v2.1` - Will be parsed as version 201

### Build Configuration

In your `platformio.ini`, set the board type for automatic detection:

```ini
[env:esp32doit-devkit-v1]
board = esp32doit-devkit-v1
build_flags = 
    -DBOARD_TYPE=\"ESP32_DEVKIT\"

[env:seeed_xiao_esp32s3]
board = seeed_xiao_esp32s3
build_flags = 
    -DBOARD_TYPE=\"XIAO_ESP32S3\"
```

## API Reference

### Constructor

```cpp
ESPOTAUpdater(const char* githubRepo, int currentFirmwareVersion)
```
- `githubRepo`: GitHub repository in format "username/repository"
- `currentFirmwareVersion`: Current firmware version as integer (e.g., 100 for v1.00)

### Core Methods

#### `void checkForUpdates()`
Check GitHub releases for newer firmware versions.

#### `void performUpdate(const char* url)`
Download and install firmware from the specified URL.

#### `bool shouldCheckForUpdates(unsigned long currentTime)`
Returns true if enough time has passed since the last update check.

#### `void updateLastCheckTime(unsigned long currentTime)`
Update the timestamp of the last update check.

### Configuration Methods

#### `void setBoardType(const String& boardType)`
Manually set the board type for firmware selection.

#### `void setUpdateInterval(unsigned long intervalMs)`
Set the interval between automatic update checks (default: 5 minutes).

#### `void enableAutoUpdate(bool enabled)`
Enable or disable automatic updates when newer firmware is found.

### Callback Methods

#### `void setUpdateAvailableCallback(UpdateAvailableCallback callback)`
Set callback function called when an update is available.

#### `void setUpdateProgressCallback(UpdateProgressCallback callback)`
Set callback function called during update progress.

#### `void setUpdateCompleteCallback(UpdateCompleteCallback callback)`
Set callback function called when update completes (success or failure).

### Callback Function Signatures

```cpp
typedef void (*UpdateAvailableCallback)(int currentVersion, int newVersion, const String& downloadUrl);
typedef void (*UpdateProgressCallback)(size_t progress, size_t total);
typedef void (*UpdateCompleteCallback)(bool success, const String& message);
```

## Supported Boards

The library automatically detects board types from compile-time definitions:

| Board Type | Build Flag | Firmware Name |
|------------|------------|---------------|
| ESP32 DevKit | `BOARD_TYPE="ESP32_DEVKIT"` | `firmware-esp32-devkit.bin` |
| Xiao ESP32S3 | `BOARD_TYPE="XIAO_ESP32S3"` | `firmware-xiao-esp32s3.bin` |
| ESP32-S3 DevKit C | `BOARD_TYPE="ESP32_S3_DEVKITC"` | `firmware-esp32s3-devkitc.bin` |
| Generic/Unknown | (any other) | `firmware.bin` |

## Requirements

### Software Dependencies
- **ESP32 Arduino Core** 2.0.0 or newer
- **ArduinoJson** 6.0.0 or newer
- **WiFi connection** (library does not handle WiFi setup)

### Hardware Requirements
- ESP32-based microcontroller
- Sufficient flash memory for firmware updates
- Stable internet connection

## Error Handling

The library includes comprehensive error handling:

- **Network Errors**: HTTP timeouts and connection failures
- **JSON Parsing**: Invalid GitHub API responses
- **Version Parsing**: Malformed version tags
- **Update Errors**: Insufficient space, write failures, validation errors

Check the Serial output for detailed error messages and debugging information.

## Security Considerations

- **HTTPS**: All GitHub API calls use HTTPS
- **Validation**: Firmware size and integrity checks
- **Rollback**: Failed updates don't brick the device
- **User Control**: Callbacks allow custom validation logic

## Troubleshooting

### Common Issues

**"No firmware binary found"**
- Ensure your GitHub release includes properly named firmware files
- Check that the board type detection is working correctly

**"Update failed"**
- Verify sufficient flash memory is available
- Check internet connection stability
- Ensure GitHub release assets are publicly accessible

**"Version parsing failed"**
- Use semantic versioning format: `v1.0`, `v2.15`, etc.
- Avoid non-numeric characters in version tags

### Debug Information

Enable Serial output at 115200 baud to see detailed debug information including:
- GitHub API responses
- Version parsing details
- Update progress
- Error messages

## Examples

See the `examples/` directory for complete working examples:

- **BasicOTA**: Simple OTA update implementation
- **AdvancedOTA**: Full-featured example with callbacks and configuration
- **ConditionalUpdate**: Update only under specific conditions

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

### Development Setup

1. Clone the repository
2. Install PlatformIO
3. Run examples: `pio run -e example_env`
4. Run tests: `pio test`

## License

MIT License - see LICENSE file for details.

## Changelog

### v1.0.0
- Initial release
- GitHub releases integration
- Multi-board support
- Progress callbacks
- Automatic update checking

---

**ESPOTAUpdater** - Keeping your ESP32 devices up to date, automatically! 🚀
