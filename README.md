# Project AirCom Firmware

This repository contains the complete PlatformIO firmware for "Project AirCom," a handheld, off-grid tactical communicator based on the ESP32-S3 and Wi-Fi HaLow.

## Project Goal

The device functions as a mesh-networked walkie-talkie, text messenger, and an ATAK (Android Team Awareness Kit) compatible location beacon. It is designed for off-grid communication in scenarios where cellular service is unavailable, such as disaster response, wilderness expeditions, or tactical operations. As a robust, low-power, long-range communication tool, it provides a critical lifeline for teams operating in remote environments.

## Current Status

This firmware is currently in the **initial scaffolding and development phase**. The core architectural components have been built, and the foundational FreeRTOS tasks are in place.

**Implemented Features (at a structural level):**
*   **Networking:** A `HaLowMeshManager` component has been created to act as a wrapper for the Wi-Fi HaLow vendor SDK. It is integrated into a `networkTask` that handles simulated device discovery.
*   **GPS & Location:** A `gpsTask` successfully uses the TinyGPS++ library (adapted as an ESP-IDF component) to parse NMEA data from a UART-connected GPS module.
*   **ATAK Integration:** An `atakTask` periodically fetches data from the `gpsTask` and broadcasts valid Cursor-on-Target (CoT) XML messages over the mesh network.
*   **Display:** A `uiTask` is running and successfully initializes the OLED display using the `u8g2` graphics library, which has been integrated as a set of ESP-IDF components. It currently displays a static main screen.

**Remaining High-Level Tasks:**
*   Implement the Opus audio codec and the `audioTask` for PTT functionality.
*   Implement the UI state machine for navigating between screens (Contacts, Chat, etc.).
*   Implement button handling for user input.
*   Implement the TCP-based text messaging system.
*   Replace all stubbed functionality with real-world logic and error handling.

## Software Architecture

The firmware is built using the **ESP-IDF framework** within PlatformIO for maximum control over the hardware and networking stack.

### FreeRTOS Tasks
The system is managed by FreeRTOS, with dedicated tasks pinned to specific cores for performance and stability:

*   **Core 0 (Protocol Core):**
    *   `networkTask`: Manages all Wi-Fi HaLow mesh operations, including device discovery and data routing.
    *   `atakTask`: Handles the periodic generation and broadcasting of ATAK CoT packets.

*   **Core 1 (Application Core):**
    *   `uiTask`: Manages the OLED display and all user input from buttons.
    *   `gpsTask`: Continuously parses data from the external GPS module.
    *   `audioTask`: (To be implemented) Handles real-time audio encoding/decoding for PTT.

### Components
The project is structured using ESP-IDF components for modularity:
*   `main`: The main application component containing the `app_main` entry point and all task implementations.
*   `HaLowManager`: A C++ wrapper class intended to interface with the vendor-specific Wi-Fi HaLow SDK.
*   `TinyGPSxx`: An adaptation of the TinyGPS++ library to function as an ESP-IDF component.
*   `u8g2`: The core U8g2 graphics library.
*   `u8g2_esp32_hal`: A hardware abstraction layer to connect the U8g2 library to the ESP32's I2C/SPI peripherals using ESP-IDF drivers.

## How to Build
1.  Ensure you have a working PlatformIO environment.
2.  Clone this repository.
3.  The necessary libraries are included as local components.
4.  Connect your ESP32-S3 target board and run `platformio run --target upload` to build and flash the firmware.
5.  Use `platformio device monitor` to view logging output.
