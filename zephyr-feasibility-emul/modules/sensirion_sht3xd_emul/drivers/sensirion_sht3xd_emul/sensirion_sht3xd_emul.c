// Define the compatible string for this emulator driver
#define DT_DRV_COMPAT sensirion_sht3xd_emul

// Include Zephyr logging infrastructure and register a log module
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sht3xd_emul, CONFIG_I2C_LOG_LEVEL);

// Standard and Zephyr includes
#include <zephyr/device.h>
#include <zephyr/drivers/emul.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/i2c_emul.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/random/random.h>
#include <string.h>
#include <errno.h>

// Include the emulator-specific header file
#include "sensirion_sht3xd_emul.h"

// -----------------------------------------------------------------------------
// CRC-8 calculation function (used to simulate real SHT3x CRC protection)

static uint8_t sht3xd_crc(uint8_t *data, size_t len)
{
	uint8_t crc = 0xFF;  // Initial CRC value
	for (size_t i = 0; i < len; i++) {
		crc ^= data[i];  // XOR data byte into CRC
		for (uint8_t b = 0; b < 8; b++) {
			// Polynomial: 0x31 (x^8 + x^5 + x^4 + 1)
			crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
		}
	}
	return crc;  // Return final CRC value
}

// -----------------------------------------------------------------------------
// I2C transfer handler for the emulator

static int sht3xd_emul_transfer(const struct emul *target,
				struct i2c_msg *msgs,
				int num_msgs,
				int addr)
{
	const struct sht3xd_emul_cfg *cfg = target->cfg;
	struct sht3xd_emul_data *data = target->data;

	// Ensure I2C address matches what emulator is expecting
	if (cfg->addr != addr) {
		LOG_ERR("I2C address mismatch: expected 0x%02x, got 0x%02x", cfg->addr, addr);
		return -EIO;
	}

	// ✅ Handle the write-read combined transaction (most common case)
	if (num_msgs == 2 &&
	    !(msgs[0].flags & I2C_MSG_READ) &&  // First message is write
	    (msgs[1].flags & I2C_MSG_READ) &&   // Second message is read
	    msgs[0].len == 2 &&                 // Expecting 2-byte command
	    msgs[1].len == 6)                   // Expecting 6-byte response
	{
		uint16_t cmd = sys_get_be16(msgs[0].buf);  // Decode 16-bit command

		if (cmd == 0x2C06) {  // Measurement command
			uint16_t temp = data->temp_raw;
			uint16_t hum  = data->hum_raw;

			// Populate temperature (2 bytes) + CRC
			msgs[1].buf[0] = temp >> 8;
			msgs[1].buf[1] = temp & 0xFF;
			msgs[1].buf[2] = sht3xd_crc((uint8_t *)&temp, 2);

			// Populate humidity (2 bytes) + CRC
			msgs[1].buf[3] = hum >> 8;
			msgs[1].buf[4] = hum & 0xFF;
			msgs[1].buf[5] = sht3xd_crc((uint8_t *)&hum, 2);

			return 0;  // Successful emulated response
		}

		// Unsupported command in write-read format
		LOG_WRN("Unsupported command in write-read: 0x%04x", cmd);
		return -EIO;
	}

	// 🧱 Fallback: store command from single write message
	if (num_msgs == 1 && !(msgs[0].flags & I2C_MSG_READ)) {
		if (msgs[0].len != 2) {
			LOG_ERR("Invalid command length");
			return -EIO;
		}
		memcpy(data->cmd_buf, msgs[0].buf, 2);  // Save command
		data->cmd_ready = true;                 // Mark it ready
		return 0;
	}

	// 🧱 Fallback: respond to read following a previous write command
	if (num_msgs == 1 && (msgs[0].flags & I2C_MSG_READ) && data->cmd_ready) {
		uint16_t cmd = sys_get_be16(data->cmd_buf);
		data->cmd_ready = false;  // Consume command

		if (cmd == 0x2C06) {
			uint16_t temp = data->temp_raw;
			uint16_t hum  = data->hum_raw;

			msgs[0].buf[0] = temp >> 8;
			msgs[0].buf[1] = temp & 0xFF;
			msgs[0].buf[2] = sht3xd_crc((uint8_t *)&temp, 2);
			msgs[0].buf[3] = hum >> 8;
			msgs[0].buf[4] = hum & 0xFF;
			msgs[0].buf[5] = sht3xd_crc((uint8_t *)&hum, 2);

			return 0;
		}
	}

	// ❌ Invalid I2C message structure
	LOG_ERR("Invalid I2C message sequence");
	return -EIO;
}

