#ifndef ZEPHYR_DRIVERS_SENSOR_SHT3XD_EMUL_H_
#define ZEPHYR_DRIVERS_SENSOR_SHT3XD_EMUL_H_

#include <zephyr/device.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief API per impostare manualmente i valori raw nel sensore emulato.
 */
struct sht3xd_emul_api {
	int (*set)(const struct device *dev, uint16_t temp_raw, uint16_t hum_raw);
};

/**
 * @brief Funzione helper per test: imposta i valori raw da iniettare.
 *
 * Può essere chiamata da un test/unit test per sovrascrivere i valori simulati.
 */
static inline int sht3xd_emul_set_measurement(const struct device *dev,
					      uint16_t temp_raw,
					      uint16_t hum_raw)
{
	const struct sht3xd_emul_api *api = (const struct sht3xd_emul_api *)dev->api;

	if (!api || !api->set) {
		return -ENOTSUP;
	}
	return api->set(dev, temp_raw, hum_raw);
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_DRIVERS_SENSOR_SHT3XD_EMUL_H_ */

