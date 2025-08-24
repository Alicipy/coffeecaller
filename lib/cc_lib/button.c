#include "cc_lib/button.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(cc_button);


#define SW0_NODE DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

void button0_debounce_handler(struct k_work *work);
void button2_debounce_handler(struct k_work *work);

static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button0_cb_data;
static K_WORK_DELAYABLE_DEFINE(button0_debounce, button0_debounce_handler);

#define NUM_OF_BUTTONS 1

static button_state_callbacks button_callbacks[NUM_OF_BUTTONS] = {0};

// Private functions

void setup_button(const struct gpio_dt_spec *button, struct gpio_callback *cb,
		  void (*button_handler)(const struct device *, struct gpio_callback *, uint32_t))
{
	int ret;

	if (!gpio_is_ready_dt(button)) {
		LOG_ERR("Error: button device %s is not ready\n", button->port->name);
		return;
	}

	ret = gpio_pin_configure_dt(button, GPIO_INPUT);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d\n", ret, button->port->name,
			button->pin);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(button, GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n", ret,
			button->port->name, button->pin);
		return;
	}

	gpio_init_callback(cb, button_handler, BIT(button->pin));
	gpio_add_callback(button->port, cb);
	LOG_INF("Set up button at %s pin %d\n", button->port->name, button->pin);
}

void button_pressed_button0(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	k_work_reschedule(&button0_debounce, K_MSEC(15));
}

void button0_debounce_handler(struct k_work *work)
{
	ARG_UNUSED(work);
	int32_t val = gpio_pin_get_dt(&button0);
	if (val) {
		LOG_DBG("Button 0 pressed\n");
		if (button_callbacks[0].on_pressed != NULL) {
			button_callbacks[0].on_pressed();
		}
	} else {
		LOG_DBG("Button 0 released\n");
		if (button_callbacks[0].on_released != NULL) {
			button_callbacks[0].on_released();
		}
	}
}

// Public functions
void button_init(void)
{
	setup_button(&button0, &button0_cb_data, button_pressed_button0);
	return;
}

void button_set_callback_for_button(button_state_callbacks *callbacks, uint8_t button_idx)
{
	if (button_idx >= NUM_OF_BUTTONS) {
		LOG_ERR("No button for index present\n");
		return;
	}

	button_callbacks[button_idx] = *callbacks;
}
