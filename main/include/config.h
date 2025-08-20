#ifndef CONFIG_H
#define CONFIG_H

// =================================================================
// General Device Configuration
// =================================================================
#define CALLSIGN "AIRCOM-IDF"
#define TAG "AIRCOM" // Logging tag for ESP_LOG

// =================================================================
// Network Configuration
// =================================================================
#define MESH_ID "AirComMeshIDF"
#define MESH_PASSWORD "SuperSecretPassword" // Must be 8-63 chars
#define MESH_CHANNEL 6

// Multicast addresses and ports
#define MESH_MULTICAST_ADDR "239.255.42.99" // For IPv4-based mesh
#define MESH_MULTICAST_ADDR_IPV6 "ff02::1" // For IPv6-based mesh (more common in 802.11s)

#define MESH_DISCOVERY_PORT 4299
#define VOICE_PORT 5000
#define TEXT_PORT 5001
#define ATAK_PORT 6969

// =================================================================
// Hardware Pin Assignments (Example Pins - MUST BE VERIFIED)
// =================================================================

// I2C for OLED Display
#define PIN_OLED_SDA 21
#define PIN_OLED_SCL 22

// I2S Audio
#define PIN_I2S_BCLK 26
#define PIN_I2S_LRC 25
#define PIN_I2S_DIN 33  // Microphone In
#define PIN_I2S_DOUT 32 // Speaker Out

// Buttons
#define PIN_BUTTON_PTT 4
#define PIN_BUTTON_UP 5
#define PIN_BUTTON_DOWN 18
#define PIN_BUTTON_SELECT 19
#define PIN_BUTTON_BACK 23

// GPS Module UART
#define GPS_UART_NUM UART_NUM_1
#define PIN_GPS_RX 16
#define PIN_GPS_TX 17
#define GPS_BAUD_RATE 9600

// =================================================================
// RTOS Configuration
// =================================================================
#define STACK_SIZE_DEFAULT (4 * 1024) // 4KB stack size for tasks

#endif // CONFIG_H
