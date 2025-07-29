#define DT_DRV_COMPAT rohm_bh1750_emul

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>     // API sensor standard
#include <zephyr/drivers/emul.h>
#include <zephyr/drivers/i2c_emul.h>   // Emulazione I2C
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

LOG_MODULE_REGISTER(bh1750_emul, CONFIG_SENSOR_LOG_LEVEL);

// -----------------------------------------------------------------------------
// Strutture dati del driver

// Stato dinamico del sensore emulato
struct bh1750_emul_data {
	uint16_t raw_lux;         // Valore grezzo di luminosità
	bool powered_on;          // Stato accensione
	bool measurement_ready;   // Indica se è disponibile un valore da leggere
};

// Configurazione statica (da devicetree)
struct bh1750_emul_config {
	uint16_t addr;            // Indirizzo I2C
};

// -----------------------------------------------------------------------------
// Conversione RAW → LUX secondo datasheet BH1750

static float raw_to_lux(uint16_t raw)
{
	return raw / 1.2f;
}

// -----------------------------------------------------------------------------
// Funzione fetch: simula una nuova lettura

static int bh1750_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	struct bh1750_emul_data *data = dev->data;

	ARG_UNUSED(chan);

	if (!data->powered_on) {
		return -EIO;
	}

	// Genera valore raw pseudo-random realistico
	data->raw_lux = 0x2000 + (sys_rand32_get() % 0x1000);
	data->measurement_ready = true;

	return 0;
}

// -----------------------------------------------------------------------------
// Funzione channel_get: converte valore grezzo in `sensor_value`

static int bh1750_channel_get(const struct device *dev,
                              enum sensor_channel chan,
                              struct sensor_value *val)
{
	struct bh1750_emul_data *data = dev->data;

	if (chan != SENSOR_CHAN_LIGHT) {
		return -ENOTSUP;
	}

	if (!data->measurement_ready) {
		return -EIO;
	}

	float lux = raw_to_lux(data->raw_lux);
	sensor_value_from_double(val, lux);

	return 0;
}

// -----------------------------------------------------------------------------
// API standard sensor_driver_api per Zephyr

static const struct sensor_driver_api bh1750_emul_driver_api = {
	.sample_fetch = bh1750_sample_fetch,
	.channel_get  = bh1750_channel_get,
};

// -----------------------------------------------------------------------------
// Emulazione I2C per test automatici

static int bh1750_emul_i2c_transfer(const struct emul *target,
                                    struct i2c_msg *msgs,
                                    int num_msgs,
                                    int addr)
{
	const struct bh1750_emul_config *cfg = target->cfg;
	struct bh1750_emul_data *data = target->data;

	if (cfg->addr != addr) {
		return -EIO;
	}

	// Scrittura di un comando (1 byte)
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
		case 0x07:  // Reset
			if (data->powered_on) {
				data->raw_lux = 0;
				data->measurement_ready = false;
			}
			break;
		case 0x10: case 0x11: case 0x13:  // Continuous mode
		case 0x20: case 0x21: case 0x23:  // One-time mode
			if (!data->powered_on) {
				return -EIO;
			}
			data->measurement_ready = true;
			break;
		default:
			return -EIO;
		}
		return 0;
	}

	// Lettura dati: 2 byte con valore grezzo
	if (num_msgs == 1 && (msgs[0].flags & I2C_MSG_READ)) {
		if (!data->powered_on || !data->measurement_ready) {
			return -EIO;
		}
		if (msgs[0].len != 2) {
			return -EIO;
		}

		msgs[0].buf[0] = data->raw_lux >> 8;
		msgs[0].buf[1] = data->raw_lux & 0xFF;

		// Dopo lettura, svuota dato per one-time
		data->measurement_ready = false;
		return 0;
	}

	// Messaggio non supportato
	return -EIO;
}

// Tabella per Zephyr I2C emulator
static struct i2c_emul_api bh1750_emul_i2c_api = {
	.transfer = bh1750_emul_i2c_transfer,
};

// -----------------------------------------------------------------------------
// Inizializzazione emulatore

static int bh1750_emul_init(const struct emul *target, const struct device *parent)
{
	struct bh1750_emul_data *data = target->data;

	data->powered_on = false;
	data->measurement_ready = false;
	data->raw_lux = 0x6666;  // Valore fittizio iniziale

	return 0;
}

// -----------------------------------------------------------------------------
// Macro di istanziazione da devicetree

#define BH1750_EMUL(n)                                                      \
	static struct bh1750_emul_data bh1750_emul_data_##n;                    \
	static const struct bh1750_emul_config bh1750_emul_cfg_##n = {          \
		.addr = DT_INST_REG_ADDR(n),                                        \
	};                                                                       \
	DEVICE_DT_INST_DEFINE(n, NULL, NULL,                                    \
		&bh1750_emul_data_##n, &bh1750_emul_cfg_##n,                        \
		POST_KERNEL, I2C_INIT_PRIORITY + 1, &bh1750_emul_driver_api);              \
	EMUL_DT_INST_DEFINE(n, bh1750_emul_init,                                \
		&bh1750_emul_data_##n, &bh1750_emul_cfg_##n,                        \
		&bh1750_emul_i2c_api, &bh1750_emul_driver_api);

// Istanzia tutte le istanze abilitate nel devicetree
DT_INST_FOREACH_STATUS_OKAY(BH1750_EMUL)

