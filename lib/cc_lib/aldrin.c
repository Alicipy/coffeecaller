#include "cc_lib/aldrin.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/pwm.h>

LOG_MODULE_REGISTER(cc_aldrin);

static uint8_t num_of_timer_repeats = 0;
static const struct pwm_dt_spec buzzer0 = PWM_DT_SPEC_GET(DT_ALIAS(buzzer0));

static void buzz_stop(struct k_timer *timer_id);
static void buzz_repeat(struct k_timer *timer_id);

K_TIMER_DEFINE(aldrin_buzzer_stop, buzz_stop, NULL);
K_TIMER_DEFINE(aldrin_buzzer_repeater, buzz_repeat, NULL);
// Private functions
void buzz_start(void)
{
	// Values determined via try and error
	pwm_set_dt(&buzzer0, PWM_HZ(200) / 2, PWM_HZ(4000) / 4);
}

static void buzz_stop(struct k_timer *timer_id)
{
	pwm_set_pulse_dt(&buzzer0, 0);
}

static void buzz_repeat(struct k_timer *timer_id)
{
	uint8_t *repeats = k_timer_user_data_get(&aldrin_buzzer_repeater);
	*repeats -= 1;
	if (*repeats == 0) {
		k_timer_stop(&aldrin_buzzer_repeater);
	} else {
		buzz_start();
		k_timer_start(&aldrin_buzzer_stop, K_MSEC(250), K_FOREVER);
	}
}

// Public functions

void aldrin_init(void)
{
	if (!pwm_is_ready_dt(&buzzer0)) {
		LOG_ERR("Error: PWM device %s is not ready\n", buzzer0.dev->name);
		return;
	}

	k_timer_user_data_set(&aldrin_buzzer_repeater, &num_of_timer_repeats);
}

void aldrin_buzz_long(void)
{
	buzz_start();
	k_timer_start(&aldrin_buzzer_stop, K_SECONDS(1), K_FOREVER);
}

void aldrin_buzz_short(void)
{
	buzz_start();
	k_timer_start(&aldrin_buzzer_stop, K_MSEC(250), K_FOREVER);
}

void aldrin_buzz_short_times(const uint8_t num_of_buzzes)
{
	num_of_timer_repeats = num_of_buzzes;
	buzz_start();
	k_timer_start(&aldrin_buzzer_stop, K_MSEC(250), K_FOREVER);
	k_timer_start(&aldrin_buzzer_repeater, K_MSEC(500), K_MSEC(500));
}

void aldrin_buzz_twice(void)
{
	aldrin_buzz_short_times(2);
}
