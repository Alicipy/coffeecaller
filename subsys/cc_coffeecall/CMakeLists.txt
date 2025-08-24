# SPDX-License-Identifier: Apache-2.0
if(CONFIG_CC_COFFEECALL_SUBSYS)
    zephyr_library()
    zephyr_library_sources(
            src/cc_leds.c
            src/main.c
            src/participant_list.c
            src/state_machine.c
    )

    zephyr_library_sources_ifdef(
        CONFIG_CC_COFFEECALL_SUBSYS_SHELL
        src/cc_shell.c
    )

endif()
