// Protezione da inclusione multipla del file header
#ifndef ZEPHYR_DRIVERS_SENSOR_ROHM_BH1750_EMUL_H_
#define ZEPHYR_DRIVERS_SENSOR_ROHM_BH1750_EMUL_H_

// -----------------------------------------------------------------------------
// INCLUDES DI SISTEMA E ZEPHYR
// -----------------------------------------------------------------------------

#include <zephyr/device.h>          // API generica per dispositivi
#include <zephyr/drivers/emul.h>    // Supporto per emulatori generici
#include <zephyr/drivers/i2c_emul.h>// Supporto specifico per emulazione I2C
#include <stdint.h>                 // Tipi standard (uint16_t, ecc.)
#include <stdbool.h>                // Tipo booleano

// Compatibilità con C++
#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// API ESTESA DELL’EMULATORE (accessibile da test o codice utente)
// -----------------------------------------------------------------------------

/**
 * @brief BH1750 Emulator API
 *
 * API opzionale usata per test automatici o configurazioni manuali dell’emulatore.
 */
struct rohm_bh1750_emul_api {
	/**
	 * Metodo per impostare un valore grezzo di luminosità
	 *
	 * @param dev      Puntatore al device BH1750
	 * @param raw_lux  Valore RAW simulato da restituire
	 * @return 0 se OK, -ENOTSUP se non implementato
	 */
	int (*set)(const struct device *dev, uint16_t raw_lux);
};

// -----------------------------------------------------------------------------
// STRUTTURE DATI EMULATORE
// -----------------------------------------------------------------------------

/**
 * @brief Configurazione statica dell'emulatore
 *
 * Questa configurazione è solitamente generata automaticamente dal Devicetree.
 */
struct rohm_bh1750_emul_config {
	uint16_t addr;  ///< Indirizzo I2C del sensore
};

/**
 * @brief Stato dinamico dell’emulatore durante l’esecuzione
 *
 * Contiene le informazioni interne utilizzate per simulare il comportamento del BH1750.
 */
struct rohm_bh1750_emul_data {
	struct i2c_emul emul;        ///< Base dell'emulatore I2C (Zephyr)
	const struct device *i2c;    ///< Puntatore al controller I2C (master)

	uint16_t raw_lux;            ///< Ultimo valore RAW generato o impostato
	bool powered_on;             ///< Stato di accensione del sensore
	bool measurement_ready;      ///< Flag per indicare se è disponibile una misura
};

// -----------------------------------------------------------------------------
// FUNZIONI DI UTILITÀ ESPORTATE
// -----------------------------------------------------------------------------

/**
 * @brief Imposta manualmente un valore RAW simulato nel sensore
 *
 * Utile nei test (es. ztest) per simulare un determinato livello di luminosità
 * senza passare dalla logica di misura automatica.
 *
 * @param dev       Puntatore al dispositivo emulato
 * @param raw_lux   Valore RAW (0–65535) da simulare
 * @return 0 se riuscito, -ENOTSUP se non supportato
 */
static inline int rohm_bh1750_emul_set_lux_raw(const struct device *dev, uint16_t raw_lux)
{
	// Ottiene il puntatore all'API implementata
	const struct rohm_bh1750_emul_api *api =
		(const struct rohm_bh1750_emul_api *)dev->api;

	// Se l'API non è disponibile o la funzione "set" non è definita
	if (!api || !api->set) {
		return -ENOTSUP;
	}

	// Chiama la funzione "set" definita dall'emulatore
	return api->set(dev, raw_lux);
}

/**
 * @brief Simula una misura e restituisce il valore convertito in lux
 *
 * Funzione alternativa a `sensor_sample_fetch()` per usare direttamente l’emulatore.
 * È utile nei test di basso livello dove si vuole bypassare l’API sensor.
 *
 * @param emul  Puntatore alla struttura emulatore
 * @param lux   Puntatore dove salvare il risultato in lux
 * @return 0 se misura disponibile, altrimenti errore negativo
 */
int rohm_bh1750_emul_sample_fetch(const struct emul *emul, float *lux);

// -----------------------------------------------------------------------------
// Fine del blocco C++
// -----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

// Fine della protezione del file
#endif  // ZEPHYR_DRIVERS_SENSOR_ROHM_BH1750_EMUL_H_

