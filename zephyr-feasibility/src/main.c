#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>  // For rand()
#include <zephyr/sys/util.h>
#include <zephyr/sys/__assert.h>

// Sensor data structure (only temperature)
struct sensor_data {
  struct sensor_value temperature;  // Only temperature value
};

// Configuration variables
static bool enable_compression = true;  // Enable/disable data compression before transmission
static int sensor_read_interval = 10;  // Sensor read interval in seconds

// Thread stacks and data
K_THREAD_STACK_DEFINE(sensor_stack, 1024);  // Stack size for the sensor thread
K_THREAD_STACK_DEFINE(lora_stack, 1024);  // Stack size for the LoRa thread
K_THREAD_STACK_DEFINE(power_stack, 512);  // Stack size for the power management thread

static struct k_thread sensor_thread_data;  // Data structure for the sensor thread
static struct k_thread lora_thread_data;  // Data structure for the LoRa thread
static struct k_thread power_thread_data;  // Data structure for the power management thread

static struct sensor_data shared_data;   // Shared data between threads
static struct k_sem data_ready_sem;    // Semaphore to synchronize data access between threads

// Generate random sensor value (fallback, if needed)
static struct sensor_value generate_random_value(void) {
  struct sensor_value value;
  value.val1 = rand() % 40 + 10;  // Random temperature between 10 and 50
  value.val2 = rand() % 1000000;  // Random fractional part (microseconds)
  return value;
}

// Helper function: Sample sensor data and get channel value
// If sensor not ready, fallback to a random value
static int fetch_sensor_data(const struct device *dev, enum sensor_channel chan, struct sensor_value *val) {
  if (!device_is_ready(dev)) {  // Check if the device is ready
    *val = generate_random_value();  // Generate random data if the sensor is not ready
    return 0;  // No error, return 0
  }
  int ret = sensor_sample_fetch(dev);  // Fetch sample from the sensor
  if (ret < 0) {
    printk("Failed to fetch sample from %s: %d\n", dev->name, ret);  // Print error if fetch fails
    return ret;
  }
  ret = sensor_channel_get(dev, chan, val);  // Get the requested channel value (e.g., temperature)
  if (ret < 0) {
    printk("Failed to get channel %d from %s: %d\n", chan, dev->name, ret);  // Print error if fetching channel fails
  }
  return ret;
}

// Thread to read temperature sensor periodically and update shared data
void read_sensors(void *arg1, void *arg2, void *arg3) {
  const struct device *temp_sensor = DEVICE_DT_GET(DT_NODELABEL(dht1));  // Fetch temperature sensor device

  ARG_UNUSED(arg2);  // Unused arguments
  ARG_UNUSED(arg3);

  printk("Sensor thread started\n");

  while (1) {
    int ret;

    // Fetch temperature data from the sensor
    ret = fetch_sensor_data(temp_sensor, SENSOR_CHAN_AMBIENT_TEMP, &shared_data.temperature);
    if (ret == 0) {
      printk("Temperature: %d.%06d C\n", shared_data.temperature.val1, shared_data.temperature.val2);  // Print temperature data
    }

    // Notify LoRa thread that new data is ready
    k_sem_give(&data_ready_sem);

    // Wait for the next interval before reading again
    k_sleep(K_SECONDS(sensor_read_interval));
  }
}

// Simple compression function (store only the high and low bytes of val1)
static void compress_data(const struct sensor_data *data, uint8_t *compressed_data) {
  compressed_data[0] = (data->temperature.val1 >> 8) & 0xFF;  // Extract the high byte of temperature
  compressed_data[1] = data->temperature.val1 & 0xFF;    // Extract the low byte of temperature
}

// LoRa send function (stub)
static void lora_send(uint8_t *data, size_t len) {
  printk("Sending data via LoRa: ");
  for (size_t i = 0; i < len; i++) {
    printk("%02x ", data[i]);  // Print the data to be sent
  }
  printk("\n");
}

// Thread to send sensor data over LoRa when available
void send_lora(void *arg1, void *arg2, void *arg3) {
  uint8_t buffer[2];  // Buffer to store compressed sensor data

  ARG_UNUSED(arg1);  // Unused arguments
  ARG_UNUSED(arg2);
  ARG_UNUSED(arg3);

  printk("LoRa thread started\n");

  while (1) {
    // Wait until sensor data is ready
    k_sem_take(&data_ready_sem, K_FOREVER);

    // If compression is enabled, compress the data
    if (enable_compression) {
      compress_data(&shared_data, buffer);
      lora_send(buffer, sizeof(buffer));  // Send the compressed data over LoRa
    } else {
      // If compression is disabled, send raw data (only val1 of temperature)
      buffer[0] = (shared_data.temperature.val1 >> 8) & 0xFF;
      buffer[1] = shared_data.temperature.val1 & 0xFF;

      lora_send(buffer, sizeof(buffer));  // Send raw data over LoRa
    }
  }
}

// Power management thread
void manage_power(void *arg1, void *arg2, void *arg3) {
  printk("Power management thread started\n");

  while (1) {
    // Suspend unused threads to save power
    k_thread_suspend(&sensor_thread_data);  // Suspend the sensor thread
    k_thread_suspend(&lora_thread_data);  // Suspend the LoRa thread

    // Enter low-power state (system will sleep here)
    printk("Entering low-power state\n");
    k_sleep(K_FOREVER);  // System will stay in low power mode

    // Once the system wakes up, resume the threads
    printk("Waking up from low-power state\n");
    k_thread_resume(&sensor_thread_data);  // Resume the sensor thread
    k_thread_resume(&lora_thread_data);  // Resume the LoRa thread
  }
}

int main(void) {
  printk("Application started\n");

  // Initialize semaphore to synchronize sensor data availability between threads
  k_sem_init(&data_ready_sem, 0, 1);

  // Create threads for sensor reading, LoRa sending, and power management
  k_thread_create(&sensor_thread_data, sensor_stack, K_THREAD_STACK_SIZEOF(sensor_stack),
          read_sensors, NULL, NULL, NULL, 7, 0, K_NO_WAIT);

  k_thread_create(&lora_thread_data, lora_stack, K_THREAD_STACK_SIZEOF(lora_stack),
          send_lora, NULL, NULL, NULL, 6, 0, K_NO_WAIT);

  k_thread_create(&power_thread_data, power_stack, K_THREAD_STACK_SIZEOF(power_stack),
          manage_power, NULL, NULL, NULL, 5, 0, K_NO_WAIT);

  // Main loop does nothing; the threads run independently
  while (1) {
    k_sleep(K_FOREVER);  // Yield control to idle thread to save power
  }

  return 0;  // Return (unreachable code)
}

