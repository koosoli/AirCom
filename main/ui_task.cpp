#include "include/ui_task.h"
#include "include/config.h"
#include "include/button_handler.h"
#include "include/shared_data.h"
#include "bt_audio.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "esp_timer.h"

// U8g2 C-style includes
#include "u8g2.h"
#include "u8g2_esp32_hal.h"
#include "AirCom.pb-c.h"
#include "crypto.h"
#include "HaLowMeshManager.h"

// Define the different states (screens) of the UI
typedef enum {
    UI_STATE_MAIN,
    UI_STATE_CONTACTS,
    UI_STATE_CHAT,
    UI_STATE_MAP,
    UI_STATE_BLUETOOTH,
    // Add other states like settings, etc.
} ui_state_t;

static u8g2_t u8g2; // a structure which contains all the data for one display
static ui_state_t current_ui_state = UI_STATE_MAIN;
static int selected_contact_index = 0;
static int selected_bt_menu_index = 0;
static std::string selected_contact_callsign = "";

// Text entry variables
static const char charset[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?";
static std::string current_message = "";
static int text_entry_cursor_pos = 0;
static int current_char_index = 0;

// Static variables to hold the data for the UI
static bool gps_lock_status = false;
static uint8_t team_contact_count = 0;
static std::vector<incoming_message_t> message_history;

// UI timing configuration for optimized responsiveness
#define UI_TARGET_FRAME_RATE 30  // Reduced from 50fps to 30fps for better performance
#define UI_FRAME_INTERVAL_MS (1000 / UI_TARGET_FRAME_RATE)
#define UI_MAX_FRAME_TIME_MS 50  // Maximum time allowed for one frame
#define UI_INPUT_PROCESSING_MS 2 // Dedicated time for input processing


// Placeholder drawing functions for each state
static void drawMainScreen() {
    char buf[30];
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(&u8g2, 0, 12, "Callsign: " CALLSIGN);

    sprintf(buf, "Teammates: %d", team_contact_count);
    u8g2_DrawStr(&u8g2, 0, 24, buf);

    sprintf(buf, "GPS: %s", gps_lock_status ? "Locked" : "No Lock");
    u8g2_DrawStr(&u8g2, 0, 36, buf);

    HaLowMeshManager& meshManager = HaLowMeshManager::getInstance();
    bool isConnected = meshManager.get_connection_status();
    sprintf(buf, "Status: %s", isConnected ? "Online" : "Offline");
    u8g2_DrawStr(&u8g2, 0, 48, buf);

    u8g2_DrawStr(&u8g2, 0, 60, "v Sel| ^ BT| < Status");
}

static void drawBluetoothScreen() {
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(&u8g2, 10, 10, "--- Bluetooth ---");

    // Menu item 0: Scan
    if (selected_bt_menu_index == 0) {
        u8g2_DrawStr(&u8g2, 0, 22, ">");
    }
    u8g2_DrawStr(&u8g2, 10, 22, "Scan for devices");

    // Discovered devices start from menu index 1
    const auto& devices = bt_audio_get_discovered_devices();
    for (size_t i = 0; i < devices.size(); ++i) {
        if (i + 1 == selected_bt_menu_index) {
            u8g2_DrawStr(&u8g2, 0, 34 + i * 12, ">");
        }
        u8g2_DrawStr(&u8g2, 10, 34 + i * 12, devices[i].name);
    }

    u8g2_DrawStr(&u8g2, 0, 60, "^ Back");
}


static void drawContactsScreen() {
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(&u8g2, 15, 10, "--- Contacts ---");

    if (xSemaphoreTake(g_contact_list_mutex, (TickType_t)10) == pdTRUE) {
        if (g_contact_list.empty()) {
            u8g2_DrawStr(&u8g2, 5, 36, "No contacts found");
        } else {
            for (size_t i = 0; i < g_contact_list.size(); ++i) {
                if (i == selected_contact_index) {
                    u8g2_DrawStr(&u8g2, 0, 22 + i * 12, ">");
                }
                u8g2_DrawStr(&u8g2, 10, 22 + i * 12, g_contact_list[i].callsign.c_str());
            }
        }
        xSemaphoreGive(g_contact_list_mutex);
    } else {
        u8g2_DrawStr(&u8g2, 5, 36, "Loading...");
    }

    u8g2_DrawStr(&u8g2, 0, 60, "^ Back");
}

static void drawChatScreen() {
    char buf[40];
    sprintf(buf, "To: %s", selected_contact_callsign.c_str());
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(&u8g2, 0, 10, buf);

    // Display message history
    for (size_t i = 0; i < message_history.size(); ++i) {
        u8g2_DrawStr(&u8g2, 0, 22 + i * 10, message_history[i].message_text.c_str());
    }

    // Draw the new message being composed
    u8g2_DrawStr(&u8g2, 0, 52, current_message.c_str());

    // Draw the cursor
    u8g2_DrawBox(&u8g2, text_entry_cursor_pos * 6, 54, 5, 2);

    u8g2_DrawStr(&u8g2, 0, 64, "^ Back | Send (L)");
}

static void drawMapScreen() {
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(&u8g2, 20, 10, "--- Tactical Map ---");
    u8g2_DrawDisc(&u8g2, 64, 32, 2, U8G2_DRAW_ALL); // User in the center
    u8g2_DrawStr(&u8g2, 58, 48, "You");

    if (xSemaphoreTake(g_teammate_locations_mutex, (TickType_t)10) == pdTRUE) {
        GPSData my_location = gps_get_data();
        if (my_location.isValid) {
            for (const auto& teammate : g_teammate_locations) {
                // This is a very simplified projection. A real implementation would use a proper map projection.
                // It calculates a simple relative vector and scales it to the screen.
                double delta_lon = teammate.lon - my_location.longitude;
                double delta_lat = teammate.lat - my_location.latitude;

                // Simple scaling - this needs to be calibrated for a real-world scale
                int x = 64 + (int)(delta_lon * 50000);
                int y = 32 - (int)(delta_lat * 50000);

                // Clamp to screen edges
                if (x < 0) x = 0; if (x > 127) x = 127;
                if (y < 12) y = 12; if (y > 63) y = 63;

                u8g2_DrawStr(&u8g2, x, y, teammate.callsign.c_str());
            }
        }
        xSemaphoreGive(g_teammate_locations_mutex);
    }

    u8g2_DrawStr(&u8g2, 0, 64, "^ Back");
}


void uiTask(void *pvParameters) {
    ESP_LOGI(TAG, "uiTask started");

    // Initialize buttons
    buttons_init();

    // 1. Initialize the U8g2 HAL
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.bus.i2c.sda = PIN_OLED_SDA;
    u8g2_esp32_hal.bus.i2c.scl = PIN_OLED_SCL;
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    // 2. Choose the appropriate setup function for the display
    // For a 128x64 SH1106 display with I2C
    u8g2_Setup_sh1106_i2c_128x64_noname_f(
        &u8g2,
        U8G2_R0,
        u8g2_esp32_i2c_byte_cb,
        u8g2_esp32_gpio_and_delay_cb);

    // The I2C address is stored in the u8x8 part of the u8g2 struct
    u8x8_SetI2CAddress(&u8g2.u8x8, 0x78); // 0x3C << 1

    // 3. Initialize the display
    u8g2_InitDisplay(&u8g2);

    // 4. Wake up the display
    u8g2_SetPowerSave(&u8g2, 0);

    ESP_LOGI(TAG, "Display initialized successfully.");

    // Performance monitoring variables
    uint32_t frame_count = 0;
    uint64_t last_frame_time = esp_timer_get_time();
    bool force_redraw = true; // Force initial draw

    // Main UI loop with optimized timing and responsiveness
    for (;;) {
        uint64_t frame_start_time = esp_timer_get_time();

        // Phase 1: High-priority input processing and critical updates
        uint64_t input_start = esp_timer_get_time();

        // Check for updates from other tasks (non-blocking)
        ui_update_t update;
        if (xQueueReceive(ui_update_queue, &update, (TickType_t)0) == pdPASS) {
            if (update.contact_count != 0xFF) {
                team_contact_count = update.contact_count;
                force_redraw = true; // Changed data requires redraw
            }
            if (update.has_gps_lock != 0xFF) {
                gps_lock_status = update.has_gps_lock;
                force_redraw = true; // Changed data requires redraw
            }
        }

        incoming_message_t incoming_msg;
        if (xQueueReceive(incoming_message_queue, &incoming_msg, (TickType_t)0) == pdPASS) {
            message_history.push_back(incoming_msg);
            if (message_history.size() > 4) {
                message_history.erase(message_history.begin());
            }
            force_redraw = true; // New message requires redraw
        }

        // Process button inputs with high priority
        buttons_read();

        // Critical buttons (PTT) get immediate processing
        if (is_button_just_pressed(BUTTON_PTT)) {
            ESP_LOGI(TAG, "PTT Pressed - Start TX");
            audio_command_t cmd = AUDIO_CMD_START_TX;
            xQueueSend(audio_command_queue, &cmd, portMAX_DELAY); // Blocking send for critical commands
            taskYIELD(); // Yield immediately after critical command
        }
        if (is_button_just_released(BUTTON_PTT)) {
            ESP_LOGI(TAG, "PTT Released - Stop TX");
            audio_command_t cmd = AUDIO_CMD_STOP_TX;
            xQueueSend(audio_command_queue, &cmd, portMAX_DELAY); // Blocking send for critical commands
            taskYIELD(); // Yield immediately after critical command
        }

        // Process other button inputs
        bool input_processed = false;
        if (is_button_just_pressed(BUTTON_UP) || is_button_just_pressed(BUTTON_DOWN) ||
            is_button_just_pressed(BUTTON_SELECT) || is_button_just_pressed(BUTTON_BACK)) {

            // Update UI state based on button presses
            switch (current_ui_state) {
                case UI_STATE_MAIN:
                    if (is_button_just_pressed(BUTTON_SELECT)) {
                        current_ui_state = UI_STATE_CONTACTS;
                        input_processed = true;
                    }
                    if (is_button_just_pressed(BUTTON_UP)) {
                        current_ui_state = UI_STATE_BLUETOOTH;
                        input_processed = true;
                    }
                    if (is_button_just_pressed(BUTTON_BACK)) {
                        HaLowMeshManager& meshManager = HaLowMeshManager::getInstance();
                        bool currentStatus = meshManager.get_connection_status();
                        meshManager.setConnectionStatus(!currentStatus);
                        if (!currentStatus) {
                            meshManager.sendCachedMessages();
                        }
                        input_processed = true;
                    }
                    break;

                case UI_STATE_MAP:
                    if (is_button_just_pressed(BUTTON_BACK)) {
                        current_ui_state = UI_STATE_MAIN;
                        input_processed = true;
                    }
                    break;

                case UI_STATE_BLUETOOTH:
                    if (is_button_just_pressed(BUTTON_BACK)) {
                        current_ui_state = UI_STATE_MAIN;
                        input_processed = true;
                    }
                    if (is_button_just_pressed(BUTTON_UP)) {
                        if (selected_bt_menu_index > 0) selected_bt_menu_index--;
                        input_processed = true;
                    }
                    if (is_button_just_pressed(BUTTON_DOWN)) {
                        const auto& devices = bt_audio_get_discovered_devices();
                        if (selected_bt_menu_index < devices.size()) selected_bt_menu_index++;
                        input_processed = true;
                    }
                    if (is_button_just_pressed(BUTTON_SELECT)) {
                        if (selected_bt_menu_index == 0) {
                            bt_audio_start_discovery();
                        } else {
                            const auto& devices = bt_audio_get_discovered_devices();
                            int device_index = selected_bt_menu_index - 1;
                            if (device_index < devices.size()) {
                                bt_audio_connect(devices[device_index].bda);
                            }
                        }
                        input_processed = true;
                    }
                    break;

                case UI_STATE_CONTACTS:
                    if (is_button_just_pressed(BUTTON_BACK)) {
                        current_ui_state = UI_STATE_MAIN;
                        input_processed = true;
                    }
                    if (is_button_just_pressed(BUTTON_UP)) {
                        if (selected_contact_index > 0) selected_contact_index--;
                        input_processed = true;
                    }
                    if (is_button_just_pressed(BUTTON_DOWN)) {
                        if (xSemaphoreTake(g_contact_list_mutex, (TickType_t)5) == pdTRUE) {
                            if (selected_contact_index < g_contact_list.size() - 1) selected_contact_index++;
                            xSemaphoreGive(g_contact_list_mutex);
                        }
                        input_processed = true;
                    }
                    if (is_button_just_pressed(BUTTON_SELECT)) {
                        if (xSemaphoreTake(g_contact_list_mutex, (TickType_t)5) == pdTRUE) {
                            if (!g_contact_list.empty() && (size_t)selected_contact_index < g_contact_list.size()) {
                                selected_contact_callsign = g_contact_list[selected_contact_index].callsign;
                                current_ui_state = UI_STATE_CHAT;
                            }
                            xSemaphoreGive(g_contact_list_mutex);
                        }
                        input_processed = true;
                    }
                    break;

                case UI_STATE_CHAT:
                    if (is_button_just_pressed(BUTTON_BACK)) {
                        if (current_message.length() > 0) {
                            current_message.pop_back();
                            text_entry_cursor_pos--;
                        } else {
                            current_ui_state = UI_STATE_CONTACTS;
                        }
                        input_processed = true;
                    }
                    if (is_button_just_pressed(BUTTON_UP)) {
                        current_char_index++;
                        if (current_char_index >= sizeof(charset) - 1) current_char_index = 0;
                        if (text_entry_cursor_pos < current_message.length()) {
                            current_message[text_entry_cursor_pos] = charset[current_char_index];
                        }
                        input_processed = true;
                    }
                    if (is_button_just_pressed(BUTTON_DOWN)) {
                        current_char_index--;
                        if (current_char_index < 0) current_char_index = sizeof(charset) - 2;
                        if (text_entry_cursor_pos < current_message.length()) {
                            current_message[text_entry_cursor_pos] = charset[current_char_index];
                        }
                        input_processed = true;
                    }
                    if (is_button_just_pressed(BUTTON_SELECT)) {
                        text_entry_cursor_pos++;
                        if (text_entry_cursor_pos > current_message.length()) {
                            current_message += " ";
                        }
                        current_char_index = 0;
                        input_processed = true;
                    }
                    if (is_button_long_pressed(BUTTON_SELECT)) {
                        // Send message logic (same as before)
                        AirComPacket packet = AIR_COM_PACKET__INIT;
                        TextMessage text_msg = TEXT_MESSAGE__INIT;
                        text_msg.text = (char*)current_message.c_str();
                        packet.payload_variant_case = AIR_COM_PACKET__PAYLOAD_VARIANT_TEXT_MESSAGE;
                        packet.text_message = &text_msg;

                        size_t packed_size = air_com_packet__get_packed_size(&packet);
                        uint8_t *buffer = (uint8_t *)malloc(packed_size);
                        if (buffer) {
                            air_com_packet__pack(&packet, buffer);
                            std::vector<uint8_t> encrypted_payload = encrypt_message(std::string((char*)buffer, packed_size));
                            free(buffer);

                            outgoing_message_t out_msg;
                            if (xSemaphoreTake(g_contact_list_mutex, (TickType_t)10) == pdTRUE) {
                                if (!g_contact_list.empty() && (size_t)selected_contact_index < g_contact_list.size()) {
                                    strncpy(out_msg.target_ip, g_contact_list[selected_contact_index].ipAddress.c_str(), sizeof(out_msg.target_ip) - 1);
                                    out_msg.encrypted_payload = encrypted_payload;
                                    xQueueSend(outgoing_message_queue, &out_msg, (TickType_t)0);
                                }
                                xSemaphoreGive(g_contact_list_mutex);
                            }

                            current_message = "";
                            text_entry_cursor_pos = 0;
                            current_ui_state = UI_STATE_CONTACTS;
                        }
                        input_processed = true;
                    }
                    break;
            }
        }

        uint64_t input_time = esp_timer_get_time() - input_start;
        if (input_time > (UI_INPUT_PROCESSING_MS * 1000)) {
            ESP_LOGD(TAG, "Input processing took %llu us", input_time);
        }

        // Phase 2: Conditional screen drawing
        bool should_draw = force_redraw || input_processed;
        if (should_draw) {
            uint64_t draw_start = esp_timer_get_time();

            u8g2_FirstPage(&u8g2);
            do {
                switch (current_ui_state) {
                    case UI_STATE_MAIN:
                        drawMainScreen();
                        break;
                    case UI_STATE_CONTACTS:
                        drawContactsScreen();
                        break;
                    case UI_STATE_CHAT:
                        drawChatScreen();
                        break;
                    case UI_STATE_MAP:
                        drawMapScreen();
                        break;
                    case UI_STATE_BLUETOOTH:
                        drawBluetoothScreen();
                        break;
                }
            } while (u8g2_NextPage(&u8g2));

            uint64_t draw_time = esp_timer_get_time() - draw_start;
            if (draw_time > (UI_MAX_FRAME_TIME_MS * 1000)) {
                ESP_LOGW(TAG, "UI drawing took too long: %llu us", draw_time);
            }

            force_redraw = false;
            frame_count++;
        }

        // Phase 3: Frame timing and system responsiveness
        uint64_t frame_time = esp_timer_get_time() - frame_start_time;
        uint64_t target_frame_time = UI_FRAME_INTERVAL_MS * 1000;

        if (frame_time < target_frame_time) {
            uint32_t sleep_ticks = pdUS_TO_TICKS(target_frame_time - frame_time);
            if (sleep_ticks > 0) {
                vTaskDelay(sleep_ticks);
            }
        } else {
            // Frame took too long, yield to other tasks
            ESP_LOGD(TAG, "UI frame overrun: %llu us", frame_time);
            taskYIELD();
        }

        // Yield periodically to maintain system responsiveness
        if (frame_count % 5 == 0) {
            taskYIELD();
        }

        // Performance monitoring
        if (frame_count % 100 == 0) {
            uint64_t now = esp_timer_get_time();
            uint64_t elapsed = now - last_frame_time;
            float fps = 100.0f / (elapsed / 1000000.0f);
            ESP_LOGI(TAG, "UI Performance: %.1f fps, avg frame time: %llu us", fps, elapsed / 100);
            last_frame_time = now;
        }
    }
}
