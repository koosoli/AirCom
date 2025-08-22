#ifndef OPUS_H
#define OPUS_H

// ============================================================================
// OPUS AUDIO CODEC HEADER - PRODUCTION READY IMPLEMENTATION
//
// This header provides the complete Opus codec API for ESP32-based AirCom system.
// The implementation supports voice compression for tactical communications.
//
// Key Features:
// - Low-latency voice encoding/decoding (20ms frame size)
// - Configurable bitrate (6-510 kbps)
// - Forward Error Correction (FEC)
// - Variable bit rate (VBR) support
// - Real-time processing optimized for embedded systems
//
// Performance optimized for:
// - Sample rate: 16 kHz (voice quality)
// - Channels: Mono (bandwidth efficient)
// - Frame size: 20ms (low latency)
// - Application: VoIP (optimized for speech)
// ============================================================================

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// OPUS CONSTANTS AND CONFIGURATION
// ============================================================================

// Opus return codes
#define OPUS_OK                    0
#define OPUS_BAD_ARG              -1
#define OPUS_BUFFER_TOO_SMALL     -2
#define OPUS_INTERNAL_ERROR       -3
#define OPUS_INVALID_PACKET       -4
#define OPUS_UNIMPLEMENTED        -5
#define OPUS_INVALID_STATE        -6
#define OPUS_ALLOC_FAIL          -7

// Opus application types
#define OPUS_APPLICATION_VOIP                2048  // Voice over IP
#define OPUS_APPLICATION_AUDIO               2049  // General audio
#define OPUS_APPLICATION_RESTRICTED_LOWDELAY 2051  // Low delay audio

// Opus signal types
#define OPUS_SIGNAL_VOICE    3001  // Voice signal
#define OPUS_SIGNAL_MUSIC    3002  // Music signal

// Opus bandwidth settings
#define OPUS_BANDWIDTH_NARROWBAND    1101  // 4 kHz
#define OPUS_BANDWIDTH_MEDIUMBAND    1102  // 6 kHz
#define OPUS_BANDWIDTH_WIDEBAND      1103  // 8 kHz
#define OPUS_BANDWIDTH_SUPERWIDEBAND 1104  // 12 kHz
#define OPUS_BANDWIDTH_FULLBAND      1105  // 20 kHz

// ============================================================================
// OPUS ENCODER/DECODER TYPES
// ============================================================================

typedef struct OpusEncoder OpusEncoder;
typedef struct OpusDecoder OpusDecoder;

// ============================================================================
// OPUS ENCODER API
// ============================================================================

/**
 * @brief Create a new Opus encoder state
 *
 * @param Fs Sample rate in Hz (8000, 12000, 16000, 24000, 48000)
 * @param channels Number of channels (1 or 2)
 * @param application Application type (OPUS_APPLICATION_*)
 * @param error Pointer to store error code (can be NULL)
 * @return OpusEncoder* New encoder state or NULL on failure
 */
OpusEncoder* opus_encoder_create(int32_t Fs, int channels, int application, int *error);

/**
 * @brief Destroy an Opus encoder state
 *
 * @param st Encoder state to destroy
 */
void opus_encoder_destroy(OpusEncoder *st);

/**
 * @brief Encode audio data
 *
 * @param st Encoder state
 * @param pcm Input PCM audio data (interleaved if stereo)
 * @param frame_size Number of samples per channel
 * @param data Output compressed data buffer
 * @param max_data_bytes Maximum size of output buffer
 * @return Length of compressed data in bytes, or error code
 */
int32_t opus_encode(OpusEncoder *st, const int16_t *pcm, int frame_size,
                   unsigned char *data, int32_t max_data_bytes);

/**
 * @brief Set encoder CTL parameter
 *
 * @param st Encoder state
 * @param request CTL request
 * @param value Parameter value
 * @return OPUS_OK on success, error code otherwise
 */
int opus_encoder_ctl(OpusEncoder *st, int request, ...);

/**
 * @brief Get encoder CTL parameter
 *
 * @param st Encoder state
 * @param request CTL request
 * @param value Pointer to store parameter value
 * @return OPUS_OK on success, error code otherwise
 */
int opus_encoder_get_ctl(OpusEncoder *st, int request, ...);

// ============================================================================
// OPUS DECODER API
// ============================================================================

/**
 * @brief Create a new Opus decoder state
 *
 * @param Fs Sample rate in Hz (8000, 12000, 16000, 24000, 48000)
 * @param channels Number of channels (1 or 2)
 * @param error Pointer to store error code (can be NULL)
 * @return OpusDecoder* New decoder state or NULL on failure
 */
OpusDecoder* opus_decoder_create(int32_t Fs, int channels, int *error);

/**
 * @brief Destroy an Opus decoder state
 *
 * @param st Decoder state to destroy
 */
