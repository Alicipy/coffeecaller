#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(cc_coffeecall_subsys);

#include "state_machine.h"
#include "cc_lib/board.h"
#include "cc_lib/button.h"

static void button0_pressed(void)
{
    LOG_INF("Button 0 - pressed");
    trigger_event(EVENT_LEFT_BTN_PRESS, NULL);
}

static void button0_released(void)
{
    LOG_DBG("Button 0 - released");
}

static void coffeecall_thread(void *a, void *b, void *c)
{
    LOG_INF("Initializing");

    board_init();

    button_state_callbacks button = {
        .on_pressed = button0_pressed,
        .on_released = button0_released
    };
    button_set_callback_for_button(&button, 0);

    state_machine_init();
    LOG_INF("Initialized");

    LOG_INF("Starting Coffee Caller!");
    run_state_machine_forever();
}

K_THREAD_DEFINE(cc_coffeecall_thread, 1024, coffeecall_thread, NULL, NULL, NULL, K_PRIO_COOP(7), 0, 0);
