#pragma once

#include <stdint.h>

/**
 * @brief Initializes the buzzer from devicetree
 *
 */
void aldrin_init(void);

/**
 * @brief One second buzz
 *
 */
void aldrin_buzz_long(void);

/**
 * @brief 250 ms buzz
 *
 */
void aldrin_buzz_short(void);

/**
 * @brief Repeats short buzz
 *
 * @param num_of_buzzes How often it should repeat
 */
void aldrin_buzz_short_times(uint8_t num_of_buzzes);

/**
 * @brief Shortcut to short buzz twice
 *
 */
void aldrin_buzz_twice(void);
