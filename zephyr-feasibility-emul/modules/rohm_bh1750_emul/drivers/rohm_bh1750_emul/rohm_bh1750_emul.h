#ifndef ZEPHYR_DRIVERS_SENSOR_ROHM_BH1750_EMUL_H_
#define ZEPHYR_DRIVERS_SENSOR_ROHM_BH1750_EMUL_H_

// -----------------------------------------------------------------------------
// Zephyr core includes

#include <zephyr/device.h>
#include <zephyr/drivers/emul.h>
#include <zephyr/drivers/i2c_emul.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BH1750 Emulator API
 *
 * Espone un'API opzionale per test o manipolazione manuale del sensore emulato.
 */
struct rohm_bh1750_emul_api {
	// Metodo per impostare manualmente un valore RAW di luminosità
	int (*set)(const struct device *dev, uint16_t raw_lux);
};

/**
 * @brief Configurazione statica dell'emulatore (da Devicetree)
 */
struct rohm_bh1750_emul_config {
	uint16_t addr;  ///< I2C address assegnato via devicetree
};

/**
 * @brief Dati dinamici dell'emulatore (runtime state)
 */
struct rohm_bh1750_emul_data {
	struct i2c_emul emul;        ///< Struttura base Zephyr I2C emulator
	const struct device *i2c;    ///< Puntatore al controller I2C

	uint16_t raw_lux;            ///< Valore RAW attuale (16-bit MSB first)
	bool powered_on;             ///< Stato accensione
	bool measurement_ready;      ///< Flag che indica lettura disponibile
};

/**
 * @brief Helper per impostare un valore RAW da codice di test
 *
 * Questo è utile nei test automatici (ztest) per iniettare valori noti.
 *
 * @param dev       Puntatore al device BH1750
 * @param raw_lux   Valore grezzo simulato da restituire (0–65535)
 * @return 0 se OK, altrimenti errore negativo
 */
static inline int rohm_bh1750_emul_set_lux_raw(const struct device *dev, uint16_t raw_lux)
{
	const struct rohm_bh1750_emul_api *api =
		(const struct rohm_bh1750_emul_api *)dev->api;

	if (!api || !api->set) {
		return -ENOTSUP;
	}

	return api->set(dev, raw_lux);
}

/**
 * @brief Simula una lettura e restituisce valore convertito in lux
 *
 * Utile per testare direttamente l’emulatore senza usare `sensor_sample_fetch`.
 *
 * @param emul  Puntatore all'istanza emulator
 * @param lux   Output: luminosità in lux
 * @return 0 se ok, errore negativo altrimenti
 */
int rohm_bh1750_emul_sample_fetch(const struct emul *emul, float *lux);

#ifdef __cplusplus
}
#endif

#endif  // ZEPHYR_DRIVERS_SENSOR_ROHM_BH1750_EMUL_H_

