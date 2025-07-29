// Imposta il nome 'compatible' per il devicetree
#define DT_DRV_COMPAT sensirion_sht3xd_emul

// Include il modulo di log Zephyr
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sht3xd_emul, CONFIG_SENSOR_LOG_LEVEL);

// Include Zephyr: periferiche, sensori, emulazione
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/emul.h>
#include <zephyr/drivers/i2c_emul.h>
#include <zephyr/sys/util.h>
#include <zephyr/random/random.h>

// === Strutture dati dell'emulatore ===

// Dati dinamici associati a ciascuna istanza
struct sht3xd_emul_data {
	uint16_t raw_temp;   // valore grezzo della temperatura (16 bit)
	uint16_t raw_hum;    // valore grezzo dell'umidità relativa (16 bit)
	struct emul emul;    // struttura Zephyr per l'emulazione I2C
	const struct device *i2c; // bus I2C associato (per simulare letture)
};

// Configurazione statica (dal devicetree)
struct sht3xd_emul_cfg {
	uint16_t addr;  // indirizzo I2C
};

// === Funzioni di conversione raw → fisico ===

// Conversione raw a temperatura in °C (formula da datasheet SHT3x)
static float raw_to_temp(uint16_t raw)
{
	return -45.0f + 175.0f * ((float)raw / 65535.0f);
}

// Conversione raw a umidità relativa in %
static float raw_to_hum(uint16_t raw)
{
	return 100.0f * ((float)raw / 65535.0f);
}

// === API driver Zephyr standard (sensor_driver_api) ===

// Funzione chiamata da Zephyr per generare una nuova lettura
static int sht3xd_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	struct sht3xd_emul_data *data = dev->data;

	ARG_UNUSED(chan); // ignoriamo il canale, generiamo tutto insieme

	// Genera nuovi valori pseudo-random per test
	data->raw_temp = 0x6666 + (sys_rand32_get() % 0x1000);  // ~25°C ± range
	data->raw_hum  = 0x8000 + (sys_rand32_get() % 0x1000);  // ~50% ± range

	return 0;  // successo
}

// Funzione chiamata da Zephyr per ottenere i valori richiesti
static int sht3xd_channel_get(const struct device *dev,
                              enum sensor_channel chan,
                              struct sensor_value *val)
{
	struct sht3xd_emul_data *data = dev->data;

	// Gestione della temperatura
	if (chan == SENSOR_CHAN_AMBIENT_TEMP) {
		float t = raw_to_temp(data->raw_temp);
		sensor_value_from_double(val, t);
		return 0;
	}

	// Gestione dell'umidità relativa
	if (chan == SENSOR_CHAN_HUMIDITY) {
		float h = raw_to_hum(data->raw_hum);
		sensor_value_from_double(val, h);
		return 0;
	}

	// Qualsiasi altro canale non è supportato
	return -ENOTSUP;
}

// Tabella di funzioni del driver Zephyr (sensor API)
static const struct sensor_driver_api sht3xd_emul_driver_api = {
	.sample_fetch = sht3xd_sample_fetch,
	.channel_get  = sht3xd_channel_get,
};

// === Emulazione I2C per test/unit-test/QEMU ===

// Funzione di trasferimento I2C simulata (richiesta da Zephyr `i2c_emul_api`)
static int sht3xd_emul_i2c_transfer(const struct emul *emul,
                                    struct i2c_msg *msgs,
                                    int num_msgs,
                                    int addr)
{
	const struct sht3xd_emul_cfg *cfg = emul->cfg;
	struct sht3xd_emul_data *data = emul->data;

	// Controlla indirizzo I2C corretto
	if (cfg->addr != addr) {
		return -EIO;
	}

	// Simula risposta a comando 0x2C06 (measure high repeatability)
	if (num_msgs == 2 &&
	    !(msgs[0].flags & I2C_MSG_READ) && // write
	    (msgs[1].flags & I2C_MSG_READ) &&  // read
	    msgs[0].len == 2 &&
	    msgs[1].len == 6) {

		uint16_t cmd = (msgs[0].buf[0] << 8) | msgs[0].buf[1];

		if (cmd == 0x2C06) {
			// Scrive temperatura + CRC
			msgs[1].buf[0] = data->raw_temp >> 8;
			msgs[1].buf[1] = data->raw_temp & 0xFF;
			msgs[1].buf[2] = 0x00; // CRC ignorato per semplicità

			// Scrive umidità + CRC
			msgs[1].buf[3] = data->raw_hum >> 8;
			msgs[1].buf[4] = data->raw_hum & 0xFF;
			msgs[1].buf[5] = 0x00;

			return 0;
		}
	}

	// Comando non riconosciuto
	return -EIO;
}

// Tabella per Zephyr per la simulazione I2C
static struct i2c_emul_api sht3xd_emul_i2c_api = {
	.transfer = sht3xd_emul_i2c_transfer,
};

// === Inizializzazione del dispositivo ===

static int sht3xd_emul_init(const struct emul *target, const struct device *parent)
{
	struct sht3xd_emul_data *data = target->data;

	// Inizializza valori dummy
	data->raw_temp = 0x6666;
	data->raw_hum  = 0x8000;

	return 0;
}

// === Macro per instanziare dispositivi da devicetree ===

#define SHT3XD_EMUL(n)                                                       \
	static struct sht3xd_emul_data sht3xd_emul_data_##n;                     \
	static const struct sht3xd_emul_cfg sht3xd_emul_cfg_##n = {              \
		.addr = DT_INST_REG_ADDR(n),                                         \
	};                                                                       \
	/* Definizione del dispositivo come driver sensor standard */            \
	DEVICE_DT_INST_DEFINE(n, NULL, NULL,                                     \
		&sht3xd_emul_data_##n, &sht3xd_emul_cfg_##n,                         \
		POST_KERNEL, I2C_INIT_PRIORITY + 1,                                  \
		&sht3xd_emul_driver_api);                                            \
	/* Definizione del dispositivo come emulatore I2C per test */            \
	EMUL_DT_INST_DEFINE(n, sht3xd_emul_init,                                 \
		&sht3xd_emul_data_##n, &sht3xd_emul_cfg_##n,                         \
		&sht3xd_emul_i2c_api, &sht3xd_emul_driver_api);

// Istanzia il driver/emulatore per ogni nodo nel devicetree
DT_INST_FOREACH_STATUS_OKAY(SHT3XD_EMUL)

