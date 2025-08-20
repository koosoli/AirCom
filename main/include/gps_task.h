#ifndef GPS_TASK_H
#define GPS_TASK_H

#include "TinyGPS++.h"

// A structure to hold GPS data, making it easy to pass around
// This could be moved to a more central location if other tasks need it directly
struct GPSData {
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    double speed = 0.0;
    uint32_t satellites = 0;
    bool isValid = false;
    uint32_t date = 0; // YYYYMMDD
    uint32_t time = 0; // HHMMSSCC
};

// Task function for GPS processing
void gpsTask(void *pvParameters);

// Public function to get the latest GPS data safely
GPSData gps_get_data();

#endif // GPS_TASK_H
