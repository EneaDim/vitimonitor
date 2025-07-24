#ifndef ZEPHYR_DRIVERS_SENSOR_SHT3XD_EMUL_H_  // Header include guard: start
#define ZEPHYR_DRIVERS_SENSOR_SHT3XD_EMUL_H_

// Include Zephyr device APIs
#include <zephyr/device.h>

// Include base I2C emulator structures and macros
#include <zephyr/drivers/i2c_emul.h>

// Include Zephyr's emulator abstraction layer
#include <zephyr/drivers/emul.h>

// Standard headers for fixed-width types and booleans
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {  // Ensure C linkage when included from C++ code
#endif

/**
 * @brief Emulator API structure for SHT3XD
 * This allows test code to set raw sensor data manually via function pointer.
 */
struct sht3xd_emul_api {
	// Function pointer to set raw temp/humidity in the emulator
	int (*set)(const struct device *dev, uint16_t temp_raw, uint16_t hum_raw);
};

/**
 * @brief Emulator static configuration
 * This holds constant config like the I2C address.
 */
struct sht3xd_emul_cfg {
	uint16_t addr;  // I2C address assigned via devicetree
};

/**
 * @brief Emulator runtime data
 * This holds internal state and last command sent, plus raw sensor data.
 */
struct sht3xd_emul_data {
	struct i2c_emul emul;       // Zephyr's base I2C emulator structure
	const struct device *i2c;   // Pointer to I2C controller the emulator is on

	uint8_t cmd_buf[2];         // Stores the last command bytes received
	bool cmd_ready;             // Indicates a valid command was received

	uint16_t temp_raw;          // Raw temperature data to return to host
	uint16_t hum_raw;           // Raw humidity data to return to host
};

/**
 * @brief Helper function to invoke emulator API and set measurement
 *
 * This is a convenience wrapper that calls the .set() method from the API.
 */
static inline int sht3xd_emul_set_measurement(const struct device *dev,
					      uint16_t temp_raw,
					      uint16_t hum_raw)
{
	// Cast device API to emulator API structure
	const struct sht3xd_emul_api *api = (const struct sht3xd_emul_api *)dev->api;

	// Fail if no API or missing .set method
	if (!api || !api->set) {
		return -ENOTSUP;
	}

	// Call the emulator’s set method to update values
	return api->set(dev, temp_raw, hum_raw);
}

/**
 * @brief Public function to fetch a simulated sensor measurement
 *
 * This simulates a full I2C transaction (write command + read response),
 * including CRC generation, and returns values in degrees Celsius and %RH.
 *
 * @param emul     Pointer to emulator instance
 * @param temp_c   Output: temperature in Celsius
 * @param hum_pct  Output: relative humidity in percent
 * @return 0 on success, or negative error code
 */
int sht3xd_emul_sample_fetch(const struct emul *emul, float *temp_c, float *hum_pct);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_DRIVERS_SENSOR_SHT3XD_EMUL_H_ */  // Header include guard: end

