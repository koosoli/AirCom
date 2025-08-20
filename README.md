# Project AirCom Firmware

This repository contains the complete PlatformIO firmware for "Project AirCom," a handheld, off-grid tactical communicator based on the ESP32-S3 and Wi-Fi HaLow.

## Project Goal

The device is a **standalone, all-in-one solution** that functions as a mesh-networked walkie-talkie, text messenger, and an ATAK-compatible location beacon, requiring no connection to a mobile phone. It is designed for off-grid communication in scenarios where cellular service is unavailable, such as disaster response, wilderness expeditions, or tactical operations. As a robust, low-power, long-range communication tool, it provides a critical lifeline for teams operating in remote environments, with a key focus on providing high-quality voice communications.

## Current Status: Conceptual Prototype

This firmware is an **early-stage conceptual prototype**. It should not be considered feature-complete or production-ready.

The purpose of this codebase is to provide a complete and robust **architectural foundation** for the project. The high-level application logic, FreeRTOS task structure, and inter-task communication are all in place. However, the code is **untested on real hardware** and requires significant work to become a field-ready product.

## How It Works

The firmware is built on the **ESP-IDF framework** and leverages **FreeRTOS** to manage multiple operations concurrently. Each core function of the device is handled by a dedicated task pinned to a specific CPU core to ensure real-time performance and stability.

*   **Core 0 (Protocol Core):** Handles all networking and data processing.
    *   `networkTask`: Manages the Wi-Fi HaLow mesh network, broadcasting discovery packets and routing outgoing messages.
    *   `tcp_server_task`: Listens for incoming text messages.
    *   `atakTask`: Periodically broadcasts the device's position as a CoT message.
    *   `atak_processor_task`: Listens for incoming CoT packets from teammates and updates their locations.

*   **Core 1 (Application Core):** Handles real-time peripherals and user interaction.
    *   `uiTask`: Manages the OLED display and user input, including a state machine for screen navigation.
    *   `gpsTask`: Continuously parses data from the GPS module.
    *   `audioTask`: Manages PTT functionality, including audio capture, encoding (stub), and transmission/reception over UDP.

## Next Steps & Future Enhancements

### Final Implementation
To create a production-ready binary, a developer must replace the current placeholders with real, hardware-specific drivers and libraries:

1.  **Integrate the Wi-Fi HaLow SDK**: The `HaLowMeshManager` component is a wrapper. The vendor-specific SDK must be added and its functions called from the existing methods.
2.  **Integrate the Opus Codec**: The `Opus` component is a placeholder. A pre-compiled `libopus.a` binary and its headers must be added.
3.  **Implement Production-Grade Encryption**: The `crypto.cpp` file uses placeholder functions. A trusted library like `libsodium` must be integrated for real security.
4.  **Testing and Debugging**: The entire system must be tested on the target hardware.

### Future Enhancements
*   **Bluetooth Headset Support**: Integrate the Bluetooth A2DP profile to allow the use of wireless headsets for audio, modifying the `audioTask` to route audio to/from the Bluetooth stack instead of I2S.
*   **Camera & Video Support**: Integrate a camera module (like an ESP32-CAM) to stream low-framerate video or send still images over the HaLow network. This would provide real-time visual intel for ATAK integration, a feature made possible by HaLow's higher bandwidth.
*   **Advanced Power Management**: Implement interrupt-driven UI and deep sleep modes to maximize battery life.
*   **Canned Messaging**: Add a UI for sending pre-defined messages quickly.

## Technology Choices & Advantages

### Wi-Fi HaLow (IEEE 802.11ah) vs. LoRa

This project uses **Wi-Fi HaLow** to optimize for high-throughput applications that can support both data and high-quality voice. This represents a different set of engineering trade-offs compared to popular LoRa-based projects like **Meshtastic**.

*   **Bandwidth vs. Range**: While LoRa-based networks often achieve a longer maximum point-to-point range, Wi-Fi HaLow provides significantly more bandwidth (Mbps vs. kbps). This project prioritizes data rate to support its core features.
*   **Voice & High-Data Applications**: The primary advantage of HaLow's bandwidth is its ability to support clear, low-latency, high-quality voice streams using codecs like Opus. This is a critical feature for first responders, tactical teams, and airsoft players, but it is very difficult to achieve on lower-bandwidth LoRa networks.
*   **Standard IP Networking**: As a true IEEE 802.11 Wi-Fi standard, HaLow operates as a standard IP network. This simplifies the software stack, enabling the direct use of proven, standard protocols like TCP/IP, UDP, and WPA3 security.

In summary, this project chooses Wi-Fi HaLow as the underlying technology to create a communications platform where high-throughput data and real-time voice are the primary requirements.

### Protocol Buffers (Protobufs)
Inspired by projects like Meshtastic, this firmware uses Google's Protocol Buffers instead of JSON for application-layer messaging. This provides a strongly-typed, versionable, and highly compact binary format for all network communication, reducing bandwidth usage and increasing reliability.

## Building the Firmware
1.  Ensure you have a working PlatformIO environment with the ESP-IDF toolchain installed.
2.  Clone this repository. All necessary libraries are included as local ESP-IDF components.
3.  Connect your ESP32-S3 target board.
4.  Run `platformio run --target upload` to build and flash the firmware.
5.  Use `platformio device monitor` to view logging output.
