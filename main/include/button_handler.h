#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

// Enum to identify each button clearly
typedef enum {
    BUTTON_PTT,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_SELECT,
    BUTTON_BACK,
    NUM_BUTTONS // A count of the number of buttons
} button_id_t;

/**
 * @brief Initializes the GPIO pins for all buttons.
 *
 * This function configures the button pins as inputs with internal pull-ups.
 * It must be called once before any other button functions are used.
 */
void buttons_init();

/**
 * @brief Reads the state of all buttons and handles debouncing.
 *
 * This function should be called periodically in a task loop (e.g., every 10-20ms).
 * It updates the internal state of all buttons.
 */
void buttons_read();

/**
 * @brief Checks if a specific button is currently being held down.
 *
 * @param button The ID of the button to check.
 * @return true if the button is pressed, false otherwise.
 */
bool is_button_pressed(button_id_t button);

/**
 * @brief Checks if a specific button was just pressed in the latest scan.
 *
 * This is useful for single-press events, as it only returns true for one
 * cycle of the task loop after the button is pressed.
 *
 * @param button The ID of the button to check.
 * @return true if the button was just pressed, false otherwise.
 */
bool is_button_just_pressed(button_id_t button);

/**
 * @brief Checks if a specific button is being held down for a long press.
 *
 * @param button The ID of the button to check.
 * @return true if a long press is detected, false otherwise.
 */
bool is_button_long_pressed(button_id_t button);

/**
 * @brief Checks if a specific button was just released in the latest scan.
 *
 * @param button The ID of the button to check.
 * @return true if the button was just released, false otherwise.
 */
bool is_button_just_released(button_id_t button);


#endif // BUTTON_HANDLER_H
