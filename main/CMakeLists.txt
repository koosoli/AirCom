# AirCom Main Application CMakeLists.txt

# Add the PlatformIO library dependency directory to the ESP-IDF component search path.
# This allows ESP-IDF to find components downloaded via PlatformIO's `lib_deps`.
set(EXTRA_COMPONENT_DIRS $ENV{PIO_LIBDEPS_DIR})

# Manually include the protobuf-c component's cmake file to ensure
# the protobuf_c_generate function is defined before we call it.
# This is a workaround for a potential build order issue with PlatformIO.
include($ENV{IDF_FRAMEWORK_DIR}/components/protobuf-c/component.cmake)

# Generate C sources from the .proto file.
# This must be done before registering the component.
protobuf_c_generate(
    PROTO_FILES proto/AirCom.proto
    COMPONENT main
)

idf_component_register(
    SRCS
        "main.cpp"
        "ui_task.cpp"
        "audio_task.cpp"
        "network_task.cpp"
        "gps_task.cpp"
        "crypto.cpp"
        "button_handler.cpp"
        "shared_data.cpp"
        "safe_callback.cpp"
        "memory_tracker.cpp"
        "config_manager.cpp"
        "logging_system.cpp"
        "gui_tester.cpp"
        "gui_preview.cpp"
        "error_handling.c"
        "network_utils.c"
        "xiao_integration_test.cpp"
        "atak_processor_task.cpp"
        "atak_task.cpp"

    INCLUDE_DIRS
        "include"
        "../components"

    REQUIRES
        bt
        bt_audio
        HaLowManager
        libsodium
        Opus
        TinyGPSxx
        u8g2
        u8g2_esp32_hal
        protobuf-c
)

# Add GUI Preview as standalone executable (for testing)
if(GUI_PREVIEW_STANDALONE)
    add_executable(gui_preview_standalone
        "gui_preview.cpp"
    )

    target_include_directories(gui_preview_standalone PRIVATE
        "include"
    )

    # Link standard C++ libraries
    target_link_libraries(gui_preview_standalone
        stdc++
        pthread
    )
endif()
