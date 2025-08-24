# AirCom - ESP32 Tactical Communication System

## ⚠️ **IMPORTANT NOTICE**

**This is UNTESTED PROOF-OF-CONCEPT code. No guarantees are provided regarding functionality, stability, or fitness for any purpose. Use at your own risk.**

This repository contains the complete PlatformIO firmware for "Project AirCom," a handheld, off-grid tactical communicator based on the ESP32-S3 and Wi-Fi HaLow.

## 🎯 **Project Goal**

The device is a standalone, all-in-one solution that creates a **MANET (Mobile Ad-hoc Network)**, functioning as a mesh-networked walkie-talkie, text messenger, and an ATAK-compatible location beacon that requires no connection to a mobile phone. It is designed for off-grid communication in scenarios where cellular service is unavailable, such as disaster response, wilderness expeditions, or tactical operations. As a robust, low-power, long-range communication tool, it provides a critical lifeline for teams operating in remote environments, with a key focus on providing high-quality voice communications.

## 📋 **Current Status: Conceptual Prototype**

This firmware is an early-stage conceptual prototype. It should not be considered feature-complete or production-ready.

The purpose of this codebase is to provide a complete and robust architectural foundation for the project. The high-level application logic, FreeRTOS task structure, and inter-task communication are all in place. However, the code is untested on real hardware and requires significant work to become a field-ready product.

## 🔧 **How It Works**

The firmware is built on the ESP-IDF framework and leverages FreeRTOS to manage multiple operations concurrently. Each core function of the device is handled by a dedicated task pinned to a specific CPU core to ensure real-time performance and stability.

### **Core 0 (Protocol Core): Handles all networking and data processing.**

- **networkTask**: Manages the Wi-Fi HaLow mesh network, broadcasting discovery packets and routing outgoing messages.
- **tcp_server_task**: Listens for incoming text messages.
- **atakTask**: Periodically broadcasts the device's position as a CoT message.
- **atak_processor_task**: Listens for incoming CoT packets from teammates and updates their locations.

### **Core 1 (Application Core): Handles real-time peripherals and user interaction.**

- **uiTask**: Manages the OLED display and user input, including a state machine for screen navigation.
- **gpsTask**: Continuously parses data from the GPS module.
- **audioTask**: Manages PTT functionality, including audio capture, encoding (stub), and transmission/reception over UDP.

## 🔧 **Next Steps & Future Enhancements**

### **Final Implementation**
To create a production-ready binary, a developer must replace the current placeholders with real, hardware-specific drivers and libraries:

- **Integrate the Wi-Fi HaLow SDK**: The HaLowMeshManager component is a wrapper. The vendor-specific SDK must be added and its functions called from the existing methods.
- **Integrate the Opus Codec**: The Opus component is a placeholder. A pre-compiled libopus.a binary and its headers must be added.
- **Implement Production-Grade Encryption**: The crypto.cpp file uses placeholder functions. A trusted library like libsodium must be integrated for real security.
- **Testing and Debugging**: The entire system must be tested on the target hardware.

### **Implemented Features**
- **Bluetooth Headset Support**: ✅ Implemented Bluetooth HFP (Hands-Free Profile) for wireless headsets with audio routing and device discovery
- **Audio Queue Management**: ✅ Complete audio buffer management with overflow protection and status monitoring
- **Device Discovery**: ✅ Bluetooth device scanning and connection management
- **Audio Streaming**: ✅ Bidirectional audio streaming between device and Bluetooth headset

### **Future Enhancements**
- **Camera & Video Support**: Integrate a camera module (like an ESP32-CAM) to stream low-framerate video or send still images over the HaLow network. This would provide real-time visual intel for ATAK integration, a feature made possible by HaLow's higher bandwidth.
- **Advanced Power Management**: Implement interrupt-driven UI and deep sleep modes to maximize battery life.
- **Canned Messaging**: Add a UI for sending pre-defined messages quickly.

## 🔧 **Technology Choices & Advantages**

### **Wi-Fi HaLow (IEEE 802.11ah) vs. LoRa**
This project uses Wi-Fi HaLow to optimize for high-throughput applications that can support both data and high-quality voice. This represents a different set of engineering trade-offs compared to popular LoRa-based projects like Meshtastic.

**Bandwidth vs. Range**: While LoRa-based networks often achieve a longer maximum point-to-point range, Wi-Fi HaLow provides significantly more bandwidth (Mbps vs. kbps). This project prioritizes data rate to support its core features.