// -----------------------------------------------------------------------------
// Emulator API: set raw temperature and humidity values

static int sht3xd_emul_api_set(const struct device *dev, uint16_t temp_raw, uint16_t hum_raw)
{
	struct sht3xd_emul_data *data = dev->data;
	if (!data) return -EINVAL;

	data->temp_raw = temp_raw;
	data->hum_raw  = hum_raw;

	return 0;
}

// Bind the above function to the emulator driver API
static const struct sht3xd_emul_api sht3xd_emul_driver_api = {
	.set = sht3xd_emul_api_set,
};

// Define the Zephyr I2C emulator API interface
static struct i2c_emul_api sht3xd_emul_i2c_api = {
	.transfer = sht3xd_emul_transfer,
};

// -----------------------------------------------------------------------------
// Public emulator API to simulate + fetch a new reading

int sht3xd_emul_sample_fetch(const struct emul *emul, float *temp_c, float *hum_pct)
{
	const struct sht3xd_emul_cfg *cfg = emul->cfg;
	struct sht3xd_emul_data *data = emul->data;

	// Generate random raw values in expected sensor range
	uint16_t temp_raw = 0x6000 + (sys_rand32_get() % 0x0800);
	uint16_t hum_raw  = 0x8000 + (sys_rand32_get() % 0x1000);

	// Set those values into the emulator device
	const struct sht3xd_emul_api *api =
		(const struct sht3xd_emul_api *) emul->dev->api;
	api->set(emul->dev, temp_raw, hum_raw);

	// Prepare command and response buffer
	uint8_t cmd[] = { 0x2C, 0x06 };   // Measurement command
	uint8_t buf[6];                   // Response buffer: Temp + CRC + Hum + CRC

	// Build I2C spec
	struct i2c_dt_spec i2c_spec = {
		.bus = data->i2c,
		.addr = cfg->addr,
	};

	// --------- Step 1: Send measurement command (I2C write) ----------
	int ret = i2c_write_dt(&i2c_spec, cmd, sizeof(cmd));
	if (ret < 0) {
		LOG_ERR("I2C write failed: %d", ret);
		return ret;
	}

	// --------- Step 2: Read 6 bytes of data from sensor (I2C read) ----------
	ret = i2c_read_dt(&i2c_spec, buf, sizeof(buf));
	if (ret < 0) {
		LOG_ERR("I2C read failed: %d", ret);
		return ret;
	}

	// Extract raw values
	uint16_t temp_val = (buf[0] << 8) | buf[1];
	uint16_t hum_val  = (buf[3] << 8) | buf[4];

	// Convert to human-readable units
	if (temp_c)
		*temp_c = -45 + 175 * ((float)temp_val / 65535.0f);
	if (hum_pct)
		*hum_pct = 100 * ((float)hum_val / 65535.0f);

	return 0;
}

// -----------------------------------------------------------------------------
// Emulator initialization

static int sht3xd_emul_init(const struct emul *target, const struct device *parent)
{
	struct sht3xd_emul_data *data = target->data;

	// Setup emulator's internal I2C emulation parameters
	data->emul.api = &sht3xd_emul_i2c_api;
	data->emul.addr = ((const struct sht3xd_emul_cfg *)target->cfg)->addr;
	data->emul.target = target;
	data->i2c = parent;

	// Set default dummy values
	data->cmd_ready = false;
	data->temp_raw = 0x6666;  // ~25°C
	data->hum_raw  = 0x8000;  // ~50% RH

	return 0;
}

// -----------------------------------------------------------------------------
// Instantiate emulator device from devicetree

#define SHT3XD_EMUL(n)                                                    \
	static struct sht3xd_emul_data sht3xd_emul_data_##n;                  \
	static const struct sht3xd_emul_cfg sht3xd_emul_cfg_##n = {          \
		.addr = DT_INST_REG_ADDR(n),                                     \
	};                                                                    \
	DEVICE_DT_INST_DEFINE(n, NULL, NULL,                                  \
		&sht3xd_emul_data_##n, &sht3xd_emul_cfg_##n,                      \
		POST_KERNEL, I2C_INIT_PRIORITY + 1, &sht3xd_emul_driver_api);    \
	EMUL_DT_INST_DEFINE(n, sht3xd_emul_init,                              \
		&sht3xd_emul_data_##n, &sht3xd_emul_cfg_##n,                      \
		&sht3xd_emul_i2c_api, &sht3xd_emul_driver_api);

// Automatically expand emulator instances for each matching DT node
DT_INST_FOREACH_STATUS_OKAY(SHT3XD_EMUL)

