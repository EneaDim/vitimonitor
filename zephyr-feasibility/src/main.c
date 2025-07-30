// -----------------------------------------------------------------------------
// Kernel and driver includes

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
#include <zephyr/drivers/lora.h>
#include <stdio.h>

#ifdef CONFIG_EMUL
#include "sensirion_sht3xd_emul.h"
#include "rohm_bh1750_emul.h"
#include "sx1262_emul.h"
#endif

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// -----------------------------------------------------------------------------
// Constants and thread configuration

#define STACK_SIZE 1024

#define LED_PRIORITY    5
#define TEMP_PRIORITY   5
#define LIGHT_PRIORITY  5
#define LORA_PRIORITY     5

#define LED_BLINK_INTERVAL_MS   500
#define TEMP_INTERVAL_MS       1000
#define LIGHT_INTERVAL_MS      1000
#define LORA_INTERVAL_MS  5000

// -----------------------------------------------------------------------------
// LED GPIO configuration

#define LED0_NODE DT_NODELABEL(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// -----------------------------------------------------------------------------
// SHT3XD (temperature/humidity) sensor configuration

#define SHT3XD_NODE DT_NODELABEL(sht3xd)
static const struct device *sht3xd_dev = DEVICE_DT_GET(SHT3XD_NODE);

#ifdef CONFIG_EMUL
static const struct emul *sht3xd_emul = EMUL_DT_GET(SHT3XD_NODE);
#endif

// -----------------------------------------------------------------------------
// BH1750 (light intensity) sensor configuration

#define BH1750_NODE DT_NODELABEL(bh1750)
static const struct device *bh1750_dev = DEVICE_DT_GET(BH1750_NODE);
static const struct i2c_dt_spec bh1750_spec = I2C_DT_SPEC_GET(BH1750_NODE);

#ifdef CONFIG_EMUL
static const struct emul *bh1750_emul = EMUL_DT_GET(BH1750_NODE);
#endif

// -----------------------------------------------------------------------------
// LoRa driver
#define SX1262_NODE DT_NODELABEL(sx1262)
static const struct device *sx1262_dev = DEVICE_DT_GET(SX1262_NODE);

#ifdef CONFIG_EMUL
static const struct emul *sx1262_emul = EMUL_DT_GET(SX1262_NODE);
#endif

// -----------------------------------------------------------------------------
// Thread stacks and control blocks