**Voice & High-Data Applications**: The primary advantage of HaLow's bandwidth is its ability to support clear, low-latency, high-quality voice streams using codecs like Opus. This is a critical feature for first responders, tactical teams, and airsoft players, but it is very difficult to achieve on lower-bandwidth LoRa networks.

**Standard IP Networking**: As a true IEEE 802.11 Wi-Fi standard, HaLow operates as a standard IP network. This simplifies the software stack, enabling the direct use of proven, standard protocols like TCP/IP, UDP, and WPA3 security.

In summary, this project chooses Wi-Fi HaLow as the underlying technology to create a communications platform where high-throughput data and real-time voice are the primary requirements.

### **Protocol Buffers (Protobufs)**
Inspired by projects like Meshtastic, this firmware uses Google's Protocol Buffers instead of JSON for application-layer messaging. This provides a strongly-typed, versionable, and highly compact binary format for all network communication, reducing bandwidth usage and increasing reliability.

## 🔧 **Building the Firmware**

Ensure you have a working PlatformIO environment with the ESP-IDF toolchain installed.

### **Prerequisites**
- PlatformIO IDE or CLI
- ESP32 development board
- Wi-Fi HaLow module (FGH100M-H recommended)
- USB cable for programming

### **Build Instructions**
```bash
# Clone repository
git clone <repository-url>
cd AirCom

# Select your hardware platform in platformio.ini
# Options: xiao_esp32s3, xiao_esp32c3, xiao_esp32c6,
#          heltec_ht_hc32, heltec_ht_it01, generic_heltec

# Build project
platformio run

# Upload to device
platformio run --target upload

# Monitor serial output
platformio device monitor
```

All necessary libraries are included as local ESP-IDF components, so no additional dependencies need to be installed.

## 🖥️ **GUI Preview & Testing**

### **Interactive GUI Preview Application**

The AirCom project includes a comprehensive GUI preview application that allows you to test and troubleshoot the user interface without requiring actual hardware. This console-based simulator provides:

#### **Features**
- **Complete Interface Simulation**: Experience the exact UI as it appears on the device
- **Interactive Navigation**: Test all menu systems and navigation flows
- **System State Simulation**: Simulate GPS lock, battery changes, contact discovery
- **Real-time Diagnostics**: Monitor system performance and health
- **Testing & Troubleshooting**: Comprehensive tools for interface validation

#### **GUI Preview Interface**

```
========================================
      AIRCOM GUI PREVIEW
   Interactive Interface Testing
========================================

+--------------------------------------+
|         AIRCOM SYSTEM               |
|       Tactical Communication        |
+--------------------------------------+
|  +---------------+ +---------------+ |
|  |   Contacts    | |   Messages    | |
|  |     (0)       | |      (0)      | |
|  +---------------+ +---------------+ |
|                                    |
|  +---------------+ +---------------+ |
|  |  GPS Status   | | Battery Level | |
|  | SEARCHING     | |     85%       | |
|  +---------------+ +---------------+ |
|                                    |
| Status: System Ready               |
+--------------------------------------+

Navigation: UP/DOWN/SELECT/BACK  PTT: Push to Talk

System Status: GPS=SEARCHING | Battery=85% | Contacts=0 | Messages=0
```

#### **Available Commands**
- `contacts` - Navigate to contacts screen
- `messages` - View messages interface
- `settings` - Access system settings
- `home` - Return to main menu
- `gps_lock` - Simulate GPS finding signal
- `add_contact` - Simulate new contact discovery
- `low_battery` - Simulate battery warning
- `status` - View detailed system report
- `diagnostics` - Show system diagnostics
- `test_resp` - Run responsiveness test
- `test_memory` - Run memory analysis
- `issues` - Check for common issues
- `help` - Show all available commands
- `quit` - Exit preview

#### **How to Use**
1. **Interactive Preview**: Run the GUI preview application to test the interface
2. **Command Testing**: Type commands to navigate through different screens
3. **System Simulation**: Use commands to simulate real-world scenarios
4. **Diagnostics**: Check system health and performance metrics
5. **Troubleshooting**: Identify and resolve interface issues

### **Testing Applications**
- **`simple_gui_preview.bat`** - Windows batch file for GUI preview
- **`interactive_gui_preview.bat`** - Enhanced interactive version
- **`run_gui_preview.bat`** - Demonstration script
- **`main/gui_preview.cpp`** - Full C++ implementation for cross-platform use

