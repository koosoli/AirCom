#ifndef CAMERA_SERVICE_H
#define CAMERA_SERVICE_H

// Camera Service - Stub Implementation
// This is a placeholder for camera functionality

/**
 * @brief Initialize the camera service
 * @return 0 on success, error code on failure
 */
int camera_service_init(void);

/**
 * @brief Start camera streaming
 * @return 0 on success, error code on failure
 */
int camera_service_start_stream(void);

/**
 * @brief Stop camera streaming
 * @return 0 on success, error code on failure
 */
int camera_service_stop_stream(void);

/**
 * @brief Capture a single image
 * @return 0 on success, error code on failure
 */
int camera_service_capture_image(void);

#endif // CAMERA_SERVICE_H