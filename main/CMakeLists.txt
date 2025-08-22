# AirCom Main Application CMakeLists.txt

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
        bt_audio
        MM-IoT-SDK
        Heltec-HaLow
        HaLowManager
        libsodium
        Opus
        TinyGPSxx
        u8g2
        u8g2_esp32_hal
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
