// Kernel and driver includes
#include <zephyr/kernel.h>           // Zephyr kernel (threads, delay, etc.)
#include <zephyr/device.h>           // Device model
#include <zephyr/devicetree.h>       // Devicetree access macros
#include <zephyr/drivers/gpio.h>     // GPIO APIs
#include <zephyr/drivers/i2c.h>      // I2C APIs
#include <zephyr/logging/log.h>      // Logging system
#include <zephyr/random/random.h>    // Random number generation

#ifdef CONFIG_EMUL
#include "sensirion_sht3xd_emul.h"
#include "rohm_bt1750_emul.h"
#endif

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// -----------------------------------------------------------------------------
// Constants and thread config

#define STACK_SIZE 1024

#define LED_PRIORITY 5
#define TEMP_PRIORITY 5
#define LIGHT_PRIORITY 5

#define LED_BLINK_INTERVAL_MS 500
#define TEMP_INTERVAL_MS      1000
#define LIGHT_INTERVAL_MS     1000

// -----------------------------------------------------------------------------
// LED setup

#define LED0_NODE DT_NODELABEL(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

// -----------------------------------------------------------------------------
// SHT3XD Emulator I2C sensor setup

#define SHT3XD_NODE DT_NODELABEL(sht3xd_emul)
static const struct i2c_dt_spec sht3x_spec = I2C_DT_SPEC_GET(SHT3XD_NODE);

#ifdef CONFIG_EMUL
static const struct emul *sht3xd_emul = EMUL_DT_GET(SHT3XD_NODE);
#endif

// -----------------------------------------------------------------------------
// BH1750 Emulator setup

#define BH1750_NODE DT_NODELABEL(bh1750_emul)
static const struct i2c_dt_spec bh1750_spec = I2C_DT_SPEC_GET(BH1750_NODE);

#ifdef CONFIG_EMUL
static const struct emul *bh1750_emul = EMUL_DT_GET(BH1750_NODE);
#endif

// -----------------------------------------------------------------------------
// Thread stacks and control blocks

K_THREAD_STACK_DEFINE(led_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(temp_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(light_stack, STACK_SIZE);

static struct k_thread led_thread_data;
static struct k_thread temp_thread_data;
static struct k_thread light_thread_data;

// -----------------------------------------------------------------------------
// LED Thread: toggles LED on/off

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
        LOG_INF("LED Blink: %s", led_is_on ? "On" : "Off");
        k_msleep(LED_BLINK_INTERVAL_MS);
    }
}

// -----------------------------------------------------------------------------
// Temperature Thread: fetches SHT3XD sensor data and logs

void temp_thread(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        float temp, hum;

#ifdef CONFIG_EMUL
        if (sht3xd_emul_sample_fetch(sht3xd_emul, &temp, &hum) == 0) {
            LOG_INF("Temperature: %.2f C, Humidity: %.2f %%", temp, hum);
        } else {
            LOG_WRN("Failed to fetch temperature/humidity data");
        }
#endif
        k_msleep(TEMP_INTERVAL_MS);
    }
}

// -----------------------------------------------------------------------------
// Light Thread: power on BH1750, trigger measurement, fetch lux data, log

void light_thread(void *arg1, void *arg2, void *arg3)
{
    const struct device *i2c_dev = bh1750_spec.bus;
    uint8_t cmd;
    uint8_t read_buf[2];
    int ret;
    float lux;

    while (1) {
        // Power on BH1750 (0x01)
        cmd = 0x01;
        ret = i2c_write(i2c_dev, &cmd, 1, bh1750_spec.addr);
        if (ret) {
            LOG_WRN("Failed to power on BH1750: %d", ret);
            k_msleep(LIGHT_INTERVAL_MS);
            continue;
        }

        // Start one-time measurement (0x23 - One-Time L-Resolution Mode)
        cmd = 0x23;
        ret = i2c_write(i2c_dev, &cmd, 1, bh1750_spec.addr);
        if (ret) {
            LOG_WRN("Failed to start BH1750 measurement: %d", ret);
            k_msleep(LIGHT_INTERVAL_MS);
            continue;
        }

        // Measurement time delay (approx 180 ms for BH1750)
        k_msleep(180);

        // Read 2 bytes from BH1750
        ret = i2c_read(i2c_dev, read_buf, 2, bh1750_spec.addr);
        if (ret) {
            LOG_WRN("Failed to read BH1750 data: %d", ret);
        } else {
            uint16_t raw = (read_buf[0] << 8) | read_buf[1];
            lux = raw / 1.2f;
            LOG_INF("Light Intensity: %.2f lux", lux);
        }

        k_msleep(LIGHT_INTERVAL_MS);
    }
}

// -----------------------------------------------------------------------------
// Main: initialize devices and start threads

int main(void)
{
    LOG_INF("Starting Zephyr Sensor Project...");

    if (!device_is_ready(led.port)) {
        LOG_ERR("LED GPIO device not ready");
        return 0;
    }

    if (!device_is_ready(sht3x_spec.bus)) {
        LOG_ERR("I2C bus for SHT3XD not ready");
        return 0;
    }

    if (!device_is_ready(bh1750_spec.bus)) {
        LOG_ERR("I2C bus for BH1750 not ready");
        return 0;
    }

#ifdef CONFIG_EMUL
    if (!device_is_ready(sht3xd_emul->dev)) {
        LOG_ERR("SHT3XD emulator device not ready");
        return 0;
    }
    if (!device_is_ready(bh1750_emul->dev)) {
        LOG_ERR("BH1750 emulator device not ready");
        return 0;
    }
#endif

    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
        LOG_ERR("Failed to configure LED GPIO");
        return 0;
    }

    LOG_INF("Devices ready. Starting threads...");

    k_thread_create(&led_thread_data, led_stack, STACK_SIZE,
                    led_thread, NULL, NULL, NULL,
                    LED_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&temp_thread_data, temp_stack, STACK_SIZE,
                    temp_thread, NULL, NULL, NULL,
                    TEMP_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&light_thread_data, light_stack, STACK_SIZE,
                    light_thread, NULL, NULL, NULL,
                    LIGHT_PRIORITY, 0, K_NO_WAIT);

    return 0;
}

