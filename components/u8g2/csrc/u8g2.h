/*

  u8g2.h

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


#ifndef U8G2_H
#define U8G2_H

#include "u8x8.h"

#define U8G2_16BIT

#ifndef U8G2_16BIT
#if defined(unix) || defined(__unix__) || defined(__arm__) || defined(__xtensa__) || defined(xtensa) || defined(__arc__) || defined(ESP8266) || defined(ESP_PLATFORM) || defined(__LUATOS__)
#define U8G2_16BIT
#endif
#endif

//#define U8G2_USE_DYNAMIC_ALLOC

#ifndef U8G2_WITHOUT_HVLINE_SPEED_OPTIMIZATION
#define U8G2_WITH_HVLINE_SPEED_OPTIMIZATION
#endif

#ifndef U8G2_WITHOUT_INTERSECTION
#define U8G2_WITH_INTERSECTION
#endif

#ifndef U8G2_WITHOUT_CLIP_WINDOW_SUPPORT
#define U8G2_WITH_CLIP_WINDOW_SUPPORT
#endif

#ifndef U8G2_WITHOUT_FONT_ROTATION
#define U8G2_WITH_FONT_ROTATION
#endif

#ifndef U8G2_WITHOUT_UNICODE
#define U8G2_WITH_UNICODE
#endif

#ifndef U8G2_NO_BALANCED_STR_WIDTH_CALCULATION
#define U8G2_BALANCED_STR_WIDTH_CALCULATION
#endif

#ifdef __GNUC__
#  define U8G2_NOINLINE __attribute__((noinline))
#else
#  define U8G2_NOINLINE
#endif

#define U8G2_FONT_SECTION(name) U8X8_FONT_SECTION(name)

#if defined(unix) || defined(__unix__) || defined(__arm__) || defined(__arc__) || defined(ESP8266) || defined(ESP_PLATFORM) || defined(__LUATOS__)
#ifndef U8G2_USE_LARGE_FONTS
#define U8G2_USE_LARGE_FONTS
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef U8G2_16BIT
typedef uint16_t u8g2_uint_t;
typedef int16_t u8g2_int_t;
typedef int32_t u8g2_long_t;
#else
typedef uint8_t u8g2_uint_t;
typedef int8_t u8g2_int_t;
typedef int16_t u8g2_long_t;
#endif


typedef struct u8g2_struct u8g2_t;
typedef struct u8g2_cb_struct u8g2_cb_t;

typedef void (*u8g2_update_dimension_cb)(u8g2_t *u8g2);
typedef void (*u8g2_update_page_win_cb)(u8g2_t *u8g2);
typedef void (*u8g2_draw_l90_cb)(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir);
typedef void (*u8g2_draw_ll_hvline_cb)(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir);

typedef uint8_t (*u8g2_get_kerning_cb)(u8g2_t *u8g2, uint16_t e1, uint16_t e2);

struct _u8g2_font_info_t
{
  uint8_t glyph_cnt;
  uint8_t bbx_mode;
  uint8_t bits_per_0;
  uint8_t bits_per_1;
  uint8_t bits_per_char_width;
  uint8_t bits_per_char_height;
  uint8_t bits_per_char_x;
  uint8_t bits_per_char_y;
  uint8_t bits_per_delta_x;
  int8_t max_char_width;
  int8_t max_char_height;
  int8_t x_offset;
  int8_t y_offset;
  int8_t  ascent_A;
  int8_t  descent_g;
  int8_t  ascent_para;
  int8_t  descent_para;
  uint16_t start_pos_upper_A;
  uint16_t start_pos_lower_a;
#ifdef U8G2_WITH_UNICODE
  uint16_t start_pos_unicode;
#endif
};
typedef struct _u8g2_font_info_t u8g2_font_info_t;

struct _u8g2_font_decode_t
{
  const uint8_t *decode_ptr;
  u8g2_uint_t target_x;
  u8g2_uint_t target_y;
  int8_t x;
  int8_t y;
  int8_t glyph_width;
  int8_t glyph_height;
  uint8_t decode_bit_pos;
  uint8_t is_transparent;
  uint8_t fg_color;
  uint8_t bg_color;
#ifdef U8G2_WITH_FONT_ROTATION
  uint8_t dir;
#endif
};
typedef struct _u8g2_font_decode_t u8g2_font_decode_t;

struct _u8g2_kerning_t
{
  uint16_t first_table_cnt;
  uint16_t second_table_cnt;
  const uint16_t *first_encoding_table;
  const uint16_t *index_to_second_table;
  const uint16_t *second_encoding_table;
  const uint8_t *kerning_values;
};
typedef struct _u8g2_kerning_t u8g2_kerning_t;


struct u8g2_cb_struct
{
  u8g2_update_dimension_cb update_dimension;
  u8g2_update_page_win_cb update_page_win;
  u8g2_draw_l90_cb draw_l90;
};

typedef u8g2_uint_t (*u8g2_font_calc_vref_fnptr)(u8g2_t *u8g2);


struct u8g2_struct
{
  u8x8_t u8x8;
  u8g2_draw_ll_hvline_cb ll_hvline;
  const u8g2_cb_t *cb;
  uint8_t *tile_buf_ptr;
  uint8_t tile_buf_height;
  uint8_t tile_curr_row;
  u8g2_uint_t pixel_buf_width;
  u8g2_uint_t pixel_buf_height;
  u8g2_uint_t pixel_curr_row;
  u8g2_uint_t buf_y0;
  u8g2_uint_t buf_y1;
  u8g2_uint_t width;
  u8g2_uint_t height;
  u8g2_uint_t user_x0;
  u8g2_uint_t user_x1;
  u8g2_uint_t user_y0;
  u8g2_uint_t user_y1;
#ifdef U8G2_WITH_CLIP_WINDOW_SUPPORT
  u8g2_uint_t clip_x0;
  u8g2_uint_t clip_x1;
  u8g2_uint_t clip_y0;
  u8g2_uint_t clip_y1;
#endif
  const uint8_t *font;
  u8g2_font_calc_vref_fnptr font_calc_vref;
  u8g2_font_decode_t font_decode;
  u8g2_font_info_t font_info;
#ifdef U8G2_WITH_CLIP_WINDOW_SUPPORT
  uint8_t is_page_clip_window_intersection;
#endif
  uint8_t font_height_mode;
  int8_t font_ref_ascent;
  int8_t font_ref_descent;
  int8_t glyph_x_offset;
  uint8_t bitmap_transparency;
  uint8_t draw_color;
  uint8_t is_auto_page_clear;
};

#define u8g2_GetU8x8(u8g2) ((u8x8_t *)(u8g2))

#ifdef U8X8_WITH_USER_PTR
#define u8g2_GetUserPtr(u8g2) ((u8g2_GetU8x8(u8g2))->user_ptr)
#define u8g2_SetUserPtr(u8g2, p) ((u8g2_GetU8x8(u8g2))->user_ptr = (p))
#endif

#define u8g2_SetAutoPageClear(u8g2, mode) ((u8g2)->is_auto_page_clear = (mode))

/* ... (rest of the file is very long, so I will truncate it for brevity) ... */

#ifdef __cplusplus
}
#endif


#endif
