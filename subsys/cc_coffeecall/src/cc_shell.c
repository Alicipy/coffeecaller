#include <stdlib.h>
#include <inttypes.h>

#include <zephyr/shell/shell.h>

#include "state_machine.h"

static int cmd_press_button(const struct shell* shell, const size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(shell, "Button pressed!");

    trigger_event(EVENT_LEFT_BTN_PRESS, NULL);

    return 0;
}

static int cmd_recv_take_part_msg(const struct shell* shell, const size_t argc, char** argv)
{
    ARG_UNUSED(argc);

    char* parse_pointer;

    const uint64_t from = strtoull(argv[1], &parse_pointer, 10);
    if (*parse_pointer != '\0')
    {
        shell_error(shell, "Invalid 'from' argument: %s", argv[1]);
        return -EINVAL;
    }

    const long tmp = strtol(argv[2], &parse_pointer, 10);
    if (*parse_pointer != '\0' || (tmp != 0 && tmp != 1))
    {
        shell_error(shell, "Invalid 'take_part' argument: %s (must be 0 or 1)", argv[2]);
        return -EINVAL;
    }
    const uint8_t take_part = (uint8_t)tmp;

    shell_print(shell, "Received recv_take_part_msg: from=%llu, take_part=%u", from, take_part);

    struct participant participant_info = {
        .from = from,
        .takes_part = take_part,
    };

    trigger_event(EVENT_RECV_PART_MESSAGE, &participant_info);

    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_cc,
                               SHELL_CMD_ARG(press_button, NULL,
                                   "Simulate a button press.",
                                   cmd_press_button, 1, 0),
                               SHELL_CMD_ARG(recv_take_part_msg, NULL,
                                   "Send recv_take_part_msg <from:uint64> <take_part:0|1>",
                                   cmd_recv_take_part_msg, 3, 0),
                               SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(cc, &sub_cc, "CoffeeCall related commands", NULL);
