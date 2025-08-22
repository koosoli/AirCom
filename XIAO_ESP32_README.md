# AirCom System - XIAO ESP32 Series Port

This document describes the porting of the AirCom system to be compatible with Seeed Studio XIAO ESP32 series boards (ESP32S3, ESP32C3, ESP32C6) using MorseMicro MM-IoT-SDK for Wi-Fi HaLow networking.

## Overview

The AirCom system has been successfully ported to support XIAO ESP32 series boards with the following key changes:

### ✅ Completed Tasks

1. **PlatformIO Configuration**: Updated `platformio.ini` for XIAO ESP32 series compatibility
2. **MM-IoT-SDK Integration**: Created a new MM-IoT-SDK component for Wi-Fi HaLow networking
3. **Pin Configuration**: Implemented board-specific pin assignments for XIAO form factor
4. **Build System**: Updated CMakeLists.txt files for new dependencies
5. **Integration Testing**: Created comprehensive integration tests

### 🔧 Technical Changes

#### 1. Board Support
- **ESP32S3**: Full support with optimized pin configurations
- **ESP32C3**: Full support with RISC-V core optimizations
- **ESP32C6**: Full support with latest ESP32 features

#### 2. Networking Architecture
- **MM-IoT-SDK**: Replaced ESP-IDF HaLow with MorseMicro's professional SDK
- **SPI Interface**: Direct SPI communication with FGH100M-H module
- **Mesh Networking**: Enhanced mesh networking capabilities
- **Event-Driven**: Asynchronous event handling for network operations

#### 3. Hardware Configuration
- **Pin Management**: Board-specific pin assignments for all XIAO variants
- **SPI Configuration**: Optimized SPI settings for FGH100M-H module
- **Power Management**: Battery monitoring support
- **LED/Button**: Onboard LED and button integration

## Building the Project

### Prerequisites

1. **PlatformIO**: Install PlatformIO Core or PlatformIO IDE
2. **XIAO ESP32 Board**: Any of ESP32S3, ESP32C3, or ESP32C6 variant
3. **FGH100M-H Module**: Wi-Fi HaLow module from MorseMicro
4. **USB Cable**: For programming and serial communication

### Build Instructions

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd AirCom
   ```

2. **Select target board**:
   Edit `platformio.ini` and uncomment the desired environment:
   ```ini
   [env:xiao_esp32s3]  # For XIAO ESP32S3
   # [env:xiao_esp32c3]  # For XIAO ESP32C3
   # [env:xiao_esp32c6]  # For XIAO ESP32C6
   ```

3. **Build the project**:
   ```bash
   platformio run
   ```

4. **Upload to board**:
   ```bash
   platformio run --target upload
   ```

5. **Monitor serial output**:
   ```bash
   platformio device monitor
   ```

### Alternative Build Commands

For specific board targets:
```bash
# Build for XIAO ESP32S3
platformio run -e xiao_esp32s3

# Build for XIAO ESP32C3
platformio run -e xiao_esp32c3

