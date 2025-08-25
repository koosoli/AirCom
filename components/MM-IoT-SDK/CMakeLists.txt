# MM-IoT-SDK Component CMakeLists.txt

idf_component_register(
    SRCS
        "src/mm_iot_sdk.cpp"

    INCLUDE_DIRS
        "include"
        "../../main/include"

    REQUIRES
        "driver"
        "esp_wifi"
        "esp_netif"
        "esp_event"
        "nvs_flash"
        "lwip"
        "log"
)

# Add compile definitions for MM-IoT-SDK
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    CONFIG_MORSEMICRO_HALOW=1
    CONFIG_SPI_MASTER=1
    CONFIG_I2C_MASTER=1
    CONFIG_UART_MASTER=1
)

# Set C++ standard to C++17 for modern features
target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_17)

# Add additional include directories if needed
# target_include_directories(${COMPONENT_LIB} PRIVATE
#     "${CMAKE_CURRENT_SOURCE_DIR}/path/to/mm-iot-sdk/include"
# )

# Link against MM-IoT-SDK library if available
# target_link_libraries(${COMPONENT_LIB} PRIVATE mm_iot_sdk)

# Add any additional compiler options for Wi-Fi HaLow
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-unused-variable
    -Wno-unused-parameter
    -Wno-format
)