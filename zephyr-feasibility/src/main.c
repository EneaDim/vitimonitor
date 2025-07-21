#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* Thread stack size & priority */
#define STACK_SIZE 1024
#define LED_PRIORITY 5
#define TEMP_PRIORITY 5
#define LUX_PRIORITY 5

#define LED_BLINK_INTERVAL_MS 500
#define TEMP_INTERVAL_MS      1000
#define LUX_INTERVAL_MS       500

/* LED DT spec: `led0` alias points to fakegpio pin in overlay */
//static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);
/* Sensors (we still use device_get_binding since Zephyr sensor API lacks DT helpers yet) */
#define TEMP_HUM_LABEL "TEMP_HUM_EMUL"
#define LIGHT_LABEL    "LIGHT_EMUL"

static const struct device *temp_hum_dev = DEVICE_DT_GET_ANY(DT_NODELABEL(temp_hum_emul));
static const struct device *light_dev = DEVICE_DT_GET_ANY(DT_NODELABEL(light_emul));

/* Stacks for dynamic threads */
K_THREAD_STACK_DEFINE(led_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(temp_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(lux_stack, STACK_SIZE);

/* Thread data */
static struct k_thread led_thread_data;
static struct k_thread temp_thread_data;
static struct k_thread lux_thread_data;

/* LED thread */
void led_thread(void *arg1, void *arg2, void *arg3)
{
    int ret;
    bool led_is_on = false;

    while (1) {
        led_is_on = !led_is_on;
        ret = gpio_pin_set_dt(&led, led_is_on);
        if (ret < 0) {
            LOG_ERR("Failed to set LED: %d", ret);
        }
        k_msleep(LED_BLINK_INTERVAL_MS);
    }
}

/* Temperature & humidity thread */
void temp_thread(void *arg1, void *arg2, void *arg3)
{
    struct sensor_value temp, hum;
    int ret;

    while (1) {
        ret = sensor_sample_fetch(temp_hum_dev);
        if (ret < 0) {
            LOG_ERR("Failed to fetch temp/hum sample: %d", ret);
            k_msleep(TEMP_INTERVAL_MS);
            continue;
        }

        sensor_channel_get(temp_hum_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
        sensor_channel_get(temp_hum_dev, SENSOR_CHAN_HUMIDITY, &hum);

        LOG_INF("Temperature: %d.%06d C, Humidity: %d.%06d %%",
                temp.val1, temp.val2, hum.val1, hum.val2);

        k_msleep(TEMP_INTERVAL_MS);
    }
}

/* Luminosity thread */
void lux_thread(void *arg1, void *arg2, void *arg3)
{
    struct sensor_value lux;
    int ret;

    while (1) {
        ret = sensor_sample_fetch(light_dev);
        if (ret < 0) {
            LOG_ERR("Failed to fetch luminosity sample: %d", ret);
            k_msleep(LUX_INTERVAL_MS);
            continue;
        }

        sensor_channel_get(light_dev, SENSOR_CHAN_LIGHT, &lux);

        LOG_INF("Luminosity: %d.%06d lx", lux.val1, lux.val2);

        k_msleep(LUX_INTERVAL_MS);
    }
}

/* Main */
int main(void)
{
    int ret;

    LOG_INF("Zephyr ESP32-S3 sensor + LED example starting…");

    if (!device_is_ready(led.port)) {
        LOG_ERR("LED GPIO device not ready");
        return 0;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure LED pin: %d", ret);
        return 0;
    }

    /* Get sensor devices */

    //temp_hum_dev = device_get_binding(TEMP_HUM_LABEL);
    //if (!temp_hum_dev) {
    //    LOG_ERR("Failed to bind temp/humidity sensor: %s", TEMP_HUM_LABEL);
    //    return 0;
    //}

    //light_dev = device_get_binding(LIGHT_LABEL);
    //if (!light_dev) {
    //    LOG_ERR("Failed to bind light sensor: %s", LIGHT_LABEL);
    //    return 0;
    //}

    if (!device_is_ready(temp_hum_dev)) {
        LOG_ERR("Temp/Humidity sensor device not ready");
        return 0;
    }

    if (!device_is_ready(light_dev)) {
        LOG_ERR("Light sensor device not ready");
        return 0;
    }
    

    LOG_INF("All devices initialized successfully, starting threads…");

    /* Start threads dynamically */
    k_thread_create(&led_thread_data, led_stack, STACK_SIZE,
                    led_thread, NULL, NULL, NULL,
                    LED_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&temp_thread_data, temp_stack, STACK_SIZE,
                    temp_thread, NULL, NULL, NULL,
                    TEMP_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&lux_thread_data, lux_stack, STACK_SIZE,
                    lux_thread, NULL, NULL, NULL,
                    LUX_PRIORITY, 0, K_NO_WAIT);

    return 0;
}

