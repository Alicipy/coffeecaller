#include "cc_lib/watchdog.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(cc_watchdog);

static const uint32_t WDT_MIN_WINDOW = 0U;
static const uint32_t WDT_MAX_WINDOW = 5000U;

#define WATCHDOG_NODE DT_ALIAS(watchdog0)
#if !DT_NODE_HAS_STATUS(WATCHDOG_NODE, okay)
#error "Unsupported board: watchdog0 devicetree alias is not defined"
#endif

static const struct device *const wdt = DEVICE_DT_GET(WATCHDOG_NODE);
static int32_t cc_wdt_channel_id = 0;

// Public functions
void watchdog_init(void)
{
	if (!device_is_ready(wdt)) {
		LOG_ERR("%s: device not ready.\n", wdt->name);
		return;
	}

	const struct wdt_timeout_cfg wdt_config = {
		.flags = WDT_FLAG_RESET_SOC,
		.window = {
			.min = WDT_MIN_WINDOW,
			.max = WDT_MAX_WINDOW,
		}
	};

	cc_wdt_channel_id = wdt_install_timeout(wdt, &wdt_config);
	if (cc_wdt_channel_id < 0) {
		LOG_ERR("Watchdog install error\n");
		return;
	}

	const int32_t ret = wdt_setup(wdt, 0);
	if (ret < 0) {
		LOG_ERR("Watchdog setup error %d\n", ret);
	}
}

void watchdog_feed(void)
{
	LOG_DBG("Feeding watchdog\n");
	wdt_feed(wdt, cc_wdt_channel_id);
}