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

While projects like **Meshtastic** have successfully created mesh networks using **LoRa**, this project uses **Wi-Fi HaLow** for several key advantages, particularly for the target use case of first responders and tactical teams:

1.  **Significantly Higher Bandwidth**:
    *   **LoRa**: Typically offers bandwidth in the kilobits per second (kbps). This is excellent for short, intermittent data like text messages or GPS coordinates, but it is highly constrained for real-time, high-quality voice.
    *   **Wi-Fi HaLow**: Offers bandwidth in the megabits per second (Mbps). This is orders of magnitude higher than LoRa.

2.  **Superior Voice Communication**:
    *   The primary advantage of HaLow's bandwidth is its ability to support **clear, low-latency, high-quality voice streams**. For first responders, intelligible voice communication is non-negotiable. Our project uses the Opus codec, which can provide excellent voice quality at bitrates (e.g., 24-32 kbps) that are trivial for HaLow to handle but challenging or impossible for LoRa.

3.  **IP-Based Networking**:
    *   HaLow is a true IEEE 802.11 Wi-Fi standard. This means it operates as a standard IP network. This simplifies the entire software stack, allowing for the direct use of standard protocols like TCP/IP and UDP, as demonstrated in this firmware. It also allows for standard security protocols like WPA3.
    *   LoRa requires a custom, non-IP networking stack (like LoRaWAN) to be built on top of the physical radio layer, adding complexity.

4.  **Scalability and Data Throughput**:
    *   The higher data rate of HaLow allows the mesh network to support more nodes and handle more simultaneous data (ATAK updates, text messages, voice) without becoming congested.

In summary, while LoRa is an excellent technology for low-bandwidth sensor networks, **Wi-Fi HaLow is a superior choice for a tactical communication device where clear voice and robust data throughput are critical requirements.**

### Protocol Buffers (Protobufs)
Inspired by projects like Meshtastic, this firmware uses Google's Protocol Buffers instead of JSON for application-layer messaging. This provides a strongly-typed, versionable, and highly compact binary format for all network communication, reducing bandwidth usage and increasing reliability.

## Building the Firmware
1.  Ensure you have a working PlatformIO environment with the ESP-IDF toolchain installed.
2.  Clone this repository. All necessary libraries are included as local ESP-IDF components.
3.  Connect your ESP32-S3 target board.
4.  Run `platformio run --target upload` to build and flash the firmware.
5.  Use `platformio device monitor` to view logging output.
