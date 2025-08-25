#include "audio_codec.h"
#include "opus.h"
#include <string.h>
#include <stdlib.h>
#include <esp_log.h>
#include <esp_timer.h>

static const char* TAG = "AUDIO_CODEC";

// ============================================================================
// INTERNAL STATE
// ============================================================================

static bool g_initialized = false;
static audio_codec_config_t g_config = {0};
static audio_codec_stats_t g_stats = {0};
static OpusEncoder* g_encoder = NULL;
static OpusDecoder* g_decoder = NULL;
static audio_codec_type_t g_current_type = AUDIO_CODEC_TYPE_AUTO;

// Default configuration
const audio_codec_config_t AUDIO_CODEC_DEFAULT_CONFIG = {
    .codec_type = AUDIO_CODEC_TYPE_OPUS,
    .sample_rate = AUDIO_CODEC_DEFAULT_SAMPLE_RATE,
    .channels = AUDIO_CODEC_DEFAULT_CHANNELS,
    .frame_size_ms = AUDIO_CODEC_DEFAULT_FRAME_MS,
    .bitrate = 24000,
    .enable_fec = true,
    .enable_vbr = true,
    .complexity = 5,
    .packet_loss_perc = 10,
    .enable_dtx = false
};

// ============================================================================
// INTERNAL HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Calculate frame size in samples from milliseconds
 */
static int calculate_frame_size(int sample_rate, int frame_size_ms) {
    return (sample_rate * frame_size_ms) / 1000;
}

/**
 * @brief Initialize Opus encoder with current configuration
 */
static int init_opus_encoder(void) {
    if (g_encoder) {
        opus_encoder_destroy(g_encoder);
        g_encoder = NULL;
    }

    int error;
    g_encoder = opus_encoder_create(g_config.sample_rate, g_config.channels,
                                   OPUS_APPLICATION_VOIP, &error);

    if (!g_encoder || error != OPUS_OK) {
        ESP_LOGE(TAG, "Failed to create Opus encoder: %d", error);
        return AUDIO_CODEC_ERROR_INIT;
    }

    // Configure encoder
    opus_encoder_ctl(g_encoder, OPUS_SET_BITRATE(g_config.bitrate));
    opus_encoder_ctl(g_encoder, OPUS_SET_VBR(g_config.enable_vbr));
    opus_encoder_ctl(g_encoder, OPUS_SET_COMPLEXITY(g_config.complexity));
    opus_encoder_ctl(g_encoder, OPUS_SET_INBAND_FEC(g_config.enable_fec));
    opus_encoder_ctl(g_encoder, OPUS_SET_PACKET_LOSS_PERC(g_config.packet_loss_perc));
    opus_encoder_ctl(g_encoder, OPUS_SET_DTX(g_config.enable_dtx));

    return AUDIO_CODEC_OK;
}

/**
 * @brief Initialize Opus decoder with current configuration
 */
static int init_opus_decoder(void) {
    if (g_decoder) {
        opus_decoder_destroy(g_decoder);
        g_decoder = NULL;
    }

    int error;
    g_decoder = opus_decoder_create(g_config.sample_rate, g_config.channels, &error);

    if (!g_decoder || error != OPUS_OK) {
        ESP_LOGE(TAG, "Failed to create Opus decoder: %d", error);
        return AUDIO_CODEC_ERROR_INIT;
    }

    return AUDIO_CODEC_OK;
}

/**
 * @brief Update codec statistics
 */
static void update_stats(bool is_encode, uint32_t processing_time_us,
                        size_t input_size, size_t output_size, bool success) {
    if (is_encode) {
        if (success) {
            g_stats.total_encoded_frames++;
            g_stats.total_bytes_encoded += output_size;
        } else {
            g_stats.encode_errors++;
        }
        g_stats.avg_encode_time_us = ((g_stats.avg_encode_time_us * (g_stats.total_encoded_frames - 1)) + processing_time_us) / g_stats.total_encoded_frames;
        if (processing_time_us > g_stats.max_encode_time_us) {
            g_stats.max_encode_time_us = processing_time_us;
        }
    } else {
        if (success) {
            g_stats.total_decoded_frames++;
            g_stats.total_bytes_decoded += input_size;
        } else {
            g_stats.decode_errors++;
        }
        g_stats.avg_decode_time_us = ((g_stats.avg_decode_time_us * (g_stats.total_decoded_frames - 1)) + processing_time_us) / g_stats.total_decoded_frames;
        if (processing_time_us > g_stats.max_decode_time_us) {
            g_stats.max_decode_time_us = processing_time_us;
        }
    }

    // Update compression ratio for encoding
    if (is_encode && success && g_stats.total_encoded_frames > 0) {
        g_stats.avg_compression_ratio = (float)g_stats.total_bytes_encoded / (g_stats.total_encoded_frames * input_size * sizeof(int16_t));
    }
}

/**
 * @brief Fallback PCM encoding (no compression)
 */
