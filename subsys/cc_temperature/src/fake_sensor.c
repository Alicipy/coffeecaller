#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "../../../subsys/cc_temperature/src/environment_sensor.h"

LOG_MODULE_REGISTER(temperature_app_fake_sensor);

struct device fake_sensor;

int initialize_sht_sensor(const struct device** sht_dev)
{

    *sht_dev = &fake_sensor;
    LOG_INF("Fake sensor initialized");
    return 0;
}

// generate -1, 0, 1, 0, -1, ...
static int next_random_val()
{
    static int last_val_counter = 0;

    const int rand_val = (last_val_counter % 3) - 1;
    last_val_counter++;

    return rand_val;
}


int read_climate_data(const struct device* sht_dev, climate_data_t* climate_data)
{

    if (sht_dev != &fake_sensor)
    {
        LOG_ERR("Wrong or uninitialized sensor");
        return -ENODEV;
    }

    climate_data->temperature = 25.0 + next_random_val() * 0.1;
    climate_data->humidity = 50.0 + next_random_val() * 0.1;

    LOG_DBG("Read temperature and humidity from fake sensor");

    return 0;

}