K_THREAD_STACK_DEFINE(led_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(temp_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(light_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(lora_stack, STACK_SIZE);

static struct k_thread led_thread_data;
static struct k_thread temp_thread_data;
static struct k_thread light_thread_data;
static struct k_thread lora_thread_data;

// -----------------------------------------------------------------------------
// LED Thread: toggles the LED periodically

void led_thread(void *arg1, void *arg2, void *arg3)
{
    bool state = false;

    while (1) {
        state = !state;
        gpio_pin_set_dt(&led, state);
        LOG_INF("LED: %s", state ? "ON" : "OFF");
        k_msleep(LED_BLINK_INTERVAL_MS);
    }
}

// -----------------------------------------------------------------------------
// Temperature Thread: reads SHT3XD via sensor API and logs values

void temp_thread(void *arg1, void *arg2, void *arg3)
{
    struct sensor_value temp, hum;

    while (1) {
        if (sensor_sample_fetch(sht3xd_dev) == 0 &&
            sensor_channel_get(sht3xd_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) == 0 &&
            sensor_channel_get(sht3xd_dev, SENSOR_CHAN_HUMIDITY, &hum) == 0) {

            float tf = sensor_value_to_double(&temp);
            float hf = sensor_value_to_double(&hum);
            LOG_INF("Temp: %.2f °C, Humidity: %.2f %%", (double)tf, (double)hf);
        } else {
            LOG_WRN("Failed to fetch SHT3XD sample");
        }

        k_msleep(TEMP_INTERVAL_MS);
    }
}

// -----------------------------------------------------------------------------
// Light Thread: reads BH1750 via sensor API and logs values
// -----------------------------------------------------------------------------

void light_thread(void *arg1, void *arg2, void *arg3)
{
    struct sensor_value lux;
    uint8_t power_on_cmd = 0x01;
    int ret;

    // Power ON → usa il bus I2C reale
    ret = i2c_write(bh1750_spec.bus, &power_on_cmd, 1, DT_REG_ADDR(BH1750_NODE));
    if (ret < 0) {
        LOG_ERR("Failed to power on BH1750: %d", ret);
        return;
    }

    while (1) {
        if (sensor_sample_fetch(bh1750_dev) == 0 &&
            sensor_channel_get(bh1750_dev, SENSOR_CHAN_LIGHT, &lux) == 0) {

            float lf = sensor_value_to_double(&lux);
            LOG_INF("Light Intensity: %.2f lux", (double)lf);
        } else {
            LOG_WRN("Failed to fetch BH1750 sample");
        }

        k_msleep(LIGHT_INTERVAL_MS);
    }
}

void lora_thread(void *arg1, void *arg2, void *arg3)
{
    struct sensor_value temp, hum, lux;
    char payload[64];

    while (1) {
        bool ok = sensor_sample_fetch(sht3xd_dev) == 0 &&
                  sensor_channel_get(sht3xd_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) == 0 &&
                  sensor_channel_get(sht3xd_dev, SENSOR_CHAN_HUMIDITY, &hum) == 0 &&
                  sensor_sample_fetch(bh1750_dev) == 0 &&
                  sensor_channel_get(bh1750_dev, SENSOR_CHAN_LIGHT, &lux) == 0;

        if (ok) {
            float tf = sensor_value_to_double(&temp);
            float hf = sensor_value_to_double(&hum);
            float lf = sensor_value_to_double(&lux);

            snprintf(payload, sizeof(payload), "T:%.1f H:%.1f L:%.1f", (double)tf, (double)hf, (double)lf);
            //LOG_INF("T: %.1f H: %.1f L: %.1f", (double)tf, (double)hf, (double)lf);

            int ret = sx1262_send(sx1262_dev, (uint8_t *)payload, strlen(payload));
            if (ret == 0) {
                LOG_INF("LoRa TX: %s", payload);
            } else {
                LOG_ERR("LoRa send failed: %d", ret);
            }
        } else {
            LOG_WRN("Sensor read failed");
        }

        k_msleep(5000);  // invia ogni 5 secondi
    }
}

// -----------------------------------------------------------------------------
// Main: initialize devices and launch threads

int main(void)
{
    LOG_INF("Booting Sensor Application...");

    // Check device readiness
    if (!device_is_ready(led.port)) {
        LOG_ERR("LED device not ready");
        return 0;
    }

    if (!device_is_ready(sht3xd_dev)) {
        LOG_ERR("SHT3XD sensor not ready");
        return 0;
    }

    if (!device_is_ready(bh1750_dev)) {
        LOG_ERR("BH1750 sensor not ready");
        return 0;
    }

    if (!device_is_ready(sx1262_dev)) {
        LOG_ERR("SX1262 not ready");
        return 0;
    }

#ifdef CONFIG_EMUL
    if (!device_is_ready(sht3xd_emul->dev)) {
        LOG_ERR("SHT3XD emulator not ready");
        return 0;
    }

    if (!device_is_ready(bh1750_emul->dev)) {
        LOG_ERR("BH1750 emulator not ready");
        return 0;
    }
    if (!device_is_ready(sx1262_emul->dev)) {
        LOG_ERR("SX1262 emulator not ready");
        return 0;
    }
#endif

    // Configure LED pin
    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
        LOG_ERR("Failed to configure LED GPIO");
        return 0;
    }

    LOG_INF("Devices ready. Launching threads...");

    // Start threads
    k_thread_create(&led_thread_data, led_stack, STACK_SIZE,
                    led_thread, NULL, NULL, NULL,
                    LED_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&temp_thread_data, temp_stack, STACK_SIZE,
                    temp_thread, NULL, NULL, NULL,
                    TEMP_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&light_thread_data, light_stack, STACK_SIZE,
                    light_thread, NULL, NULL, NULL,
                    LIGHT_PRIORITY, 0, K_NO_WAIT);
    k_thread_create(&lora_thread_data, lora_stack, STACK_SIZE,
                    lora_thread, NULL, NULL, NULL,
                    LORA_PRIORITY, 0, K_NO_WAIT);
    

    return 0;
}

