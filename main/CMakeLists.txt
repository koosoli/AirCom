# CMakeLists.txt for the main application component

# The component is a library
idf_component_register(SRCS "main.cpp" "gps_task.cpp" "atak_task.cpp" "ui_task.cpp" "audio_task.cpp" "button_handler.cpp" "shared_data.cpp" "crypto.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES TinyGPSxx HaLowManager u8g2_esp32_hal Opus protobuf-c)

# Find all .proto files in the 'proto' subdirectory
file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/proto/*.proto")

# Add a command to compile the .proto files to C source/header files
# This will generate AirCom.pb-c.h and AirCom.pb-c.c in the build directory
idf_proto_compile_c(${PROTO_FILES} PROTO_SRCS PROTO_HDRS)

# Add the generated source files to the component's source list
list(APPEND SRCS ${PROTO_SRCS})
