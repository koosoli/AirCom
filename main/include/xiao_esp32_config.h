/**
 * @file xiao_esp32_config.h
 * @brief XIAO ESP32 series pin configuration for AirCom system
 *
 * This file provides pin assignments and hardware configurations
 * for different XIAO ESP32 board variants (ESP32S3, ESP32C3, ESP32C6).
 *
 * @author AirCom Development Team
 * @version 1.0.0
 * @date 2024
 */

#ifndef XIAO_ESP32_CONFIG_H
#define XIAO_ESP32_CONFIG_H

#include <stdint.h>

// Board type detection (should be defined in platformio.ini build_flags)
#ifdef CONFIG_XIAO_ESP32S3
    #define XIAO_BOARD_TYPE "ESP32S3"
    #define XIAO_BOARD_TYPE_ID 1
#elif CONFIG_XIAO_ESP32C3
    #define XIAO_BOARD_TYPE "ESP32C3"
    #define XIAO_BOARD_TYPE_ID 2
#elif CONFIG_XIAO_ESP32C6
    #define XIAO_BOARD_TYPE "ESP32C6"
    #define XIAO_BOARD_TYPE_ID 3
#else
    #warning "No XIAO board type defined, defaulting to ESP32S3"
    #define XIAO_BOARD_TYPE "ESP32S3"
    #define XIAO_BOARD_TYPE_ID 1
#endif

// ============================================================================
// XIAO ESP32S3 PIN ASSIGNMENTS
// ============================================================================

#if XIAO_BOARD_TYPE_ID == 1

// SPI pins for FGH100M-H module
#define PIN_SPI_MOSI        35
#define PIN_SPI_MISO        37
#define PIN_SPI_SCLK        36
#define PIN_SPI_CS          34
#define PIN_SPI_RESET       33
#define PIN_SPI_INT         38

// UART pins
#define PIN_UART_TX         43
#define PIN_UART_RX         44

// I2C pins
#define PIN_I2C_SDA         5
#define PIN_I2C_SCL         6

// GPIO pins
#define PIN_GPIO_0          1
#define PIN_GPIO_1          2
#define PIN_GPIO_2          3
#define PIN_GPIO_3          4
#define PIN_GPIO_4          7
#define PIN_GPIO_5          8

// LED pin (onboard)
#define PIN_LED             21

// Button pin (if available)
#define PIN_BUTTON          0

// Battery voltage monitoring
#define PIN_BATTERY_ADC     4

#endif // XIAO_ESP32S3

// ============================================================================
// XIAO ESP32C3 PIN ASSIGNMENTS
// ============================================================================

#if XIAO_BOARD_TYPE_ID == 2

// SPI pins for FGH100M-H module
#define PIN_SPI_MOSI        6
#define PIN_SPI_MISO        7
#define PIN_SPI_SCLK        8
#define PIN_SPI_CS          9
#define PIN_SPI_RESET       10
#define PIN_SPI_INT         5

// UART pins
#define PIN_UART_TX         21
#define PIN_UART_RX         20

// I2C pins
#define PIN_I2C_SDA         4
#define PIN_I2C_SCL         5

// GPIO pins
#define PIN_GPIO_0          0
#define PIN_GPIO_1          1
#define PIN_GPIO_2          2
#define PIN_GPIO_3          3
#define PIN_GPIO_4          10
#define PIN_GPIO_5          18

// LED pin (onboard)
#define PIN_LED             21

// Button pin (if available)
#define PIN_BUTTON          9

// Battery voltage monitoring
#define PIN_BATTERY_ADC     2

#endif // XIAO_ESP32C3

// ============================================================================
// XIAO ESP32C6 PIN ASSIGNMENTS
// ============================================================================

#if XIAO_BOARD_TYPE_ID == 3

// SPI pins for FGH100M-H module
#define PIN_SPI_MOSI        18
#define PIN_SPI_MISO        19
#define PIN_SPI_SCLK        20
#define PIN_SPI_CS          21
#define PIN_SPI_RESET       22
#define PIN_SPI_INT         17

// UART pins
#define PIN_UART_TX         24
#define PIN_UART_RX         23

// I2C pins
#define PIN_I2C_SDA         6
#define PIN_I2C_SCL         7

