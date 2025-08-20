# Project AirCom Firmware

This repository contains the complete PlatformIO firmware for "Project AirCom," a handheld, off-grid tactical communicator based on the ESP32-S3 and Wi-Fi HaLow.

## Project Goal

The device functions as a mesh-networked walkie-talkie, text messenger, and an ATAK (Android Team Awareness Kit) compatible location beacon. It is designed for off-grid communication in scenarios where cellular service is unavailable, such as disaster response, wilderness expeditions, or tactical operations. As a robust, low-power, long-range communication tool, it provides a critical lifeline for teams operating in remote environments, with a key focus on providing high-quality voice communications.

## Current Status

This firmware is **Feature Complete** at the application logic level. The core architecture is fully implemented, all required FreeRTOS tasks are running, and the high-level logic for every feature is in place. The project is now ready for final hardware integration and testing.

The final step is to replace the hardware abstraction stubs (`HaLowMeshManager`, `Opus` codec, `E2E Encryption`) with the real-world, production drivers and libraries for the target hardware.

## How It Works

The firmware is built on the **ESP-IDF framework** and leverages **FreeRTOS** to manage multiple operations concurrently. Each core function of the device is handled by a dedicated task pinned to a specific CPU core to ensure real-time performance and stability.

*   **Core 0 (Protocol Core):** Handles all networking and data processing.
    *   `networkTask`: Manages the Wi-Fi HaLow mesh network, including broadcasting discovery packets (using Protobufs) to find other nodes. It also handles the routing of outgoing text messages.
    *   `tcp_server_task`: Listens for incoming TCP connections for text messages, decrypts them (stub), unpacks the protobuf, and forwards the message to the UI task.
    *   `atakTask`: Periodically reads GPS data and broadcasts it as an ATAK-compatible Cursor-on-Target (CoT) XML packet over the mesh.

*   **Core 1 (Application Core):** Handles real-time peripherals and user interaction.
    *   `uiTask`: Manages the OLED display and all user input. It runs a state machine to switch between screens (Main, Contacts, Chat) and uses a dedicated button handler with debouncing and long-press detection. It receives updates from other tasks via queues to display dynamic information.
    *   `gpsTask`: Continuously reads and parses NMEA sentences from the hardware UART connected to the GPS module.
    *   `audioTask`: Manages the Push-to-Talk (PTT) functionality. On PTT press, it reads from the I2S microphone, encodes with Opus (stub), and broadcasts over UDP. It simultaneously listens for incoming UDP voice packets, decodes them, and plays them on the I2S speaker.

## Technology Choices & Advantages

### Wi-Fi HaLow (IEEE 802.11ah) vs. LoRa

This project uses **Wi-Fi HaLow** to optimize for high-throughput applications that can support both data and high-quality voice. This represents a different set of engineering trade-offs compared to popular LoRa-based projects like **Meshtastic**.

*   **Bandwidth vs. Range**: While LoRa-based networks often achieve a longer maximum point-to-point range, Wi-Fi HaLow provides significantly more bandwidth (Mbps vs. kbps). This project prioritizes data rate to support its core features.

*   **Voice & High-Data Applications**: The primary advantage of HaLow's bandwidth is its ability to support clear, low-latency, high-quality voice streams using codecs like Opus. This is a critical feature for first responders, tactical teams, and airsoft players, but it is very difficult to achieve on lower-bandwidth LoRa networks. This higher data rate also allows for future expansion into features like image or video transmission.

*   **Standard IP Networking**: As a true IEEE 802.11 Wi-Fi standard, HaLow operates as a standard IP network. This simplifies the software stack, enabling the direct use of proven, standard protocols like TCP/IP, UDP, and WPA3 security, as demonstrated in this firmware.

In summary, this project chooses Wi-Fi HaLow as the underlying technology to create a communications platform where high-throughput data and real-time voice are the primary requirements, accepting a potential trade-off in maximum range to achieve this goal.

### Protocol Buffers (Protobufs)
Inspired by projects like Meshtastic, this firmware uses Google's Protocol Buffers instead of JSON for application-layer messaging. This provides a strongly-typed, versionable, and highly compact binary format for all network communication, reducing bandwidth usage and increasing reliability.

## Building the Firmware
1.  Ensure you have a working PlatformIO environment with the ESP-IDF toolchain installed.
2.  Clone this repository. All necessary libraries are included as local ESP-IDF components.
3.  Connect your ESP32-S3 target board.
4.  Run `platformio run --target upload` to build and flash the firmware.
5.  Use `platformio device monitor` to view logging output.

## Hardware Dependencies and Final Implementation
This firmware is a complete application layer and architecture. To create a final, production-ready binary, you must provide the following hardware-specific and security-critical components:

1.  **Wi-Fi HaLow SDK**:
    *   **What is needed**: The proprietary SDK (header files and binary library) from the manufacturer of your Wi-Fi HaLow chip.
    *   **How to integrate**: Add the SDK files to the `components/HaLowManager` component and update the C++ methods in `HaLowMeshManager.cpp` to call the appropriate SDK functions for initialization and packet transmission.

2.  **Opus Codec Library**:
    *   **What is needed**: A pre-compiled binary (`libopus.a`) of the Opus codec, optimized for the ESP32, along with its public headers.
    *   **How to integrate**: Place the `.a` file in `components/Opus/lib/` and the header files in `components/Opus/include/`. The build system is already configured to link it automatically. Then, uncomment the Opus-related function calls in `audio_task.cpp`.

3.  **End-to-End Encryption Library**:
    *   **What is needed**: A trusted, peer-reviewed cryptographic library like `libsodium`.
    *   **How to integrate**: Add the library as a new ESP-IDF component. Replace the placeholder functions in `crypto.cpp` with calls to the real library's functions for key generation, encryption, and decryption. **The current XOR cipher is for demonstration only and provides no real security.**
