#pragma once

#include <zephyr/drivers/led_strip.h>

#define LEDS_RGB(_r, _g, _b)                                                                       \
	{                                                                                          \
		.r = (_r), .g = (_g), .b = (_b)                                                    \
	}

typedef struct led_rgb leds_color;

/**
 * @brief Initializes the strip from devicetree
 *
 */
void leds_init(void);

/**
 * @brief Sets given color for all pixels on the strip
 *
 * @param new_color LEDS_RGB value of the new color
 */
void leds_set_rgb_all(const leds_color* new_color);

/**
 * @brief Sets given color for one pixel on the strip
 *
 * @param new_color LEDS_RGB value of the new color
 * @param pos Position of the pixel to set starting at 0
 */
void leds_set_rgb_at_pos(const leds_color* new_color, uint8_t pos);

/**
 * @brief Sets all pixels on the strip to off
 *
 */
void leds_set_all_off(void);

/**
 * @brief Retrieve the number of pixels on the strip
 *
 * @return uint8_t number of pixels
 */
uint8_t leds_get_num_of_pixels(void);
