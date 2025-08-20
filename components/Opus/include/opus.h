#ifndef OPUS_H
#define OPUS_H

// ============================================================================
// NOTE: This is a PLACEHOLDER file.
//
// You must replace this with the real opus.h header file from the Opus codec
// library. The official header files can be found at:
// https://github.com/xiph/opus/tree/master/include
// ============================================================================

#include <stdint.h>

// Define some opaque types to allow the code to compile.
// The real header will have the actual struct definitions.
typedef struct OpusEncoder OpusEncoder;
typedef struct OpusDecoder OpusDecoder;

// Define some function prototypes that are likely to be used.
// This allows the application code to be written, but it will not link
// without the real library.

#ifdef __cplusplus
extern "C" {
#endif

#define OPUS_OK 0
#define OPUS_AUTO -1000
#define OPUS_APPLICATION_VOIP 2048

OpusEncoder* opus_encoder_create(int32_t Fs, int channels, int application, int *error);
int32_t opus_encode(OpusEncoder *st, const int16_t *pcm, int frame_size, unsigned char *data, int32_t max_data_bytes);
void opus_encoder_destroy(OpusEncoder *st);

OpusDecoder* opus_decoder_create(int32_t Fs, int channels, int *error);
int opus_decode(OpusDecoder *st, const unsigned char *data, int32_t len, int16_t *pcm, int frame_size, int decode_fec);
void opus_decoder_destroy(OpusDecoder *st);

#ifdef __cplusplus
}
#endif

#endif // OPUS_H
