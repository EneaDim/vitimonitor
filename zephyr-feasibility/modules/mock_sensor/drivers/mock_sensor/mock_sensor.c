#include "mock_sensor.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>  // rand()
#include <zephyr/drivers/sensor.h>

static struct sensor_value generate_random_value(void)
{
    struct sensor_value val;
    val.val1 = rand() % 40 + 10;     // 10..50
    val.val2 = rand() % 1000000;     // decimali
    return val;
}

static int mock_sensor_sample_fetch_impl(const struct device *dev, enum sensor_channel chan)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(chan);
    // Simulazione ritardo acquisizione dati
    k_sleep(K_MSEC(10));
    return 0;
}

static int mock_sensor_channel_get_impl(const struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
    ARG_UNUSED(dev);

    if (!val) {
        return -EINVAL;
    }

    switch (chan) {
    case SENSOR_CHAN_AMBIENT_TEMP:
    case SENSOR_CHAN_HUMIDITY:
    case SENSOR_CHAN_LIGHT:
        *val = generate_random_value();
        return 0;
    default:
        return -ENOTSUP;
    }
}

// API driver sensore per Zephyr
static const struct sensor_driver_api mock_sensor_api = {
    .sample_fetch = mock_sensor_sample_fetch_impl,
    .channel_get = mock_sensor_channel_get_impl,
};

// Inizializzazione del driver mock
static int mock_sensor_init(const struct device *dev)
{
    printk("Mock sensor %s initialized\n", dev->name);
    return 0;
}

#if IS_ENABLED(CONFIG_MOCK_SENSOR)
DEVICE_DT_DEFINE(DT_INST(0, mock_sensor),
                 mock_sensor_init,
                 NULL,
                 NULL,
                 NULL,
                 POST_KERNEL,
                 CONFIG_SENSOR_INIT_PRIORITY,
                 &mock_sensor_api);
#endif

// Funzioni helper pubbliche che richiamano le _impl
int mock_sensor_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    return mock_sensor_sample_fetch_impl(dev, chan);
}

int mock_sensor_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
    return mock_sensor_channel_get_impl(dev, chan, val);
}

