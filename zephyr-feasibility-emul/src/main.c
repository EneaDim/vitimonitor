#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/emul.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

#include "sensirion_sht3xd_emul.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// -----------------------------------------------------------------------------
// Configs and constants

#define STACK_SIZE 1024
#define LED_PRIORITY 5
#define TEMP_PRIORITY 5

#define LED_BLINK_INTERVAL_MS 500
#define TEMP_INTERVAL_MS      1000

// LED
#define LED0_NODE DT_NODELABEL(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

// I2C bus
#define I2C_NODE DT_NODELABEL(i2c0)
static const struct device *i2c_dev = DEVICE_DT_GET(I2C_NODE);

// SHT3XD emulator node (emulated device)
#define SHT3XD_EMUL_NODE DT_NODELABEL(sht3xd_emul)
static const struct emul *sht3xd_emul = EMUL_DT_GET(SHT3XD_EMUL_NODE);

// Thread stacks and data
K_THREAD_STACK_DEFINE(led_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(temp_stack, STACK_SIZE);
static struct k_thread led_thread_data;
static struct k_thread temp_thread_data;

// -----------------------------------------------------------------------------
// LED Thread

void led_thread(void)
{
    int ret;
    bool led_is_on = false;

    while (1) {
        led_is_on = !led_is_on;
        ret = gpio_pin_set_dt(&led, led_is_on);
        if (ret < 0) {
            LOG_ERR("Failed to set LED: %d", ret);
        }
        LOG_INF("Emulated LED Blink: %s", led_is_on ? "On" : "Off");
        k_msleep(LED_BLINK_INTERVAL_MS);
    }
}

// -----------------------------------------------------------------------------
// Temperature Thread
void temp_thread(void)
{
    uint8_t cmd[] = { 0x2C, 0x06 };  // High repeatability measurement command
    uint8_t read_buf[6];            // Temp (2+1 CRC) + Humidity (2+1 CRC)

    while (1) {
        // Randomize values to simulate changing environment
        uint16_t temp_raw = 0x6000 + (sys_rand32_get() % 0x0800);
        uint16_t hum_raw  = 0x8000 + (sys_rand32_get() % 0x1000);

        const struct sht3xd_emul_api *api =
            (const struct sht3xd_emul_api *) sht3xd_emul->dev->api;

        api->set(sht3xd_emul->dev, temp_raw, hum_raw);

        int ret = i2c_write(i2c_dev, cmd, sizeof(cmd), 0x44);
        if (ret != 0) {
            LOG_ERR("I2C write failed: %d", ret);
            k_msleep(TEMP_INTERVAL_MS);
            continue;
        }

        ret = i2c_read(i2c_dev, read_buf, sizeof(read_buf), 0x44);
        if (ret != 0) {
            LOG_ERR("I2C read failed: %d", ret);
            k_msleep(TEMP_INTERVAL_MS);
            continue;
        }

        uint16_t temp_val = (read_buf[0] << 8) | read_buf[1];
        uint16_t hum_val  = (read_buf[3] << 8) | read_buf[4];

        float temp_c = -45 + 175 * ((float)temp_val / 65535);
        float hum_pct = 100 * ((float)hum_val / 65535);

        LOG_INF("Emulated Temperature: %.2f C, Humidity: %.2f %%", temp_c, hum_pct);

        k_msleep(TEMP_INTERVAL_MS);
    }
}

// -----------------------------------------------------------------------------
// Main function

int main(void)
{
    LOG_INF("Zephyr SHT3XD Emulator + LED example starting...");

    if (!device_is_ready(led.port)) {
        LOG_ERR("LED GPIO device not ready");
        return 0;
    }

    if (!device_is_ready(i2c_dev)) {
        LOG_ERR("I2C device not ready");
        return 0;
    }

    if (!device_is_ready(sht3xd_emul->dev)) {
        LOG_ERR("SHT3XD emulator device not ready");
        return 0;
    }

    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
        LOG_ERR("Failed to configure LED pin");
        return 0;
    }

    LOG_INF("All devices initialized successfully. Starting threads...");

    k_thread_create(&led_thread_data, led_stack, STACK_SIZE,
                    led_thread, NULL, NULL, NULL,
                    LED_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&temp_thread_data, temp_stack, STACK_SIZE,
                    temp_thread, NULL, NULL, NULL,
                    TEMP_PRIORITY, 0, K_NO_WAIT);

    return 0;
}