static int pcm_encode(const int16_t* input_pcm, int input_samples,
                     uint8_t* output_buffer, size_t output_buffer_size,
                     size_t* bytes_encoded) {
    size_t required_size = input_samples * sizeof(int16_t);

    if (output_buffer_size < required_size) {
        return AUDIO_CODEC_ERROR_ENCODE;
    }

    memcpy(output_buffer, input_pcm, required_size);
    *bytes_encoded = required_size;
    return AUDIO_CODEC_OK;
}

/**
 * @brief Fallback PCM decoding (no decompression)
 */
static int pcm_decode(const uint8_t* input_buffer, size_t input_bytes,
                     int16_t* output_pcm, int output_samples,
                     int* samples_decoded) {
    size_t required_size = output_samples * sizeof(int16_t);

    if (input_bytes < required_size) {
        return AUDIO_CODEC_ERROR_DECODE;
    }

    memcpy(output_pcm, input_buffer, required_size);
    *samples_decoded = output_samples;
    return AUDIO_CODEC_OK;
}

// ============================================================================
// PUBLIC API IMPLEMENTATION
// ============================================================================

int audio_codec_init(const audio_codec_config_t* config) {
    if (g_initialized) {
        ESP_LOGW(TAG, "Audio codec already initialized");
        return AUDIO_CODEC_ERROR_INIT;
    }

    if (!config) {
        ESP_LOGE(TAG, "Invalid configuration");
        return AUDIO_CODEC_ERROR_INVALID_PARAM;
    }

    // Copy configuration
    memcpy(&g_config, config, sizeof(audio_codec_config_t));

    // Determine codec type
    if (config->codec_type == AUDIO_CODEC_TYPE_AUTO) {
        // Try Opus first, fall back to PCM if not available
        g_current_type = AUDIO_CODEC_TYPE_OPUS;
    } else {
        g_current_type = config->codec_type;
    }

    // Initialize codec
    int result = AUDIO_CODEC_OK;

    if (g_current_type == AUDIO_CODEC_TYPE_OPUS) {
        result = init_opus_encoder();
        if (result == AUDIO_CODEC_OK) {
            result = init_opus_decoder();
        }

        if (result != AUDIO_CODEC_OK) {
            ESP_LOGW(TAG, "Opus codec failed, falling back to PCM");
            g_current_type = AUDIO_CODEC_TYPE_PCM;
            result = AUDIO_CODEC_OK;
        }
    }

    if (result == AUDIO_CODEC_OK) {
        g_initialized = true;
        memset(&g_stats, 0, sizeof(g_stats));
        ESP_LOGI(TAG, "Audio codec initialized with type: %d", g_current_type);
    }

    return result;
}

void audio_codec_deinit(void) {
    if (!g_initialized) {
        return;
    }

    if (g_encoder) {
        opus_encoder_destroy(g_encoder);
        g_encoder = NULL;
    }

    if (g_decoder) {
        opus_decoder_destroy(g_decoder);
        g_decoder = NULL;
    }

    g_initialized = false;
    ESP_LOGI(TAG, "Audio codec deinitialized");
}

int audio_codec_reconfigure(const audio_codec_config_t* config) {
    if (!g_initialized) {
        return AUDIO_CODEC_ERROR_NOT_INIT;
    }

    if (!config) {
        return AUDIO_CODEC_ERROR_INVALID_PARAM;
    }

    // Store old config for rollback
    audio_codec_config_t old_config;
    memcpy(&old_config, &g_config, sizeof(audio_codec_config_t));

    // Apply new configuration
    memcpy(&g_config, config, sizeof(audio_codec_config_t));

    // Reinitialize codecs if necessary
    int result = AUDIO_CODEC_OK;

    if (g_current_type == AUDIO_CODEC_TYPE_OPUS) {
        result = init_opus_encoder();
        if (result == AUDIO_CODEC_OK) {
            result = init_opus_decoder();
        }
    }

    if (result != AUDIO_CODEC_OK) {
        // Rollback on failure
        memcpy(&g_config, &old_config, sizeof(audio_codec_config_t));
        ESP_LOGE(TAG, "Failed to reconfigure codec, rolled back");
    } else {
        ESP_LOGI(TAG, "Audio codec reconfigured successfully");
    }

    return result;
}

int audio_codec_encode(const int16_t* input_pcm, int input_samples,
                      uint8_t* output_buffer, size_t output_buffer_size,
                      size_t* bytes_encoded) {
    if (!g_initialized) {
        return AUDIO_CODEC_ERROR_NOT_INIT;
    }

    if (!input_pcm || !output_buffer || !bytes_encoded) {
        return AUDIO_CODEC_ERROR_INVALID_PARAM;
    }

    uint64_t start_time = esp_timer_get_time();
    int result = AUDIO_CODEC_ERROR_ENCODE;
    bool success = false;

    if (g_current_type == AUDIO_CODEC_TYPE_OPUS && g_encoder) {
        int32_t encoded_bytes = opus_encode(g_encoder, input_pcm, input_samples,
                                           output_buffer, output_buffer_size);

        if (encoded_bytes > 0) {
            *bytes_encoded = (size_t)encoded_bytes;
            result = AUDIO_CODEC_OK;
            success = true;
        } else {
            ESP_LOGW(TAG, "Opus encoding failed: %d", encoded_bytes);
        }
    }

    // Fallback to PCM if Opus failed or not available
    if (result != AUDIO_CODEC_OK) {
        result = pcm_encode(input_pcm, input_samples, output_buffer,
                          output_buffer_size, bytes_encoded);
        if (result == AUDIO_CODEC_OK) {
            success = true;
        }
    }

    uint32_t processing_time = (uint32_t)(esp_timer_get_time() - start_time);
    update_stats(true, processing_time, input_samples * sizeof(int16_t), *bytes_encoded, success);

    return result;
}

