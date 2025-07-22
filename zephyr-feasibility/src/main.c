/*
 * Zephyr app: LED + emulated random values (no I2C sensor drivers)
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define STACK_SIZE 1024
#define LED_PRIORITY 5
#define TEMP_PRIORITY 5
#define LUX_PRIORITY 5

#define LED_BLINK_INTERVAL_MS 500
#define TEMP_INTERVAL_MS      1000
#define LUX_INTERVAL_MS       500

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);

K_THREAD_STACK_DEFINE(led_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(temp_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(lux_stack, STACK_SIZE);

static struct k_thread led_thread_data;
static struct k_thread temp_thread_data;
static struct k_thread lux_thread_data;

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

void temp_thread(void *arg1, void *arg2, void *arg3)
{
    int32_t temp_val, hum_val;
    int temp_int, temp_frac, hum_int, hum_frac;

    while (1) {
        temp_val = sys_rand32_get() % 4000;
        hum_val  = sys_rand32_get() % 10000;

        temp_int = temp_val / 100;
        temp_frac = (temp_val % 100) * 10000;
        hum_int  = hum_val / 100;
        hum_frac = (hum_val % 100) * 10000;

        LOG_INF("Emulated Temperature: %d.%06d C, Humidity: %d.%06d %%",
                temp_int, temp_frac, hum_int, hum_frac);

        k_msleep(TEMP_INTERVAL_MS);
    }
}

void lux_thread(void *arg1, void *arg2, void *arg3)
{
    int32_t lux_val;

    while (1) {
        lux_val = sys_rand32_get() % 100000;

        LOG_INF("Emulated Luminosity: %d lx", lux_val);

        k_msleep(LUX_INTERVAL_MS);
    }
}

int main(void)
{
    LOG_INF("Zephyr ESP32-S3 sensor + LED example starting…");

    if (!device_is_ready(led.port)) {
        LOG_ERR("LED GPIO device not ready");
        return 0;
    }

    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
        LOG_ERR("Failed to configure LED pin");
        return 0;
    }

    LOG_INF("All devices initialized successfully, starting threads…");

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

