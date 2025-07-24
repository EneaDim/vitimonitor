// Kernel and driver includes
#include <zephyr/kernel.h>           // Zephyr kernel (threads, delay, etc.)
#include <zephyr/device.h>           // Device model
#include <zephyr/devicetree.h>       // Devicetree access macros
#include <zephyr/drivers/gpio.h>     // GPIO APIs
#include <zephyr/drivers/i2c.h>      // I2C APIs
#include <zephyr/logging/log.h>      // Logging system
#include <zephyr/random/random.h>    // Random number generation

// Optional emulator header (only included if emulator is enabled in config)
#ifdef CONFIG_EMUL
#include "sensirion_sht3xd_emul.h"
#endif

// Register this file for logging with the "main" tag and INFO log level
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// -----------------------------------------------------------------------------
// Constants and thread configuration

#define STACK_SIZE 1024              // Stack size for each thread
#define LED_PRIORITY 5               // LED thread priority
#define TEMP_PRIORITY 5              // Temperature thread priority

#define LED_BLINK_INTERVAL_MS 500    // LED toggle interval
#define TEMP_INTERVAL_MS      1000   // Temp sampling interval

// -----------------------------------------------------------------------------
// LED setup using devicetree

#define LED0_NODE DT_NODELABEL(led0)                           // Node label from devicetree
static const struct gpio_dt_spec led =                         // Get GPIO spec struct
    GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);                   // From LED alias in devicetree

// -----------------------------------------------------------------------------
// I2C sensor setup

#define SHT3XD_NODE DT_NODELABEL(sht3xd_emul)                  // Node label for SHT3XD emulator
static const struct i2c_dt_spec sht3x_spec =                   // Get I2C spec struct
    I2C_DT_SPEC_GET(SHT3XD_NODE);                              // Includes bus + address

#ifdef CONFIG_EMUL
// If emulator is enabled, get emulator object handle
static const struct emul *sht3xd_emul = EMUL_DT_GET(SHT3XD_NODE);
#endif

// -----------------------------------------------------------------------------
// Thread resources (stacks + thread metadata)

K_THREAD_STACK_DEFINE(led_stack, STACK_SIZE);     // Stack memory for LED thread
K_THREAD_STACK_DEFINE(temp_stack, STACK_SIZE);    // Stack memory for temp thread
static struct k_thread led_thread_data;           // Thread control block for LED thread
static struct k_thread temp_thread_data;          // Thread control block for temp thread

// -----------------------------------------------------------------------------
// LED Thread
// Periodically toggles the LED output

void led_thread(void)
{
    int ret;
    bool led_is_on = false;

    while (1) {
        led_is_on = !led_is_on;                          // Toggle LED state
        ret = gpio_pin_set_dt(&led, led_is_on);          // Set the new value to GPIO
        if (ret < 0) {
            LOG_ERR("Failed to set LED: %d", ret);       // Log error if GPIO write failed
        }
        LOG_INF("LED Blink: %s", led_is_on ? "On" : "Off"); // Log LED state
        k_msleep(LED_BLINK_INTERVAL_MS);                 // Wait for blink interval
    }
}

// -----------------------------------------------------------------------------
// Temperature Thread
// Periodically generates a sample and logs the result

void temp_thread(void)
{
    while (1) {
        float temp, hum;

        // Fetch simulated temperature and humidity from emulator
        if (sht3xd_emul_sample_fetch(sht3xd_emul, &temp, &hum) == 0) {
            LOG_INF("Temperature: %.2f C, Humidity: %.2f %%", temp, hum);  // Log values
        }

        k_msleep(TEMP_INTERVAL_MS);    // Wait until next sample
    }
}

// -----------------------------------------------------------------------------
// Main function
// Initializes peripherals and starts both threads

int main(void)
{
    LOG_INF("Starting Zephyr SHT3XD project...");

    // Check if LED device (GPIO controller) is ready
    if (!device_is_ready(led.port)) {
        LOG_ERR("LED GPIO device not ready");
        return 0;
    }

    // Check if the I2C bus is ready
    if (!device_is_ready(sht3x_spec.bus)) {
        LOG_ERR("I2C bus not ready");
        return 0;
    }

#ifdef CONFIG_EMUL
    // Check if emulator device is ready
    if (!device_is_ready(sht3xd_emul->dev)) {
        LOG_ERR("Emulated SHT3XD device not ready");
        return 0;
    }
#endif

    // Configure LED pin as output and set it active
    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
        LOG_ERR("Failed to configure LED pin");
        return 0;
    }

    LOG_INF("Devices ready. Starting threads...");

    // Create and start LED thread
    k_thread_create(&led_thread_data, led_stack, STACK_SIZE,
                    led_thread, NULL, NULL, NULL,
                    LED_PRIORITY, 0, K_NO_WAIT);

    // Create and start temperature thread
    k_thread_create(&temp_thread_data, temp_stack, STACK_SIZE,
                    temp_thread, NULL, NULL, NULL,
                    TEMP_PRIORITY, 0, K_NO_WAIT);

    return 0;  // Main function returns; threads continue running
}

