#include "include/button_handler.h"
#include "include/config.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DEBOUNCE_TIME_MS 50
#define LONG_PRESS_TIME_MS 1000

// Array to hold the pin number for each button
static const gpio_num_t button_pins[NUM_BUTTONS] = {
    (gpio_num_t)PIN_BUTTON_PTT,
    (gpio_num_t)PIN_BUTTON_UP,
    (gpio_num_t)PIN_BUTTON_DOWN,
    (gpio_num_t)PIN_BUTTON_SELECT,
    (gpio_num_t)PIN_BUTTON_BACK
};

// Internal state for each button
static bool button_state[NUM_BUTTONS];
static bool last_button_state[NUM_BUTTONS];
static bool just_pressed_flag[NUM_BUTTONS];
static bool just_released_flag[NUM_BUTTONS];
static bool long_press_flag[NUM_BUTTONS];
static uint32_t last_debounce_time[NUM_BUTTONS];
static uint32_t press_start_time[NUM_BUTTONS];


void buttons_init() {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << button_pins[i]);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        gpio_config(&io_conf);

        // Initialize states
        button_state[i] = true; // Assume released (high due to pull-up)
        last_button_state[i] = true;
        just_pressed_flag[i] = false;
        just_released_flag[i] = false;
        long_press_flag[i] = false;
        last_debounce_time[i] = 0;
        press_start_time[i] = 0;
    }
}

void buttons_read() {
    uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());

    for (int i = 0; i < NUM_BUTTONS; i++) {
        // Reset the "just actioned" flags after one cycle
        just_pressed_flag[i] = false;
        just_released_flag[i] = false;
        long_press_flag[i] = false;

        bool reading = gpio_get_level(button_pins[i]);

        // If the switch changed, due to noise or pressing:
        if (reading != last_button_state[i]) {
            // reset the debouncing timer
            last_debounce_time[i] = now;
        }

        if ((now - last_debounce_time[i]) > DEBOUNCE_TIME_MS) {
            // If the button state has changed:
            if (reading != button_state[i]) {
                button_state[i] = reading;

                if (button_state[i] == false) { // Button was just pressed
                    just_pressed_flag[i] = true;
                    press_start_time[i] = now;
                } else { // Button was just released
                    just_released_flag[i] = true;
                }
            }
        }

        // Check for long press if the button is currently held down
        if (button_state[i] == false) {
            if ((now - press_start_time[i]) > LONG_PRESS_TIME_MS) {
                long_press_flag[i] = true;
                // To prevent this from re-triggering, you could optionally reset press_start_time here
                // press_start_time[i] = now;
            }
        }

        last_button_state[i] = reading;
    }
}

bool is_button_pressed(button_id_t button) {
    if (button >= NUM_BUTTONS) return false;
    // Active low, so pressed is false
    return !button_state[button];
}

bool is_button_just_pressed(button_id_t button) {
    if (button >= NUM_BUTTONS) return false;
    return just_pressed_flag[button];
}

bool is_button_long_pressed(button_id_t button) {
    if (button >= NUM_BUTTONS) return false;
    return long_press_flag[button];
}

bool is_button_just_released(button_id_t button) {
    if (button >= NUM_BUTTONS) return false;
    return just_released_flag[button];
}
