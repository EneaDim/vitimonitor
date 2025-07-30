// Definisce l'etichetta per il driver emulato (devicetree binding)
#define DT_DRV_COMPAT rohm_bh1750_emul

// -------------------------
// Include Zephyr e standard
// -------------------------
#include <zephyr/device.h>              // Gestione dispositivi Zephyr
#include <zephyr/drivers/sensor.h>      // API standard per sensori
#include <zephyr/drivers/emul.h>        // Supporto emulatori Zephyr
#include <zephyr/drivers/i2c_emul.h>    // Supporto specifico per emulazione I2C
#include <zephyr/logging/log.h>         // Logging
#include <zephyr/random/random.h>       // Generazione numeri pseudo-random

// Registra il modulo di log per il driver
LOG_MODULE_REGISTER(bh1750_emul, CONFIG_SENSOR_LOG_LEVEL);

// -----------------------------------------------------------------------------
// STRUTTURE DATI
// -----------------------------------------------------------------------------

// Struttura di runtime: stato dinamico del sensore emulato
struct bh1750_emul_data {
	uint16_t raw_lux;         // Valore grezzo simulato (da 0 a 0xFFFF)
	bool powered_on;          // Indica se il sensore è acceso
	bool measurement_ready;   // Se è disponibile una nuova misura
};

// Struttura di configurazione: viene da devicetree
struct bh1750_emul_config {
	uint16_t addr;            // Indirizzo I2C del sensore
};

// -----------------------------------------------------------------------------
// CONVERSIONE RAW → LUX
// -----------------------------------------------------------------------------

// Converti il valore grezzo secondo il datasheet BH1750 (lux = raw / 1.2)
static float raw_to_lux(uint16_t raw)
{
	return raw / 1.2f;
}

// -----------------------------------------------------------------------------
// sample_fetch: simula acquisizione da parte del sensore
// -----------------------------------------------------------------------------

static int bh1750_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	struct bh1750_emul_data *data = dev->data;

	// Ignora il tipo di canale richiesto
	ARG_UNUSED(chan);

	// Se il sensore è spento, errore di I/O
	if (!data->powered_on) {
		return -EIO;
	}

	// Genera un valore pseudo-random valido
	data->raw_lux = 0x2000 + (sys_rand32_get() % 0x1000);
	data->measurement_ready = true;

	return 0;
}

// -----------------------------------------------------------------------------
// channel_get: restituisce valore convertito in struct sensor_value
// -----------------------------------------------------------------------------

static int bh1750_channel_get(const struct device *dev,
                              enum sensor_channel chan,
                              struct sensor_value *val)
{
	struct bh1750_emul_data *data = dev->data;

	// Supporta solo canale "light" (illuminamento)
	if (chan != SENSOR_CHAN_LIGHT) {
		return -ENOTSUP;
	}

	// Se non è disponibile una nuova misura, errore
	if (!data->measurement_ready) {
		return -EIO;
	}

	// Converte in lux e lo inserisce nella struct Zephyr
	float lux = raw_to_lux(data->raw_lux);
	sensor_value_from_double(val, lux);

	return 0;
}

// -----------------------------------------------------------------------------
// Struttura delle API standard per sensor_driver_api
// -----------------------------------------------------------------------------

static const struct sensor_driver_api bh1750_emul_driver_api = {
	.sample_fetch = bh1750_sample_fetch,   // Funzione di acquisizione
	.channel_get  = bh1750_channel_get,    // Funzione per leggere il valore
};

// -----------------------------------------------------------------------------
// EMULAZIONE I2C: gestisce comandi scritti e letture dal sensore
// -----------------------------------------------------------------------------

static int bh1750_emul_i2c_transfer(const struct emul *target,
                                    struct i2c_msg *msgs,
                                    int num_msgs,
                                    int addr)
{
	const struct bh1750_emul_config *cfg = target->cfg;
	struct bh1750_emul_data *data = target->data;

	// Se l'indirizzo non corrisponde, fallisce
	if (cfg->addr != addr) {
		return -EIO;
	}

	// Scrittura di un comando I2C (1 byte)
	if (num_msgs == 1 && !(msgs[0].flags & I2C_MSG_READ)) {
		uint8_t cmd = msgs[0].buf[0];

		switch (cmd) {
		case 0x00:  // Power down
			data->powered_on = false;
			data->measurement_ready = false;
			break;

		case 0x01:  // Power on
			data->powered_on = true;
			break;

		case 0x07:  // Reset (solo se acceso)
			if (data->powered_on) {
				data->raw_lux = 0;
				data->measurement_ready = false;
			}
			break;

		// Comandi validi di misurazione continua o singola
		case 0x10: case 0x11: case 0x13:  // Continuous mode
		case 0x20: case 0x21: case 0x23:  // One-time mode
			if (!data->powered_on) {
				return -EIO;
			}
			data->measurement_ready = true;
			break;

		default:  // Comando non valido
			return -EIO;
		}

		return 0;
	}

	// Lettura dei dati simulati (2 byte: MSB + LSB)
	if (num_msgs == 1 && (msgs[0].flags & I2C_MSG_READ)) {
		if (!data->powered_on || !data->measurement_ready) {
			return -EIO;
		}
		if (msgs[0].len != 2) {
			return -EIO;
		}

		msgs[0].buf[0] = data->raw_lux >> 8;       // MSB
		msgs[0].buf[1] = data->raw_lux & 0xFF;     // LSB

		// Se è una misura "one-shot", azzera lo stato
		data->measurement_ready = false;
		return 0;
	}

	// Formato non supportato
	return -EIO;
}

// Struttura di API I2C per emulatore
static struct i2c_emul_api bh1750_emul_i2c_api = {
	.transfer = bh1750_emul_i2c_transfer,
};

// -----------------------------------------------------------------------------
// Inizializzazione dell’emulatore
// -----------------------------------------------------------------------------

static int bh1750_emul_init(const struct emul *target, const struct device *parent)
{
	struct bh1750_emul_data *data = target->data;

	data->powered_on = false;
	data->measurement_ready = false;
	data->raw_lux = 0x6666;  // Valore iniziale arbitrario

	return 0;
}

// -----------------------------------------------------------------------------
// MACRO PER ISTANZIARE DRIVER ED EMULATORE DA DEVICETREE
// -----------------------------------------------------------------------------

#define BH1750_EMUL(n)                                                      \
	static struct bh1750_emul_data bh1750_emul_data_##n;                    \
	static const struct bh1750_emul_config bh1750_emul_cfg_##n = {          \
		.addr = DT_INST_REG_ADDR(n),                                        \
	};                                                                       \
	DEVICE_DT_INST_DEFINE(n, NULL, NULL,                                    \
		&bh1750_emul_data_##n, &bh1750_emul_cfg_##n,                        \
		POST_KERNEL, I2C_INIT_PRIORITY + 1, &bh1750_emul_driver_api);       \
	EMUL_DT_INST_DEFINE(n, bh1750_emul_init,                                \
		&bh1750_emul_data_##n, &bh1750_emul_cfg_##n,                        \
		&bh1750_emul_i2c_api, &bh1750_emul_driver_api);

// Istanzia tutti i nodi abilitati (status = "okay") dal devicetree
DT_INST_FOREACH_STATUS_OKAY(BH1750_EMUL)

