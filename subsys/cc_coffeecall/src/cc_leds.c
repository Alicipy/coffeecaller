#include <zephyr/kernel.h>

#include "cc_leds.h"
#include "cc_lib/leds.h"

static leds_color red = LEDS_RGB(0x0F, 0x00, 0x00);
static leds_color yellow = LEDS_RGB(0x0F, 0x0F, 0x00);
static leds_color green = LEDS_RGB(0x00, 0x0F, 0x00);
static leds_color blue = LEDS_RGB(0x00, 0x00, 0x0F);

static void timeout_led_off(struct k_timer* timer_id)
{
    cc_leds_all_off();
}

K_TIMER_DEFINE(led_off_timer, timeout_led_off, NULL);

static void start_led_off_timer()
{
    k_timer_start(&led_off_timer, K_SECONDS(5), K_FOREVER);
}

int cc_leds_coffee_call_successful()
{
    leds_set_rgb_all(&green);
    start_led_off_timer();
    return 0;
}

int cc_leds_coffee_call_failed()
{
    leds_set_rgb_all(&red);
    start_led_off_timer();
    return 0;
}

int cc_leds_all_off()
{
    leds_set_all_off();
    return 0;
}

int cc_leds_show_number_of_participants(int participant_count)
{
    leds_set_rgb_all(&yellow);
    for (uint8_t idx = 0; idx < participant_count && idx < leds_get_num_of_pixels(); ++idx)
    {
        k_sleep(K_MSEC(100));
        leds_set_rgb_at_pos(&blue, idx);
    }
    return 0;
}
