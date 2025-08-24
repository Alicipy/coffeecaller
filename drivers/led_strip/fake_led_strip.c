/*
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT zephyr_fake_led_strip

#include <zephyr/drivers/led_strip.h>

#define LOG_LEVEL CONFIG_LED_STRIP_LOG_LEVEL

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(fake_led_strip);

#include <zephyr/kernel.h>
#include <zephyr/device.h>

struct fake_led_strip_cfg
{
    struct led_rgb* current_pixel_colors;
    size_t length;
};

static const struct fake_led_strip_cfg* dev_cfg(const struct device* dev)
{
    return dev->config;
}

static int fake_strip_update_rgb(const struct device* dev,
                                 struct led_rgb* pixels,
                                 size_t num_pixels)
{
    const struct fake_led_strip_cfg* cfg = dev_cfg(dev);

    if (num_pixels > cfg->length)
    {
        return -EINVAL;
    }

    for (int i = 0; i < num_pixels; i++)
    {
        struct led_rgb* pixel = &cfg->current_pixel_colors[i];
        pixel->r = pixels[i].r;
        pixel->g = pixels[i].g;
        pixel->b = pixels[i].b;
    }

    return 0;
}

static size_t fake_strip_length(const struct device* dev)
{
    const struct fake_led_strip_cfg* cfg = dev_cfg(dev);

    return cfg->length;
}

static DEVICE_API(led_strip, fake_led_strip_api) = {
    .update_rgb = fake_strip_update_rgb,
    .length = fake_strip_length,
};

#define FAKE_LED_NUM_PIXELS(idx) (DT_INST_PROP(idx, chain_length))

#define FAKE_LED_STRIP_DEVICE(idx)						 \
									 \
	static struct led_rgb fake_led_strip##idx##_buf[FAKE_LED_NUM_PIXELS(idx)]; \
									 \
	static const struct fake_led_strip_cfg fake_led_strip##idx##_cfg = {	 \
		.current_pixel_colors = fake_led_strip##idx##_buf,			 \
		.length = FAKE_LED_NUM_PIXELS(idx),               \
	};								 \
									 \
	DEVICE_DT_INST_DEFINE(idx,					 \
			      NULL,				 \
			      NULL,					 \
			      NULL,					 \
			      &fake_led_strip##idx##_cfg,			 \
			      POST_KERNEL,				 \
			      CONFIG_LED_STRIP_INIT_PRIORITY,		 \
			      &fake_led_strip_api);

DT_INST_FOREACH_STATUS_OKAY(FAKE_LED_STRIP_DEVICE)
