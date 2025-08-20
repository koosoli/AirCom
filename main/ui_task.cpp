#include "include/ui_task.h"
#include "include/config.h"
#include "include/button_handler.h"
#include "include/shared_data.h"
#include "esp_log.h"

// U8g2 C-style includes
#include "u8g2.h"
#include "u8g2_esp32_hal.h"
#include "AirCom.pb-c.h"
#include "crypto.h"

// Define the different states (screens) of the UI
typedef enum {
    UI_STATE_MAIN,
    UI_STATE_CONTACTS,
    UI_STATE_CHAT,
    // Add other states like settings, etc.
} ui_state_t;

static u8g2_t u8g2; // a structure which contains all the data for one display
static ui_state_t current_ui_state = UI_STATE_MAIN;
static int selected_contact_index = 0;
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


// Placeholder drawing functions for each state
static void drawMainScreen() {
    char buf[20];
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(&u8g2, 0, 12, "Callsign: " CALLSIGN);

    sprintf(buf, "Teammates: %d", team_contact_count);
    u8g2_DrawStr(&u8g2, 0, 24, buf);

    sprintf(buf, "GPS: %s", gps_lock_status ? "Locked" : "No Lock");
    u8g2_DrawStr(&u8g2, 0, 36, buf);

    u8g2_DrawStr(&u8g2, 0, 48, "Status: Online");
    u8g2_DrawStr(&u8g2, 0, 60, "v Select for Contacts");
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

    // Main UI loop
    for (;;) {
        // 0. Check for updates from other tasks
        ui_update_t update;
        if (xQueueReceive(ui_update_queue, &update, (TickType_t)0) == pdPASS) {
            if (update.contact_count != 0xFF) {
                team_contact_count = update.contact_count;
            }
            if (update.has_gps_lock != 0xFF) { // Assuming we can use 0xFF for bool too
                gps_lock_status = update.has_gps_lock;
            }
        }
        incoming_message_t incoming_msg;
        if (xQueueReceive(incoming_message_queue, &incoming_msg, (TickType_t)0) == pdPASS) {
            // For now, just add to history. A real implementation would check if the
            // sender is the currently selected contact.
            message_history.push_back(incoming_msg);
            if (message_history.size() > 4) { // Keep only the last 4 messages
                message_history.erase(message_history.begin());
            }
        }


        // 1. Read button states
        buttons_read();

        if (is_button_just_pressed(BUTTON_UP)) ESP_LOGI(TAG, "Up button pressed");
        if (is_button_just_pressed(BUTTON_DOWN)) ESP_LOGI(TAG, "Down button pressed");
        if (is_button_just_pressed(BUTTON_SELECT)) ESP_LOGI(TAG, "Select button pressed");
        if (is_button_just_pressed(BUTTON_BACK)) ESP_LOGI(TAG, "Back button pressed");

        if (is_button_just_pressed(BUTTON_PTT)) {
            ESP_LOGI(TAG, "PTT Pressed - Start TX");
            audio_command_t cmd = AUDIO_CMD_START_TX;
            xQueueSend(audio_command_queue, &cmd, 0);
        }
        if (is_button_just_released(BUTTON_PTT)) {
            ESP_LOGI(TAG, "PTT Released - Stop TX");
            audio_command_t cmd = AUDIO_CMD_STOP_TX;
            xQueueSend(audio_command_queue, &cmd, 0);
        }


        // 2. Update UI State
        switch (current_ui_state) {
            case UI_STATE_MAIN:
                if (is_button_just_pressed(BUTTON_SELECT)) {
                    current_ui_state = UI_STATE_CONTACTS;
                }
                break;
            case UI_STATE_CONTACTS:
                if (is_button_just_pressed(BUTTON_BACK)) {
                    current_ui_state = UI_STATE_MAIN;
                }
                if (is_button_just_pressed(BUTTON_UP)) {
                    if (selected_contact_index > 0) {
                        selected_contact_index--;
                    }
                }
                if (is_button_just_pressed(BUTTON_DOWN)) {
                    if (xSemaphoreTake(g_contact_list_mutex, (TickType_t)10) == pdTRUE) {
                        if (selected_contact_index < g_contact_list.size() - 1) {
                            selected_contact_index++;
                        }
                        xSemaphoreGive(g_contact_list_mutex);
                    }
                }
                if (is_button_just_pressed(BUTTON_SELECT)) {
                    if (xSemaphoreTake(g_contact_list_mutex, (TickType_t)10) == pdTRUE) {
                        if (!g_contact_list.empty() && (size_t)selected_contact_index < g_contact_list.size()) {
                            selected_contact_callsign = g_contact_list[selected_contact_index].callsign;
                            current_ui_state = UI_STATE_CHAT;
                        }
                        xSemaphoreGive(g_contact_list_mutex);
                    }
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
                }
                if (is_button_just_pressed(BUTTON_UP)) {
                    current_char_index++;
                    if (current_char_index >= sizeof(charset) - 1) current_char_index = 0;
                    current_message[text_entry_cursor_pos] = charset[current_char_index];
                }
                if (is_button_just_pressed(BUTTON_DOWN)) {
                    current_char_index--;
                    if (current_char_index < 0) current_char_index = sizeof(charset) - 2;
                    current_message[text_entry_cursor_pos] = charset[current_char_index];
                }
                if (is_button_just_pressed(BUTTON_SELECT)) {
                    text_entry_cursor_pos++;
                    current_message += " "; // Add a new character placeholder
                    current_char_index = 0;
                }
                if (is_button_long_pressed(BUTTON_SELECT)) {
                    ESP_LOGI(TAG, "Queueing message for sending: %s", current_message.c_str());

                    // 1. Create protobuf message
                    AirComPacket packet = AIR_COM_PACKET__INIT;
                    TextMessage text_msg = TEXT_MESSAGE__INIT;
                    text_msg.text = (char*)current_message.c_str();
                    packet.payload_variant_case = AIR_COM_PACKET__PAYLOAD_VARIANT_TEXT_MESSAGE;
                    packet.text_message = &text_msg;
                    // TODO: Set packet_id, from_node, to_node

                    // 2. Serialize and encrypt
                    size_t packed_size = air_com_packet__get_packed_size(&packet);
                    uint8_t *buffer = (uint8_t *)malloc(packed_size);
                    air_com_packet__pack(&packet, buffer);
                    std::vector<uint8_t> encrypted_payload = encrypt_message(std::string((char*)buffer, packed_size));
                    free(buffer);

                    // 3. Create outgoing message and queue it
                    outgoing_message_t out_msg;
                    strncpy(out_msg.target_ip, g_contact_list[selected_contact_index].ipAddress.c_str(), sizeof(out_msg.target_ip) - 1);
                    out_msg.encrypted_payload = encrypted_payload;
                    xQueueSend(outgoing_message_queue, &out_msg, (TickType_t)0);

                    // 4. Reset UI
                    current_message = "";
                    text_entry_cursor_pos = 0;
                    current_ui_state = UI_STATE_CONTACTS;
                }
                break;
        }


        // 3. Draw screen
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
            }
        } while (u8g2_NextPage(&u8g2));

        vTaskDelay(pdMS_TO_TICKS(20)); // ~50fps loop
    }
}