int audio_codec_decode(const uint8_t* input_buffer, size_t input_bytes,
                      int16_t* output_pcm, int output_samples,
                      int* samples_decoded) {
    if (!g_initialized) {
        return AUDIO_CODEC_ERROR_NOT_INIT;
    }

    if (!input_buffer || !output_pcm || !samples_decoded) {
        return AUDIO_CODEC_ERROR_INVALID_PARAM;
    }

    uint64_t start_time = esp_timer_get_time();
    int result = AUDIO_CODEC_ERROR_DECODE;
    bool success = false;

    if (g_current_type == AUDIO_CODEC_TYPE_OPUS && g_decoder) {
        int decoded_samples = opus_decode(g_decoder, input_buffer, input_bytes,
                                         output_pcm, output_samples, 0);

        if (decoded_samples > 0) {
            *samples_decoded = decoded_samples;
            result = AUDIO_CODEC_OK;
            success = true;
        } else {
            ESP_LOGW(TAG, "Opus decoding failed: %d", decoded_samples);
        }
    }

    // Fallback to PCM if Opus failed or not available
    if (result != AUDIO_CODEC_OK) {
        result = pcm_decode(input_buffer, input_bytes, output_pcm,
                          output_samples, samples_decoded);
        if (result == AUDIO_CODEC_OK) {
            success = true;
        }
    }

    uint32_t processing_time = (uint32_t)(esp_timer_get_time() - start_time);
    update_stats(false, processing_time, input_bytes, *samples_decoded * sizeof(int16_t), success);

    return result;
}

int audio_codec_get_stats(audio_codec_stats_t* stats) {
    if (!g_initialized) {
        return AUDIO_CODEC_ERROR_NOT_INIT;
    }

    if (!stats) {
        return AUDIO_CODEC_ERROR_INVALID_PARAM;
    }

    memcpy(stats, &g_stats, sizeof(audio_codec_stats_t));
    return AUDIO_CODEC_OK;
}

void audio_codec_reset_stats(void) {
    memset(&g_stats, 0, sizeof(g_stats));
}

bool audio_codec_is_ready(void) {
    return g_initialized;
}

audio_codec_type_t audio_codec_get_type(void) {
    return g_current_type;
}

int audio_codec_get_max_frame_size(void) {
    if (!g_initialized) {
        return 0;
    }

    return calculate_frame_size(g_config.sample_rate, g_config.frame_size_ms);
}

void audio_codec_get_buffer_sizes(size_t* input_buffer_size, size_t* output_buffer_size) {
    if (!g_initialized) {
        if (input_buffer_size) *input_buffer_size = 0;
        if (output_buffer_size) *output_buffer_size = 0;
        return;
    }

    int frame_size = calculate_frame_size(g_config.sample_rate, g_config.frame_size_ms);

    if (input_buffer_size) {
        *input_buffer_size = frame_size * sizeof(int16_t) * g_config.channels;
    }

    if (output_buffer_size) {
        *output_buffer_size = (g_current_type == AUDIO_CODEC_TYPE_OPUS) ?
                             AUDIO_CODEC_MAX_PACKET_SIZE :
                             (frame_size * sizeof(int16_t) * g_config.channels);
    }
}

int audio_codec_health_check(void) {
    if (!g_initialized) {
        return AUDIO_CODEC_ERROR_NOT_INIT;
    }

    // Basic health check - try to encode/decode a small test frame
    int16_t test_input[160];  // 10ms at 16kHz
    uint8_t test_encoded[4000];
    int16_t test_output[160];
    size_t encoded_size;
    int decoded_samples;

    // Generate test signal
    for (int i = 0; i < 160; i++) {
        test_input[i] = (int16_t)(1000 * sin(2 * 3.14159 * 440 * i / 16000)); // 440Hz tone
    }

    // Test encoding
    int encode_result = audio_codec_encode(test_input, 160, test_encoded,
                                          sizeof(test_encoded), &encoded_size);
    if (encode_result != AUDIO_CODEC_OK) {
        return AUDIO_CODEC_ERROR_ENCODE;
    }

    // Test decoding
    int decode_result = audio_codec_decode(test_encoded, encoded_size, test_output,
                                          160, &decoded_samples);
    if (decode_result != AUDIO_CODEC_OK) {
        return AUDIO_CODEC_ERROR_DECODE;
    }

    return AUDIO_CODEC_OK;
}