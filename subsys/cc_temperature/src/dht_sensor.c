#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "environment_sensor.h"

LOG_MODULE_REGISTER(temperature_app_dht_sensor);

int initialize_sht_sensor(const struct device** sht_dev)
{
    *sht_dev = DEVICE_DT_GET_ANY(sensirion_sht4x);

    if (!device_is_ready(*sht_dev))
    {
        LOG_ERR("SHT4x sensor not ready");
        return -ENODEV;
    }

    LOG_INF("Found SHT4x sensor: %s", (*sht_dev)->name);
    return 0;
}


int read_climate_data(const struct device* sht_dev, climate_data_t* climate_data)
{
    struct sensor_value temp, humidity;

    int ret = sensor_sample_fetch(sht_dev);
    if (ret)
    {
        LOG_ERR("Sensor fetch failed (%d)", ret);
        return ret;
    }

    ret = sensor_channel_get(sht_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
    if (ret)
    {
        LOG_ERR("Failed to get temperature (%d)", ret);
        return ret;
    }
    LOG_DBG("Got temperature from sensor: z%.2f", sensor_value_to_double(&temp));

    ret = sensor_channel_get(sht_dev, SENSOR_CHAN_HUMIDITY, &humidity);
    if (ret)
    {
        LOG_ERR("Failed to get humidity (%d)", ret);
        return ret;
    }
    LOG_DBG("Got humidity from sensor: %.2f", sensor_value_to_double(&temp));

    climate_data->temperature = sensor_value_to_double(&temp);
    climate_data->humidity = sensor_value_to_double(&humidity);

    return 0;
}
