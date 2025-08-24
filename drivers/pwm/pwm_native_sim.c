#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/util.h>

#define DT_DRV_COMPAT zephyr_pwm_native_sim

/** Fake PWM config structure */
struct fake_pwm_config {
	uint64_t frequency_hz;
};

static int fake_pwm_set_cycles(const struct device *dev, uint32_t channel,
				uint32_t period_cycles, uint32_t pulse_cycles,
				pwm_flags_t flags)
{
	return 0;
}


static int fake_pwm_get_cycles_per_sec(const struct device *dev, uint32_t channel, uint64_t *cycles)
{
	ARG_UNUSED(channel);
	const struct fake_pwm_config *config = dev->config;

	*cycles = config->frequency_hz;

	return 0;
}

static DEVICE_API(pwm, fake_pwm_driver_api) = {
	.set_cycles = fake_pwm_set_cycles,
	.get_cycles_per_sec = fake_pwm_get_cycles_per_sec,
};

#define FAKE_PWM_INIT(inst)                                                                        \
	static const struct fake_pwm_config fake_pwm_config_##inst = {                             \
		.frequency_hz = DT_INST_PROP(inst, frequency),                                     \
	};                                                                                         \
                                                                                                   \
	DEVICE_DT_INST_DEFINE(inst, NULL, NULL, NULL, &fake_pwm_config_##inst, POST_KERNEL,        \
			      CONFIG_PWM_INIT_PRIORITY, &fake_pwm_driver_api);

DT_INST_FOREACH_STATUS_OKAY(FAKE_PWM_INIT)
