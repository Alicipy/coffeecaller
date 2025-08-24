# SPDX-License-Identifier: Apache-2.0
if(CONFIG_CC_TEMPERATURE_SUBSYS)
    zephyr_library()
    zephyr_library_sources(
            src/main.c
    )

    zephyr_library_sources_ifdef(CONFIG_CC_TEMPERATURE_SENSOR_DHT src/dht_sensor.c)
    zephyr_library_sources_ifdef(CONFIG_CC_TEMPERATURE_SENSOR_FAKE src/fake_sensor.c)
endif()
