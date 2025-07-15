#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>  // For rand()

// Sensor definitions
#define SENSOR_TEMP "DHT22"
#define SENSOR_MOISTURE "Soil_Moisture"
#define SENSOR_LIGHT "BH1750"

// Sensor data structure
struct sensor_data {
    struct sensor_value temperature;
    struct sensor_value humidity_air;
    struct sensor_value humidity_soil;
    struct sensor_value luminosity;
};

// Configuration variables
bool enable_compression = true;  // Variable to enable/disable compression
int sensor_read_interval = 10;   // Sensor read interval in seconds

// Thread definitions
K_THREAD_STACK_DEFINE(sensor_stack, 1024);
K_THREAD_STACK_DEFINE(lora_stack, 1024);
K_THREAD_STACK_DEFINE(power_stack, 512);

struct k_thread sensor_thread_data;
struct k_thread lora_thread_data;
struct k_thread power_thread_data;

// Function to generate random sensor values
struct sensor_value generate_random_value(void) {
    struct sensor_value value;
    value.val1 = rand() % 40 + 10;  // Random value between 10 and 50
    value.val2 = rand() % 1000000;  // Random decimal part
    return value;
}

// Mock function to simulate sensor data fetch
int mock_sensor_sample_fetch(const struct device *dev) {
    // Simulate sensor sample fetch by generating random values for sensors
    printk("Fetching mock data for sensor: %s\n", dev->name);
    return 0;  // Success
}

// Mock function to simulate getting data from sensors
int mock_sensor_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val) {
    if (chan == SENSOR_CHAN_AMBIENT_TEMP) {
        *val = generate_random_value();
        return 0;
    } else if (chan == SENSOR_CHAN_HUMIDITY) {
        *val = generate_random_value();
        return 0;
    } else if (chan == SENSOR_CHAN_LIGHT) {
        *val = generate_random_value();
        return 0;
    }
    return -EINVAL;  // Error if channel not recognized
}

// LoRa send function (stub for sending data via LoRa)
void lora_send(uint8_t *data, size_t len) {
    printk("Sending data via LoRa: ");
    for (int i = 0; i < len; i++) {
        printk("%02x ", data[i]);
    }
    printk("\n");
}

// Sensor reading function
void read_sensors(void *arg1, void *arg2, void *arg3) {
    struct sensor_data *data = (struct sensor_data *)arg1;
    const struct device *temp_sensor = DEVICE_DT_GET(DT_NODELABEL(dht1));
    const struct device *soil_sensor = DEVICE_DT_GET(DT_NODELABEL(soil_moisture));
    const struct device *light_sensor = DEVICE_DT_GET(DT_NODELABEL(bh1750));

    if (!device_is_ready(temp_sensor)) {
        printk("Temperature sensor not ready\n");
        return;
    }

    if (!device_is_ready(soil_sensor)) {
        printk("Soil moisture sensor not ready\n");
        return;
    }

    if (!device_is_ready(light_sensor)) {
        printk("Light sensor not ready\n");
        return;
    }

    printk("All sensors ready\n");

    while (1) {
        int ret;
        ret = mock_sensor_sample_fetch(temp_sensor);  // Using the mock
        if (ret != 0) {
            printk("Sensor sample fetch failed with code: %d\n", ret);
            return;
        }

        // Read sensor values
        if (mock_sensor_channel_get(temp_sensor, SENSOR_CHAN_AMBIENT_TEMP, &data->temperature) < 0) {
            printk("Cannot get temperature\n");
        } else {
            printk("Temperature: %d.%06d C\n", data->temperature.val1, data->temperature.val2);
        }

        if (mock_sensor_channel_get(temp_sensor, SENSOR_CHAN_HUMIDITY, &data->humidity_air) < 0) {
            printk("Cannot get humidity\n");
        } else {
            printk("Humidity: %d.%06d %%\n", data->humidity_air.val1, data->humidity_air.val2);
        }

        if (mock_sensor_channel_get(soil_sensor, SENSOR_CHAN_HUMIDITY, &data->humidity_soil) < 0) {
            printk("Cannot get soil humidity\n");
        } else {
            printk("Soil humidity: %d.%06d %%\n", data->humidity_soil.val1, data->humidity_soil.val2);
        }

        if (mock_sensor_channel_get(light_sensor, SENSOR_CHAN_LIGHT, &data->luminosity) < 0) {
            printk("Cannot get luminosity\n");
        } else {
            printk("Luminosity: %d.%06d lux\n", data->luminosity.val1, data->luminosity.val2);
        }

        k_sleep(K_SECONDS(sensor_read_interval));  // Sleep between sensor readings
    }
}

// Function to compress sensor data (simple example)
void compress_data(struct sensor_data *data, uint8_t *compressed_data) {
    compressed_data[0] = data->temperature.val1 >> 8;
    compressed_data[1] = data->temperature.val1 & 0xFF;
    compressed_data[2] = data->humidity_air.val1 >> 8;
    compressed_data[3] = data->humidity_air.val1 & 0xFF;
    compressed_data[4] = data->humidity_soil.val1 >> 8;
    compressed_data[5] = data->humidity_soil.val1 & 0xFF;
    compressed_data[6] = data->luminosity.val1 >> 8;
    compressed_data[7] = data->luminosity.val1 & 0xFF;
}

// LoRa send function with data compression
void send_lora(void *arg1, void *arg2, void *arg3) {
    struct sensor_data *data = (struct sensor_data *)arg1;
    uint8_t compressed_data[8];
    uint8_t raw_data[8];

    if (enable_compression) {
        compress_data(data, compressed_data);
        lora_send(compressed_data, sizeof(compressed_data));  // Send compressed data
    } else {
        raw_data[0] = data->temperature.val1 >> 8;
        raw_data[1] = data->temperature.val1 & 0xFF;
        raw_data[2] = data->humidity_air.val1 >> 8;
        raw_data[3] = data->humidity_air.val1 & 0xFF;
        raw_data[4] = data->humidity_soil.val1 >> 8;
        raw_data[5] = data->humidity_soil.val1 & 0xFF;
        raw_data[6] = data->luminosity.val1 >> 8;
        raw_data[7] = data->luminosity.val1 & 0xFF;

        lora_send(raw_data, sizeof(raw_data));  // Send raw data
    }
}

// Power management function
void manage_power(void *arg1, void *arg2, void *arg3) {
    while (1) {
        k_sleep(K_SECONDS(10));  // Sleep for power management
    }
}

int main(void) {
    struct sensor_data data;

    // Create threads for sensor reading, LoRa sending, and power management
    k_thread_create(&sensor_thread_data, sensor_stack, K_THREAD_STACK_SIZEOF(sensor_stack),
                    read_sensors, &data, NULL, NULL, 7, 0, K_NO_WAIT);

    k_thread_create(&lora_thread_data, lora_stack, K_THREAD_STACK_SIZEOF(lora_stack),
                    send_lora, &data, NULL, NULL, 6, 0, K_NO_WAIT);

    k_thread_create(&power_thread_data, power_stack, K_THREAD_STACK_SIZEOF(power_stack),
                    manage_power, NULL, NULL, NULL, 5, 0, K_NO_WAIT);

    // Main loop to keep the application running
    while (1) {
        k_sleep(K_SECONDS(sensor_read_interval));  // Sleep between sensor reads
    }

    return 0;
}

