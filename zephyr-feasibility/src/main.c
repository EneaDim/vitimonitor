#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>  // For rand()
#include <zephyr/sys/util.h>
#include <zephyr/sys/__assert.h>

// Sensor data structure
struct sensor_data {
    struct sensor_value temperature;
    struct sensor_value humidity_air;
    struct sensor_value humidity_soil;
    struct sensor_value luminosity;
};

// Configuration variables
static bool enable_compression = true;  // Enable/disable compression
static int sensor_read_interval = 10;   // Sensor read interval in seconds

// Thread stacks and data
K_THREAD_STACK_DEFINE(sensor_stack, 1024);
K_THREAD_STACK_DEFINE(lora_stack, 1024);
K_THREAD_STACK_DEFINE(power_stack, 512);

static struct k_thread sensor_thread_data;
static struct k_thread lora_thread_data;
static struct k_thread power_thread_data;

static struct sensor_data shared_data;
static struct k_sem data_ready_sem;  // Semaphore to sync sensor reading and LoRa sending

// Generate random sensor value (fallback, if needed)
static struct sensor_value generate_random_value(void) {
    struct sensor_value value;
    value.val1 = rand() % 40 + 10;  // 10..50
    value.val2 = rand() % 1000000;  // fractional part
    return value;
}

// Helper: sample sensor and get channel value, fallback to random if sensor not ready
static int fetch_sensor_data(const struct device *dev, enum sensor_channel chan, struct sensor_value *val) {
    if (!device_is_ready(dev)) {
        // printk("Sensor device %s not ready, using mock value\n", dev->name);
        *val = generate_random_value();
        return 0;
    }
    int ret = sensor_sample_fetch(dev);
    if (ret < 0) {
        printk("Failed to fetch sample from %s: %d\n", dev->name, ret);
        return ret;
    }
    ret = sensor_channel_get(dev, chan, val);
    if (ret < 0) {
        printk("Failed to get channel %d from %s: %d\n", chan, dev->name, ret);
    }
    return ret;
}

// Thread: read sensors periodically and update shared data
void read_sensors(void *arg1, void *arg2, void *arg3) {
    const struct device *temp_sensor = DEVICE_DT_GET(DT_NODELABEL(dht1));
    const struct device *soil_sensor = DEVICE_DT_GET(DT_NODELABEL(soil_moisture));
    const struct device *light_sensor = DEVICE_DT_GET(DT_NODELABEL(bh1750));

    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    printk("Sensor thread started\n");

    while (1) {
        int ret;

        ret = fetch_sensor_data(temp_sensor, SENSOR_CHAN_AMBIENT_TEMP, &shared_data.temperature);
        if (ret == 0) {
            printk("Temperature: %d.%06d C\n", shared_data.temperature.val1, shared_data.temperature.val2);
        }

        ret = fetch_sensor_data(temp_sensor, SENSOR_CHAN_HUMIDITY, &shared_data.humidity_air);
        if (ret == 0) {
            printk("Air Humidity: %d.%06d %%\n", shared_data.humidity_air.val1, shared_data.humidity_air.val2);
        }

        ret = fetch_sensor_data(soil_sensor, SENSOR_CHAN_HUMIDITY, &shared_data.humidity_soil);
        if (ret == 0) {
            printk("Soil Humidity: %d.%06d %%\n", shared_data.humidity_soil.val1, shared_data.humidity_soil.val2);
        }

        ret = fetch_sensor_data(light_sensor, SENSOR_CHAN_LIGHT, &shared_data.luminosity);
        if (ret == 0) {
            printk("Luminosity: %d.%06d lux\n", shared_data.luminosity.val1, shared_data.luminosity.val2);
        }

        // Notify LoRa thread that new data is ready
        k_sem_give(&data_ready_sem);

        k_sleep(K_SECONDS(sensor_read_interval));
    }
}

// Simple compression example (store only val1 high and low bytes)
static void compress_data(const struct sensor_data *data, uint8_t *compressed_data) {
    compressed_data[0] = (data->temperature.val1 >> 8) & 0xFF;
    compressed_data[1] = data->temperature.val1 & 0xFF;
    compressed_data[2] = (data->humidity_air.val1 >> 8) & 0xFF;
    compressed_data[3] = data->humidity_air.val1 & 0xFF;
    compressed_data[4] = (data->humidity_soil.val1 >> 8) & 0xFF;
    compressed_data[5] = data->humidity_soil.val1 & 0xFF;
    compressed_data[6] = (data->luminosity.val1 >> 8) & 0xFF;
    compressed_data[7] = data->luminosity.val1 & 0xFF;
}

// LoRa send (stub)
static void lora_send(uint8_t *data, size_t len) {
    printk("Sending data via LoRa: ");
    for (size_t i = 0; i < len; i++) {
        printk("%02x ", data[i]);
    }
    printk("\n");
}

// Thread: send sensor data over LoRa when available
void send_lora(void *arg1, void *arg2, void *arg3) {
    uint8_t buffer[8];

    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    printk("LoRa thread started\n");

    while (1) {
        // Wait until sensor data is ready
        k_sem_take(&data_ready_sem, K_FOREVER);

        if (enable_compression) {
            compress_data(&shared_data, buffer);
            lora_send(buffer, sizeof(buffer));
        } else {
            // Send raw (val1 only)
            buffer[0] = (shared_data.temperature.val1 >> 8) & 0xFF;
            buffer[1] = shared_data.temperature.val1 & 0xFF;
            buffer[2] = (shared_data.humidity_air.val1 >> 8) & 0xFF;
            buffer[3] = shared_data.humidity_air.val1 & 0xFF;
            buffer[4] = (shared_data.humidity_soil.val1 >> 8) & 0xFF;
            buffer[5] = shared_data.humidity_soil.val1 & 0xFF;
            buffer[6] = (shared_data.luminosity.val1 >> 8) & 0xFF;
            buffer[7] = shared_data.luminosity.val1 & 0xFF;

            lora_send(buffer, sizeof(buffer));
        }
    }
}

// Power management thread (dummy)
void manage_power(void *arg1, void *arg2, void *arg3) {
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    printk("Power management thread started\n");
    while (1) {
        // Placeholder for power management logic
        k_sleep(K_SECONDS(10));
    }
}

int main(void) {
    printk("Application started\n");

    // Init semaphore
    k_sem_init(&data_ready_sem, 0, 1);

    // Create threads
    k_thread_create(&sensor_thread_data, sensor_stack, K_THREAD_STACK_SIZEOF(sensor_stack),
                    read_sensors, NULL, NULL, NULL, 7, 0, K_NO_WAIT);

    k_thread_create(&lora_thread_data, lora_stack, K_THREAD_STACK_SIZEOF(lora_stack),
                    send_lora, NULL, NULL, NULL, 6, 0, K_NO_WAIT);

    k_thread_create(&power_thread_data, power_stack, K_THREAD_STACK_SIZEOF(power_stack),
                    manage_power, NULL, NULL, NULL, 5, 0, K_NO_WAIT);

    // Main loop does nothing, threads run independently
    while (1) {
        k_sleep(K_SECONDS(60));
    }

    return 0;
}

