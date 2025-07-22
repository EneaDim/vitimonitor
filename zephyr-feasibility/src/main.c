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

/* Emulated I2C read function */
int emulated_i2c_read(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, size_t len)
{
    switch (device_addr) {
    case 0x44:  /* Temperature & Humidity sensor */
        if (reg_addr == 0x00) {  // Temperature register
            uint16_t temp = sys_rand32_get() % 4000;
            data[0] = (temp >> 8) & 0xFF;
            data[1] = temp & 0xFF;
        } else if (reg_addr == 0x01) {  // Humidity register
            uint16_t hum = sys_rand32_get() % 10000;
            data[0] = (hum >> 8) & 0xFF;
            data[1] = hum & 0xFF;
        }
        break;

    case 0x23:  /* Lux sensor */
        if (reg_addr == 0x10) {
            uint32_t lux = sys_rand32_get() % 100000;
            data[0] = (lux >> 16) & 0xFF;
            data[1] = (lux >> 8) & 0xFF;
            data[2] = lux & 0xFF;
        }
        break;

    default:
        LOG_WRN("Unknown I2C device address 0x%02X", device_addr);
        return -EINVAL;
    }

    LOG_DBG("Emulated I2C read from 0x%02X reg 0x%02X len %zu", device_addr, reg_addr, len);
    return 0;
}

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
        LOG_INF("Emulated Led Blink: %s", led_is_on ? "True" : "False");
        k_msleep(LED_BLINK_INTERVAL_MS);
    }
}

void temp_thread(void *arg1, void *arg2, void *arg3)
{
    uint8_t buf[2];
    uint16_t temp_val, hum_val;
    int temp_int, temp_frac, hum_int, hum_frac;

    while (1) {
        if (emulated_i2c_read(0x44, 0x00, buf, 2) == 0) {
            temp_val = ((uint16_t)buf[0] << 8) | buf[1];
        }

        if (emulated_i2c_read(0x44, 0x01, buf, 2) == 0) {
            hum_val = ((uint16_t)buf[0] << 8) | buf[1];
        }

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
    uint8_t buf[3];
    uint32_t lux_val;

    while (1) {
        if (emulated_i2c_read(0x23, 0x10, buf, 3) == 0) {
            lux_val = ((uint32_t)buf[0] << 16) |
                      ((uint32_t)buf[1] << 8) |
                       (uint32_t)buf[2];
        }

        LOG_INF("Emulated Luminosity: %u lx", lux_val);

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