// GPIO pins
#define PIN_GPIO_0          0
#define PIN_GPIO_1          1
#define PIN_GPIO_2          2
#define PIN_GPIO_3          3
#define PIN_GPIO_4          4
#define PIN_GPIO_5          5

// LED pin (onboard)
#define PIN_LED             15

// Button pin (if available)
#define PIN_BUTTON          9

// Battery voltage monitoring
#define PIN_BATTERY_ADC     10

#endif // XIAO_ESP32C6

// ============================================================================
// SPI CONFIGURATION FOR FGH100M-H MODULE
// ============================================================================

// SPI bus configuration
#define FGH100M_SPI_HOST          SPI2_HOST
#define FGH100M_SPI_CLOCK_SPEED   1000000   // 1 MHz (adjust as needed)
#define FGH100M_SPI_MODE          0         // SPI mode
#define FGH100M_SPI_QUEUE_SIZE    7         // Queue size for transactions

// SPI command definitions (specific to FGH100M-H)
#define FGH100M_CMD_READ          0x03
#define FGH100M_CMD_WRITE         0x02
#define FGH100M_CMD_STATUS        0x05
#define FGH100M_CMD_SLEEP         0xB9
#define FGH100M_CMD_WAKEUP        0xAB

// Buffer sizes
#define FGH100M_MAX_TX_BUFFER     4096
#define FGH100M_MAX_RX_BUFFER     4096
#define FGH100M_HEADER_SIZE       4
#define FGH100M_PAYLOAD_SIZE      1024

// Timing constants (in milliseconds)
#define FGH100M_RESET_DELAY       100
#define FGH100M_STARTUP_DELAY     500
#define FGH100M_SPI_TIMEOUT       1000
#define FGH100M_COMMAND_TIMEOUT   5000

// ============================================================================
// I2C CONFIGURATION
// ============================================================================

#define I2C_MASTER_FREQ_HZ        400000    // 400 kHz
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_TIMEOUT_MS     1000

// ============================================================================
// UART CONFIGURATION
// ============================================================================

#define UART_BAUD_RATE            115200
#define UART_DATA_BITS            UART_DATA_8_BITS
#define UART_PARITY               UART_PARITY_DISABLE
#define UART_STOP_BITS            UART_STOP_BITS_1
#define UART_FLOW_CTRL            UART_HW_FLOWCTRL_DISABLE

// ============================================================================
// BOARD-SPECIFIC CONFIGURATION FUNCTIONS
// ============================================================================

/**
 * @brief Get the SPI MOSI pin for the current board
 * @return GPIO pin number
 */
static inline int get_spi_mosi_pin(void) {
    return PIN_SPI_MOSI;
}

/**
 * @brief Get the SPI MISO pin for the current board
 * @return GPIO pin number
 */
static inline int get_spi_miso_pin(void) {
    return PIN_SPI_MISO;
}

/**
 * @brief Get the SPI SCLK pin for the current board
 * @return GPIO pin number
 */
static inline int get_spi_sclk_pin(void) {
    return PIN_SPI_SCLK;
}

/**
 * @brief Get the SPI CS pin for the current board
 * @return GPIO pin number
 */
static inline int get_spi_cs_pin(void) {
    return PIN_SPI_CS;
}

/**
 * @brief Get the SPI RESET pin for the current board
 * @return GPIO pin number
 */
static inline int get_spi_reset_pin(void) {
    return PIN_SPI_RESET;
}

/**
 * @brief Get the SPI INT pin for the current board
 * @return GPIO pin number
 */
static inline int get_spi_int_pin(void) {
    return PIN_SPI_INT;
}

/**
 * @brief Get the LED pin for the current board
 * @return GPIO pin number
 */
static inline int get_led_pin(void) {
    return PIN_LED;
}

/**
 * @brief Get the button pin for the current board (if available)
 * @return GPIO pin number, or -1 if not available
 */
static inline int get_button_pin(void) {
    #ifdef PIN_BUTTON
        return PIN_BUTTON;
    #else
        return -1;
    #endif
}

/**
 * @brief Get the battery ADC pin for the current board (if available)
 * @return ADC channel number, or -1 if not available
 */
static inline int get_battery_adc_pin(void) {
    #ifdef PIN_BATTERY_ADC
        return PIN_BATTERY_ADC;
    #else
        return -1;
    #endif
}

#endif // XIAO_ESP32_CONFIG_H