# Build for XIAO ESP32C6
platformio run -e xiao_esp32c6
```

## Hardware Setup

### FGH100M-H Module Connection

| FGH100M-H Pin | XIAO ESP32 Pin | Description |
|---------------|----------------|-------------|
| MOSI | 35 (S3), 6 (C3), 18 (C6) | SPI Master Out Slave In |
| MISO | 37 (S3), 7 (C3), 19 (C6) | SPI Master In Slave Out |
| SCLK | 36 (S3), 8 (C3), 20 (C6) | SPI Clock |
| CS | 34 (S3), 9 (C3), 21 (C6) | SPI Chip Select |
| RESET | 33 (S3), 10 (C3), 22 (C6) | Module Reset |
| INT | 38 (S3), 5 (C3), 17 (C6) | Interrupt |

### Additional Connections

| Function | XIAO ESP32 Pin | Description |
|----------|----------------|-------------|
| LED | 21 (S3), 21 (C3), 15 (C6) | Status LED |
| Button | 0 (S3), 9 (C3), 9 (C6) | User Button |
| Battery ADC | 4 (S3), 2 (C3), 10 (C6) | Battery Voltage |

## Software Architecture

### New Components

#### 1. MM-IoT-SDK Component (`components/MM-IoT-SDK/`)
- **Purpose**: C++ wrapper for MorseMicro MM-IoT-SDK
- **Features**:
  - Singleton pattern for SDK management
  - Event-driven architecture
  - Board-specific pin configurations
  - SPI communication with FGH100M-H module

#### 2. XIAO Configuration (`main/include/xiao_esp32_config.h`)
- **Purpose**: Board-specific configuration management
- **Features**:
  - Pin assignments for all XIAO variants
  - SPI configuration constants
  - Build-time board detection

#### 3. Integration Tests (`main/xiao_integration_test.cpp`)
- **Purpose**: Verify XIAO ESP32 integration
- **Features**:
  - Board configuration validation
  - MM-IoT-SDK functionality tests
  - SPI communication verification

### Updated Components

#### 1. HaLowMeshManager
- **Integration**: Now uses MM-IoT-SDK instead of ESP-IDF HaLow
- **Features**:
  - Enhanced mesh networking
  - Better error handling
  - Event-based connection management

#### 2. PlatformIO Configuration
- **Multi-board support**: Separate environments for each XIAO variant
- **Optimized settings**: Board-specific upload speeds and configurations

## API Changes

### MM-IoT-SDK Interface

```cpp
class MMIoTSDK {
public:
    static MMIoTSDK& getInstance();
    bool initialize(const std::string& ssid, const std::string& password, const std::string& country_code = "00");
    bool startDiscovery();
    bool connectToPeer(const std::string& peer_id);
    bool sendData(const std::string& peer_id, const std::vector<uint8_t>& data);
    bool broadcastData(const std::vector<uint8_t>& data);
    // ... more methods
};
```

### Board Configuration

```cpp
// Get board-specific pins at runtime
int mosi_pin = get_spi_mosi_pin();
int miso_pin = get_spi_miso_pin();
// ... etc
```

## Testing

### Integration Tests

Run the integration tests to verify the porting:

```cpp
// In main.cpp or a test task
bool tests_passed = run_xiao_integration_tests();
if (tests_passed) {
    ESP_LOGI(TAG, "All integration tests passed!");
} else {
    ESP_LOGE(TAG, "Some integration tests failed!");
}
```

### Manual Testing

1. **Board Detection**: Check serial output for correct board type
2. **Pin Configuration**: Verify SPI and GPIO pin assignments
3. **Network Discovery**: Test peer discovery functionality
4. **Data Transmission**: Test sending/receiving data
5. **Mesh Networking**: Verify mesh network formation

## Troubleshooting

### Common Issues

#### 1. Build Errors
- **Problem**: Missing dependencies
- **Solution**: Ensure all components are properly configured in CMakeLists.txt

#### 2. SPI Communication Issues
- **Problem**: FGH100M-H module not responding
- **Solution**: Check SPI pin connections and power supply

#### 3. Network Connection Issues
- **Problem**: Unable to discover peers
- **Solution**: Verify antenna connections and Wi-Fi HaLow configuration

#### 4. Board Detection Issues
- **Problem**: Incorrect board type detected
- **Solution**: Check build flags in platformio.ini

### Debug Information

Enable debug logging for detailed information:

```cpp
// In platformio.ini
build_flags =
    -DCORE_DEBUG_LEVEL=5
    -DCONFIG_MORSEMICRO_HALOW_DEBUG=1
```

## Performance Optimizations

### ESP32C3 Specific
- **RISC-V Core**: Optimized instruction set for better performance
- **Low Power**: Reduced power consumption for battery-powered applications

### ESP32C6 Specific
- **Latest Features**: Support for latest ESP32 enhancements
- **Improved Wi-Fi**: Better Wi-Fi HaLow performance

### ESP32S3 Specific
- **Dual Core**: Enhanced multitasking capabilities
- **Large Memory**: Support for larger applications

## Migration Guide

### From ESP-IDF HaLow

1. **Update includes**: Replace `HaLowMeshManager.h` with `mm_iot_sdk.h`
2. **Update initialization**: Use `MMIoTSDK::getInstance().initialize()`
3. **Update networking calls**: Use MM-IoT-SDK methods instead of ESP-IDF calls
4. **Update pin assignments**: Use board-specific pin functions

### Code Example

```cpp
// Old code (ESP-IDF HaLow)
HaLowMeshManager meshManager;
meshManager.begin();

// New code (MM-IoT-SDK)
auto& mmSDK = MMIoTSDK::getInstance();
mmSDK.initialize("AirCom-HaLow", "password");
```

## Future Enhancements

### Planned Features
- **OTA Updates**: Over-the-air firmware updates
- **Power Optimization**: Advanced power management
- **Security Enhancements**: Additional encryption options
- **Performance Monitoring**: Real-time performance metrics

### Board Support Expansion
- **ESP32C2**: Support for ultra-low power applications
- **ESP32H2**: Support for Matter protocol integration

## Support

For issues related to the XIAO ESP32 port:

1. **Check the integration tests**: Run `run_xiao_integration_tests()`
2. **Verify hardware connections**: Double-check FGH100M-H module connections
3. **Review serial output**: Enable debug logging for detailed information
4. **Check board compatibility**: Ensure using supported XIAO ESP32 variant

## License

This port maintains the same license as the original AirCom project.

## Contributors

- **AirCom Development Team**: Original system architecture
- **XIAO ESP32 Port Team**: Board-specific adaptations and MM-IoT-SDK integration

---

**Note**: This document is specific to the XIAO ESP32 series port. For general AirCom system documentation, refer to the main README.md file.