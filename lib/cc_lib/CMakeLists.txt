# Declare a Zephyr library
zephyr_library()

# Add source files to the library
zephyr_library_sources(
        aldrin.c
        board.c
        button.c
        leds.c
        watchdog.c
)