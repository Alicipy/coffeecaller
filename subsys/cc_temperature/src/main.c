#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "environment_sensor.h"

#include "cc_broadcast/cc_broadcast.h"

LOG_MODULE_REGISTER(temperature_app);

static const char* BROADCAST_TYPE = "cc_temperature";

static void receive_temperature_message_handler(const cc_broadcast_message_t* message)
{
    printf("%llx: %s\r\n", message->sender_id, message->payload);
}

static void temperature_read_thread(void *a, void *b, void *c)
{
    const struct device* sht_dev;
    int ret = initialize_sht_sensor(&sht_dev);
    if (ret)
    {
        LOG_ERR("Failed to initialize SHT4x sensor");
        return;
    }

    cc_broadcast_client_id_t broadcast_client_id;
    ret = cc_broadcast_register_client(&broadcast_client_id, BROADCAST_TYPE, receive_temperature_message_handler);
    if (ret)
    {
        LOG_ERR("Failed to register broadcast client for temperature");
        return;
    }

    while (1)
    {

        climate_data_t climate_data;
        ret = read_climate_data(sht_dev, &climate_data);
        if (ret < 0)
        {
            LOG_ERR("Failed to read climate data (%d)", ret);
            goto end_of_loop;
        }

        char payload[200];

        const int msg_len = snprintf(payload, sizeof(payload),
                                     "Temperature: %.2f °C | Humidity: %.2f %%",
                                     climate_data.temperature, climate_data.humidity);
        if (msg_len < 0 || msg_len >= sizeof(payload))
        {
            LOG_ERR("Error formatting temperature message");
            goto end_of_loop;
        }

        ret = cc_broadcast_send(broadcast_client_id, payload);
        if (ret < 0)
        {
            LOG_ERR("Failed to send temperature message (%d)", ret);
            goto end_of_loop;
        }

        LOG_DBG("Temperature message sent successfully");

    end_of_loop:
        k_sleep(K_SECONDS(2));
    }
}

K_THREAD_DEFINE(cc_temperature_thread, 1024, temperature_read_thread, NULL, NULL, NULL, K_PRIO_COOP(7), 0, 0);