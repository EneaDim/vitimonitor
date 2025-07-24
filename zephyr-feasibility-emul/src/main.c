// Include Zephyr kernel headers and drivers
#include <zephyr/kernel.h>         // Kernel APIs (threads, delays)
#include <zephyr/device.h>         // Device driver model
#include <zephyr/devicetree.h>     // Devicetree access macros
#include <zephyr/drivers/gpio.h>   // GPIO driver API
#include <zephyr/drivers/i2c.h>    // I2C driver API
#include <zephyr/drivers/emul.h>   // Emulation driver API
#include <zephyr/logging/log.h>    // Logging infrastructure
#include <zephyr/random/random.h>  // Random number generation

// Include SHT3x-D emulator API (custom emulation interface)
#include "sensirion_sht3xd_emul.h"

// Initialize logging module with tag "main" and INFO log level
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// -----------------------------------------------------------------------------
// Configuration and constants

#define STACK_SIZE 1024           // Stack size for each thread
#define LED_PRIORITY 5            // LED thread priority
#define TEMP_PRIORITY 5           // Temperature thread priority

#define LED_BLINK_INTERVAL_MS 500 // LED blink interval in milliseconds
#define TEMP_INTERVAL_MS      1000 // Temp read interval in milliseconds

// LED configuration from devicetree
#define LED0_NODE DT_NODELABEL(led0)
// Get GPIO specification for the LED
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

// I2C configuration from devicetree
#define I2C_NODE DT_NODELABEL(i2c0)
// Get device handle for I2C bus
static const struct device *i2c_dev = DEVICE_DT_GET(I2C_NODE);

// SHT3XD emulator configuration from devicetree
#define SHT3XD_EMUL_NODE DT_NODELABEL(sht3xd_emul)
// Get handle to the emulator object
static const struct emul *sht3xd_emul = EMUL_DT_GET(SHT3XD_EMUL_NODE);

// Define thread stacks and metadata
K_THREAD_STACK_DEFINE(led_stack, STACK_SIZE);       // Stack for LED thread
K_THREAD_STACK_DEFINE(temp_stack, STACK_SIZE);      // Stack for temp thread
static struct k_thread led_thread_data;             // LED thread control block
static struct k_thread temp_thread_data;            // Temp thread control block

// -----------------------------------------------------------------------------
// LED Thread - blinks LED at fixed interval

void led_thread(void)
{
    int ret;
    bool led_is_on = false;

    while (1) {
        led_is_on = !led_is_on;                       // Toggle LED state
        ret = gpio_pin_set_dt(&led, led_is_on);       // Set GPIO output
        if (ret < 0) {
            LOG_ERR("Failed to set LED: %d", ret);     // Log error if set fails
        }
        LOG_INF("Emulated LED Blink: %s", led_is_on ? "On" : "Off"); // Log LED state
        k_msleep(LED_BLINK_INTERVAL_MS);              // Delay for blink interval
    }
}

// -----------------------------------------------------------------------------
// Temperature Thread - simulates and reads temperature & humidity data

void temp_thread(void)
{
    uint8_t cmd[] = { 0x2C, 0x06 };  // SHT3x command: high repeatability, clock stretching
    uint8_t read_buf[6];            // Buffer for I2C read: Temp + CRC + Humidity + CRC

    while (1) {
        // Generate random raw temperature and humidity values
        uint16_t temp_raw = 0x6000 + (sys_rand32_get() % 0x0800);
        uint16_t hum_raw  = 0x8000 + (sys_rand32_get() % 0x1000);

        // Get access to the emulator's API
        const struct sht3xd_emul_api *api =
            (const struct sht3xd_emul_api *) sht3xd_emul->dev->api;

        // Set new temperature and humidity values in the emulator
        api->set(sht3xd_emul->dev, temp_raw, hum_raw);

        // Send measurement command to the emulator via I2C
        int ret = i2c_write(i2c_dev, cmd, sizeof(cmd), 0x44); // 0x44 = SHT3x default addr
        if (ret != 0) {
            LOG_ERR("I2C write failed: %d", ret); // Log error
            k_msleep(TEMP_INTERVAL_MS);
            continue;
        }

        // Read 6 bytes of sensor data from the emulator
        ret = i2c_read(i2c_dev, read_buf, sizeof(read_buf), 0x44);
        if (ret != 0) {
            LOG_ERR("I2C read failed: %d", ret); // Log error
            k_msleep(TEMP_INTERVAL_MS);
            continue;
        }

        // Extract raw temperature and humidity from buffer
        uint16_t temp_val = (read_buf[0] << 8) | read_buf[1];
        uint16_t hum_val  = (read_buf[3] << 8) | read_buf[4];

        // Convert raw values to human-readable units
        float temp_c = -45 + 175 * ((float)temp_val / 65535);      // °C formula from datasheet
        float hum_pct = 100 * ((float)hum_val / 65535);            // %RH formula from datasheet

        // Log the simulated values
        LOG_INF("Emulated Temperature: %.2f C, Humidity: %.2f %%", temp_c, hum_pct);

        // Delay until next measurement cycle
        k_msleep(TEMP_INTERVAL_MS);
    }
}

// -----------------------------------------------------------------------------
// Main function - initializes devices and launches threads

int main(void)
{
    LOG_INF("Zephyr SHT3XD Emulator + LED example starting...");

    // Check if LED GPIO device is ready
    if (!device_is_ready(led.port)) {
        LOG_ERR("LED GPIO device not ready");
        return 0;
    }

    // Check if I2C device is ready
    if (!device_is_ready(i2c_dev)) {
        LOG_ERR("I2C device not ready");
        return 0;
    }

    // Check if emulator device is ready
    if (!device_is_ready(sht3xd_emul->dev)) {
        LOG_ERR("SHT3XD emulator device not ready");
        return 0;
    }

    // Configure the LED pin as output and set it active
    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
        LOG_ERR("Failed to configure LED pin");
        return 0;
    }

    LOG_INF("All devices initialized successfully. Starting threads...");

    // Start LED thread
    k_thread_create(&led_thread_data, led_stack, STACK_SIZE,
                    led_thread, NULL, NULL, NULL,
                    LED_PRIORITY, 0, K_NO_WAIT);

    // Start temperature thread
    k_thread_create(&temp_thread_data, temp_stack, STACK_SIZE,
                    temp_thread, NULL, NULL, NULL,
                    TEMP_PRIORITY, 0, K_NO_WAIT);

    return 0; // Main thread exits, threads keep running
}

