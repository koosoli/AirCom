#ifndef AUDIO_CODEC_H
#define AUDIO_CODEC_H

// ============================================================================
// AUDIO CODEC WRAPPER - PRODUCTION READY IMPLEMENTATION
//
// This module provides a unified interface for audio compression/decompression
// with built-in error handling, fallback mechanisms, and performance monitoring.
//
// Features:
// - Opus codec integration with fallback to PCM
// - Configurable audio parameters
// - Error recovery and retry mechanisms
// - Performance monitoring and statistics
// - Memory-safe buffer management
// - Thread-safe operations
//
// Usage:
// 1. Initialize with audio_codec_init()
// 2. Configure with audio_codec_config_t
// 3. Encode/decode with audio_codec_encode/decode functions
// 4. Monitor performance with audio_codec_get_stats()
// 5. Cleanup with audio_codec_deinit()
// ============================================================================

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// AUDIO CODEC CONSTANTS
// ============================================================================

#define AUDIO_CODEC_MAX_FRAME_SIZE      960   // Maximum samples per frame (48kHz * 20ms)
#define AUDIO_CODEC_MAX_PACKET_SIZE     4000  // Maximum compressed packet size
#define AUDIO_CODEC_DEFAULT_SAMPLE_RATE 16000 // Default sample rate for voice
#define AUDIO_CODEC_DEFAULT_CHANNELS    1     // Mono for tactical comms
#define AUDIO_CODEC_DEFAULT_FRAME_MS    20    // 20ms frames for low latency

// Error codes
#define AUDIO_CODEC_OK                  0
#define AUDIO_CODEC_ERROR_INIT         -1
#define AUDIO_CODEC_ERROR_CONFIG       -2
#define AUDIO_CODEC_ERROR_ENCODE       -3
#define AUDIO_CODEC_ERROR_DECODE       -4
#define AUDIO_CODEC_ERROR_MEMORY       -5
#define AUDIO_CODEC_ERROR_INVALID_PARAM -6
#define AUDIO_CODEC_ERROR_NOT_INIT     -7
#define AUDIO_CODEC_ERROR_TIMEOUT      -8

// Codec types
typedef enum {
    AUDIO_CODEC_TYPE_OPUS,        // Use Opus codec (preferred)
    AUDIO_CODEC_TYPE_PCM,         // PCM fallback
    AUDIO_CODEC_TYPE_AUTO         // Auto-select based on availability
} audio_codec_type_t;

// ============================================================================
// AUDIO CODEC CONFIGURATION
// ============================================================================

typedef struct {
    audio_codec_type_t codec_type;      // Codec type to use
    int sample_rate;                    // Sample rate in Hz
    int channels;                       // Number of channels (1 or 2)
    int frame_size_ms;                  // Frame size in milliseconds
    int bitrate;                        // Target bitrate in bps (Opus only)
    bool enable_fec;                    // Enable forward error correction
    bool enable_vbr;                    // Enable variable bitrate
    int complexity;                     // Codec complexity (0-10)
    int packet_loss_perc;               // Expected packet loss percentage
    bool enable_dtx;                    // Enable discontinuous transmission
} audio_codec_config_t;

// Default configuration
extern const audio_codec_config_t AUDIO_CODEC_DEFAULT_CONFIG;

// ============================================================================
// AUDIO CODEC STATISTICS
// ============================================================================

typedef struct {
    uint32_t total_encoded_frames;      // Total frames encoded
    uint32_t total_decoded_frames;      // Total frames decoded
    uint32_t encode_errors;             // Number of encoding errors
    uint32_t decode_errors;             // Number of decoding errors
    uint32_t encode_retries;            // Number of encode retries
    uint32_t decode_retries;            // Number of decode retries
    uint32_t avg_encode_time_us;        // Average encoding time in microseconds
    uint32_t avg_decode_time_us;        // Average decoding time in microseconds
    uint32_t max_encode_time_us;        // Maximum encoding time
    uint32_t max_decode_time_us;        // Maximum decoding time
    float avg_compression_ratio;        // Average compression ratio
    uint32_t total_bytes_encoded;       // Total bytes encoded
    uint32_t total_bytes_decoded;       // Total bytes decoded
} audio_codec_stats_t;

// ============================================================================
// AUDIO CODEC API
// ============================================================================

/**
 * @brief Initialize the audio codec system
 *
 * @param config Configuration parameters
 * @return AUDIO_CODEC_OK on success, error code otherwise
 */
int audio_codec_init(const audio_codec_config_t* config);

/**
 * @brief Deinitialize the audio codec system
 */
void audio_codec_deinit(void);

/**
 * @brief Reconfigure the audio codec with new parameters
 *
 * @param config New configuration parameters
 * @return AUDIO_CODEC_OK on success, error code otherwise
 */
int audio_codec_reconfigure(const audio_codec_config_t* config);

/**
 * @brief Encode audio data
 *
 * @param input_pcm Input PCM data (interleaved if stereo)
 * @param input_samples Number of input samples per channel
 * @param output_buffer Output buffer for compressed data
 * @param output_buffer_size Size of output buffer
 * @param bytes_encoded Pointer to store number of bytes encoded
 * @return AUDIO_CODEC_OK on success, error code otherwise
 */
int audio_codec_encode(const int16_t* input_pcm, int input_samples,
                      uint8_t* output_buffer, size_t output_buffer_size,
                      size_t* bytes_encoded);

/**
 * @brief Decode compressed audio data
 *
 * @param input_buffer Input compressed data
 * @param input_bytes Number of bytes in input buffer
 * @param output_pcm Output PCM buffer (interleaved if stereo)
 * @param output_samples Number of samples to decode per channel
 * @param samples_decoded Pointer to store number of samples decoded
 * @return AUDIO_CODEC_OK on success, error code otherwise
 */
int audio_codec_decode(const uint8_t* input_buffer, size_t input_bytes,
                      int16_t* output_pcm, int output_samples,
                      int* samples_decoded);

/**
 * @brief Get codec statistics
 *
 * @param stats Pointer to store statistics
 * @return AUDIO_CODEC_OK on success, error code otherwise
 */
int audio_codec_get_stats(audio_codec_stats_t* stats);

/**
 * @brief Reset codec statistics
 */
void audio_codec_reset_stats(void);

/**
 * @brief Check if codec is initialized and ready
 *
 * @return true if ready, false otherwise
 */
bool audio_codec_is_ready(void);

/**
 * @brief Get the current codec type being used
 *
 * @return Current codec type
 */
audio_codec_type_t audio_codec_get_type(void);

/**
 * @brief Get the maximum frame size in samples for current configuration
 *
 * @return Maximum frame size in samples per channel
 */
int audio_codec_get_max_frame_size(void);

/**
 * @brief Get the recommended buffer sizes for current configuration
 *
 * @param input_buffer_size Recommended input buffer size in bytes
 * @param output_buffer_size Recommended output buffer size in bytes
 */
void audio_codec_get_buffer_sizes(size_t* input_buffer_size, size_t* output_buffer_size);

/**
 * @brief Perform codec health check
 *
 * @return AUDIO_CODEC_OK if healthy, error code otherwise
 */
int audio_codec_health_check(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_CODEC_H