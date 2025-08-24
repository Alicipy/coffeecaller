#pragma once

#include <stdint.h>

typedef void (*button_callback)(void);

typedef struct {
	button_callback on_pressed;
	button_callback on_released;
} button_state_callbacks;

/**
 * @brief Initializes buttons from devicetree
 *
 */
void button_init(void);

/**
 * @brief Registers callbacks for a given button
 *
 * @param callbacks Struct holding the callbacks for certain events
 * @param button_idx Button to set the callbacks for
 */
void button_set_callback_for_button(button_state_callbacks *callbacks, uint8_t button_idx);