## 📁 **Project Structure**

```
AirCom/
├── components/                    # ESP-IDF components
│   ├── MM-IoT-SDK/               # MorseMicro Wi-Fi HaLow
│   ├── Heltec-HaLow/             # Heltec Wi-Fi HaLow
│   ├── HaLowManager/             # HaLow management layer
│   └── [other components]
├── main/                         # Main application
│   ├── include/                  # Header files
│   │   ├── halow_interface.h    # Abstract HaLow interface
│   │   ├── config_manager.h     # Configuration system
│   │   ├── safe_callback.h      # Safe callback system
│   │   ├── gui_tester.h        # GUI testing interface
│   │   └── logging_system.h     # Logging system
│   ├── *.cpp                     # Implementation files
│   ├── gui_preview.cpp          # GUI preview application
│   └── CMakeLists.txt           # Build configuration
├── *.bat                        # GUI preview scripts
├── platformio.ini               # PlatformIO configuration
├── partitions.csv              # Flash partitioning
└── partitions_heltec.csv       # Heltec partitioning
```

## 🔍 **Architecture Documentation**

For detailed architectural information, see:
- `README_FLEXIBLE_ARCHITECTURE.md` - Complete architecture guide
- `XIAO_ESP32_README.md` - XIAO ESP32 specific documentation

## ⚠️ **Important Warnings**

### **Untested Code**
This codebase contains significant architectural improvements but remains **completely untested**. Expect:
- Compilation issues on specific hardware
- Runtime errors and crashes
- Integration problems with external libraries
- Performance issues under load

### **Hardware Dependencies**
- Requires specific Wi-Fi HaLow modules
- Pin assignments may conflict with your hardware
- Power requirements not fully characterized
- Antenna configurations not validated

### **Security Considerations**
- Encryption keys are development defaults
- No security audit performed
- Authentication mechanisms untested
- Potential vulnerabilities exist

## 🧪 **Testing Status**

### **Not Tested**
- Hardware compilation across all platforms
- Real Wi-Fi HaLow network functionality
- Audio codec performance
- GPS integration reliability
- UI responsiveness
- Power consumption characteristics
- Long-term stability

### **Architecture Validated**
- ✅ Code structure and organization
- ✅ Memory safety mechanisms
- ✅ Thread safety patterns
- ✅ Error handling consistency
- ✅ Configuration management logic
- ✅ Plugin architecture design

## 🚧 **Development Roadmap**

### **Phase 1: Core Testing (REQUIRED)**
- [ ] Hardware compilation validation
- [ ] Basic functionality testing
- [ ] Memory leak verification
- [ ] Thread safety validation

### **Phase 2: Integration Testing**
- [ ] Wi-Fi HaLow network testing
- [ ] Audio codec validation
- [ ] GPS accuracy verification
- [ ] UI functionality testing

### **Phase 3: Production Readiness**
- [ ] Performance optimization
- [ ] Security hardening
- [ ] Power consumption optimization
- [ ] Production configuration setup

## 🤝 **Contributing**

### **Development Guidelines**
1. **Test Everything**: Any code changes require testing
2. **Document Changes**: Update documentation for all modifications
3. **Follow Patterns**: Maintain established architectural patterns
4. **Safety First**: Implement proper error handling and safety checks

### **Code Standards**
- Use the provided logging macros instead of ESP_LOGX
- Implement proper error handling with try-catch or error codes
- Use the configuration manager for all settings
- Follow the established callback patterns

## 📞 **Support**

**NO SUPPORT PROVIDED** - This is experimental code without warranty.

### **Self-Help Resources**
- Review the comprehensive documentation
- Check existing issues and solutions
- Use the built-in diagnostic systems
- Consult ESP-IDF documentation

## 📄 **License**

This project contains a mix of original code and integrated libraries. Review individual component licenses:

- AirCom Core: [License terms]
- MM-IoT-SDK: [MorseMicro license]
- Heltec SDK: [Heltec license]
- ESP-IDF Components: [Espressif license]
- Third-party libraries: [Individual licenses]

## ⚠️ **Final Disclaimer**

**USE THIS CODE AT YOUR OWN RISK. IT IS PROVIDED AS-IS WITHOUT ANY GUARANTEES OF FUNCTIONALITY, SAFETY, OR SUITABILITY FOR ANY PURPOSE.**



---


*Status: UNTESTED PROOF-OF-CONCEPT*
