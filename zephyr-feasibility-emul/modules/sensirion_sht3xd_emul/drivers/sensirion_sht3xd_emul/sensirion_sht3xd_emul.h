#ifndef ZEPHYR_DRIVERS_SENSOR_SHT3XD_EMUL_H_
#define ZEPHYR_DRIVERS_SENSOR_SHT3XD_EMUL_H_

#include <zephyr/device.h>
#include <zephyr/drivers/i2c_emul.h>  // Required for struct i2c_emul
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief API for SHT3XD emulator
 */
struct sht3xd_emul_api {
	/**
	 * @brief Set the raw temperature and humidity registers
	 *
	 * @param dev Emulator device pointer
	 * @param temp_raw 16-bit raw temperature
	 * @param hum_raw  16-bit raw humidity
	 *
	 * @return 0 on success, negative errno on failure
	 */
	int (*set)(const struct device *dev, uint16_t temp_raw, uint16_t hum_raw);
};

/**
 * @brief Emulator configuration (I2C address)
 */
struct sht3xd_emul_cfg {
	uint16_t addr;
};

/**
 * @brief Emulator runtime data
 */
struct sht3xd_emul_data {
	struct i2c_emul emul;            /**< I2C emulator interface */
	const struct device *i2c;        /**< I2C controller this emulator is attached to */

	uint8_t cmd_buf[2];              /**< Buffer for last received command */
	bool cmd_ready;                  /**< Flag indicating command is ready */

	uint16_t temp_raw;               /**< Raw temperature data */
	uint16_t hum_raw;                /**< Raw humidity data */
};

/**
 * @brief Helper to set raw measurement values using the emulator API
 *
 * @param dev Device pointer to emulator (must support sht3xd_emul_api)
 * @param temp_raw Raw temperature
 * @param hum_raw Raw humidity
 *
 * @return 0 on success, -ENOTSUP or -EINVAL on failure
 */
static inline int sht3xd_emul_set_measurement(const struct device *dev,
					      uint16_t temp_raw,
					      uint16_t hum_raw)
{
	const struct sht3xd_emul_api *api = (const struct sht3xd_emul_api *)dev->api;

	if (api == NULL || api->set == NULL) {
		return -ENOTSUP;
	}

	return api->set(dev, temp_raw, hum_raw);
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_DRIVERS_SENSOR_SHT3XD_EMUL_H_ */

