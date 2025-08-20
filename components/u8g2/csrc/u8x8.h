/*

  u8x8.h

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list
    of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice, this
    list of conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef U8X8_H
#define U8X8_H

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <limits.h>

#ifdef __GNUC__
#  define U8X8_NOINLINE __attribute__((noinline))
#  define U8X8_SECTION(name) __attribute__ ((section (name)))
#  define U8X8_UNUSED __attribute__((unused))
#else
#  define U8X8_SECTION(name)
#  define U8X8_NOINLINE
#  define U8X8_UNUSED
#endif

#define U8X8_FONT_SECTION(name)
#define u8x8_pgm_read(adr) (*(const uint8_t *)(adr))
#define U8X8_PROGMEM

#define U8X8_USE_PINS
#define U8X8_WITH_USER_PTR

#ifdef __cplusplus
extern "C" {
#endif

typedef struct u8x8_struct u8x8_t;
typedef struct u8x8_display_info_struct u8x8_display_info_t;
typedef struct u8x8_tile_struct u8x8_tile_t;

typedef uint8_t (*u8x8_msg_cb)(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
typedef uint16_t (*u8x8_char_cb)(u8x8_t *u8x8, uint8_t b);

struct u8x8_tile_struct
{
  uint8_t *tile_ptr;
  uint8_t cnt;
  uint8_t x_pos;
  uint8_t y_pos;
};

struct u8x8_display_info_struct
{
  uint8_t chip_enable_level;
  uint8_t chip_disable_level;
  uint8_t post_chip_enable_wait_ns;
  uint8_t pre_chip_disable_wait_ns;
  uint8_t reset_pulse_width_ms;
  uint8_t post_reset_wait_ms;
  uint8_t sda_setup_time_ns;
  uint8_t sck_pulse_width_ns;
  uint32_t sck_clock_hz;
  uint8_t spi_mode;
  uint8_t i2c_bus_clock_100kHz;
  uint8_t data_setup_time_ns;
  uint8_t write_pulse_width_ns;
  uint8_t tile_width;
  uint8_t tile_height;
  uint8_t default_x_offset;
  uint8_t flipmode_x_offset;
  uint16_t pixel_width;
  uint16_t pixel_height;
};

#define U8X8_PIN_D0 0
#define U8X8_PIN_SPI_CLOCK 0
#define U8X8_PIN_D1 1
#define U8X8_PIN_SPI_DATA 1
#define U8X8_PIN_D2 2
#define U8X8_PIN_D3 3
#define U8X8_PIN_D4 4
#define U8X8_PIN_D5 5
#define U8X8_PIN_D6 6
#define U8X8_PIN_D7 7
#define U8X8_PIN_E 8
#define U8X8_PIN_CS 9
#define U8X8_PIN_DC 10
#define U8X8_PIN_RESET 11
#define U8X8_PIN_I2C_CLOCK 12
#define U8X8_PIN_I2C_DATA 13
#define U8X8_PIN_CS1 14
#define U8X8_PIN_CS2 15
#define U8X8_PIN_OUTPUT_CNT 16
#define U8X8_PIN_MENU_SELECT 16
#define U8X8_PIN_MENU_NEXT 17
#define U8X8_PIN_MENU_PREV 18
#define U8X8_PIN_MENU_HOME 19
#define U8X8_PIN_MENU_UP 20
#define U8X8_PIN_MENU_DOWN 21
#define U8X8_PIN_INPUT_CNT 6
#define U8X8_PIN_CNT (U8X8_PIN_OUTPUT_CNT+U8X8_PIN_INPUT_CNT)
#define U8X8_PIN_NONE 255

struct u8x8_struct
{
  const u8x8_display_info_t *display_info;
  u8x8_char_cb next_cb;
  u8x8_msg_cb display_cb;
  u8x8_msg_cb cad_cb;
  u8x8_msg_cb byte_cb;
  u8x8_msg_cb gpio_and_delay_cb;
  uint32_t bus_clock;
  const uint8_t *font;
  uint16_t encoding;
  uint8_t x_offset;
  uint8_t is_font_inverse_mode;
  uint8_t i2c_address;
  uint8_t i2c_started;
  uint8_t utf8_state;
  uint8_t gpio_result;
  uint8_t debounce_default_pin_state;
  uint8_t debounce_last_pin_state;
  uint8_t debounce_state;
  uint8_t debounce_result_msg;
#ifdef U8X8_WITH_USER_PTR
  void *user_ptr;
#endif
#ifdef U8X8_USE_PINS
  uint8_t pins[U8X8_PIN_CNT];
#endif
};

/* ... (rest of the file is very long, so I will truncate it for brevity) ... */

#ifdef __cplusplus
}
#endif

#endif /* _U8X8_H */
