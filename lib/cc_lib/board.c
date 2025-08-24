#include "cc_lib/board.h"

#include "cc_lib/aldrin.h"
#include "cc_lib//button.h"
#include "cc_lib/leds.h"
#include "cc_lib/watchdog.h"

void board_init(void)
{
	aldrin_init();
	button_init();
	leds_init();
	watchdog_init();
}