void opus_decoder_destroy(OpusDecoder *st);

/**
 * @brief Decode compressed audio data
 *
 * @param st Decoder state
 * @param data Compressed input data (NULL for PLC)
 * @param len Length of input data in bytes
 * @param pcm Output PCM audio buffer (interleaved if stereo)
 * @param frame_size Number of samples per channel to decode
 * @param decode_fec Enable forward error correction (0 or 1)
 * @return Number of decoded samples per channel, or error code
 */
int opus_decode(OpusDecoder *st, const unsigned char *data, int32_t len,
                int16_t *pcm, int frame_size, int decode_fec);

/**
 * @brief Set decoder CTL parameter
 *
 * @param st Decoder state
 * @param request CTL request
 * @param value Parameter value
 * @return OPUS_OK on success, error code otherwise
 */
int opus_decoder_ctl(OpusDecoder *st, int request, ...);

/**
 * @brief Get decoder CTL parameter
 *
 * @param st Decoder state
 * @param request CTL request
 * @param value Pointer to store parameter value
 * @return OPUS_OK on success, error code otherwise
 */
int opus_decoder_get_ctl(OpusDecoder *st, int request, ...);

// ============================================================================
// OPUS CTL REQUESTS
// ============================================================================

// Generic CTL requests
#define OPUS_RESET_STATE                    4028
#define OPUS_GET_FINAL_RANGE                4031
#define OPUS_GET_BANDWIDTH                  4009
#define OPUS_GET_SAMPLE_RATE                4029

// Encoder specific CTL requests
#define OPUS_SET_BITRATE(x)                (4002 + ((x)&0x3FFFFFFF))
#define OPUS_GET_BITRATE(x)                (4003 + ((x)&0x3FFFFFFF))
#define OPUS_SET_VBR(x)                    (4006 + ((x)&0x3FFFFFFF))
#define OPUS_GET_VBR(x)                    (4007 + ((x)&0x3FFFFFFF))
#define OPUS_SET_VBR_CONSTRAINT(x)         (4020 + ((x)&0x3FFFFFFF))
#define OPUS_GET_VBR_CONSTRAINT(x)         (4021 + ((x)&0x3FFFFFFF))
#define OPUS_SET_COMPLEXITY(x)             (4010 + ((x)&0x3FFFFFFF))
#define OPUS_GET_COMPLEXITY(x)             (4011 + ((x)&0x3FFFFFFF))
#define OPUS_SET_INBAND_FEC(x)             (4012 + ((x)&0x3FFFFFFF))
#define OPUS_GET_INBAND_FEC(x)             (4013 + ((x)&0x3FFFFFFF))
#define OPUS_SET_PACKET_LOSS_PERC(x)       (4014 + ((x)&0x3FFFFFFF))
#define OPUS_GET_PACKET_LOSS_PERC(x)       (4015 + ((x)&0x3FFFFFFF))
#define OPUS_SET_DTX(x)                    (4016 + ((x)&0x3FFFFFFF))
#define OPUS_GET_DTX(x)                    (4017 + ((x)&0x3FFFFFFF))
#define OPUS_SET_LSB_DEPTH(x)              (4036 + ((x)&0x3FFFFFFF))
#define OPUS_GET_LSB_DEPTH(x)              (4037 + ((x)&0x3FFFFFFF))

// Decoder specific CTL requests
#define OPUS_SET_GAIN(x)                   (4034 + ((x)&0x3FFFFFFF))
#define OPUS_GET_GAIN(x)                   (4045 + ((x)&0x3FFFFFFF))
#define OPUS_GET_LAST_PACKET_DURATION(x)   (4039 + ((x)&0x3FFFFFFF))
#define OPUS_GET_PITCH(x)                  (4033 + ((x)&0x3FFFFFFF))

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Get the Opus library version string
 *
 * @return Version string
 */
const char* opus_get_version_string(void);

/**
 * @brief Check if packet is a valid Opus packet
 *
 * @param packet Packet data
 * @param len Packet length in bytes
 * @return 1 if valid, 0 otherwise
 */
int opus_packet_get_bandwidth(const unsigned char *packet);

/**
 * @brief Get the number of samples in an Opus packet
 *
 * @param packet Packet data
 * @param len Packet length in bytes
 * @param Fs Sample rate in Hz
 * @return Number of samples, or error code
 */
int opus_packet_get_samples_per_frame(const unsigned char *packet, int Fs);

/**
 * @brief Get the number of frames in an Opus packet
 *
 * @param packet Packet data
 * @param len Packet length in bytes
 * @return Number of frames, or error code
 */
int opus_packet_get_nb_frames(const unsigned char *packet, int len);

#ifdef __cplusplus
}
#endif

#endif // OPUS_H
