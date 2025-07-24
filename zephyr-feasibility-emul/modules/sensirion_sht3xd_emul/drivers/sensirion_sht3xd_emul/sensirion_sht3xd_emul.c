// Set driver compatibility string for devicetree binding
#define DT_DRV_COMPAT sensirion_sht3xd_emul

// Logging for this module
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sht3xd_emul, CONFIG_I2C_LOG_LEVEL); // Register log with dynamic log level

// Zephyr and standard headers
#include <zephyr/device.h>          // Device model
#include <zephyr/drivers/emul.h>    // Emulator driver interface
#include <zephyr/devicetree.h>      // Devicetree macros
#include <zephyr/drivers/i2c.h>     // I2C API
#include <zephyr/drivers/i2c_emul.h>// I2C emulator API
#include <zephyr/sys/byteorder.h>   // Endian conversion
#include <string.h>                 // memcpy
#include <errno.h>                  // Standard error codes

// Header for this emulator's API
#include "sensirion_sht3xd_emul.h"

// ----------------------------------------------------------------------------
// CRC-8 calculation function (per Sensirion SHT3x datasheet)

static uint8_t sht3xd_crc(uint8_t *data, size_t len)
{
	uint8_t crc = 0xFF;  // Initial value
	for (size_t i = 0; i < len; i++) {
		crc ^= data[i];  // XOR input byte with CRC
		for (uint8_t b = 0; b < 8; b++) {
			// Polynomial 0x31 (x^8 + x^5 + x^4 + 1)
			crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
		}
	}
	return crc; // Return final CRC-8 value
}

// ----------------------------------------------------------------------------
// I2C transfer emulation handler

static int sht3xd_emul_transfer(const struct emul *target,
				struct i2c_msg *msgs,
				int num_msgs,
				int addr)
{
	const struct sht3xd_emul_cfg *cfg = target->cfg;   // Emulator config
	struct sht3xd_emul_data *data = target->data;      // Emulator data

	// Validate I2C address
	if (cfg->addr != addr) {
		LOG_ERR("I2C address mismatch: expected 0x%02x, got 0x%02x", cfg->addr, addr);
		return -EIO;
	}

	// Handle command write (1 msg, write only)
	if (num_msgs == 1 && !(msgs[0].flags & I2C_MSG_READ)) {
		if (msgs[0].len != 2) { // Expecting a 2-byte command
			LOG_ERR("Invalid command length");
			return -EIO;
		}
		memcpy(data->cmd_buf, msgs[0].buf, 2); // Store the command
		data->cmd_ready = true;                // Mark command as ready
		return 0;
	}

	// Handle read after command was written
	if (num_msgs == 1 && (msgs[0].flags & I2C_MSG_READ) && data->cmd_ready) {
		uint16_t cmd = sys_get_be16(data->cmd_buf); // Read 16-bit command
		data->cmd_ready = false;                    // Clear command flag

		if (cmd == 0x2C06) {
			// High repeatability measurement request
			uint16_t temp = data->temp_raw;
			uint16_t hum  = data->hum_raw;

			// Write temperature + CRC to buffer
			msgs[0].buf[0] = temp >> 8;
			msgs[0].buf[1] = temp & 0xFF;
			msgs[0].buf[2] = sht3xd_crc((uint8_t *)&temp, 2);

			// Write humidity + CRC to buffer
			msgs[0].buf[3] = hum >> 8;
			msgs[0].buf[4] = hum & 0xFF;
			msgs[0].buf[5] = sht3xd_crc((uint8_t *)&hum, 2);

			return 0;
		} else if (cmd == 0x2400) {
			// Read status register (returns 0x0000)
			uint16_t status = 0x0000;
			msgs[0].buf[0] = status >> 8;
			msgs[0].buf[1] = status & 0xFF;
			msgs[0].buf[2] = sht3xd_crc((uint8_t *)&status, 2);
			return 0;
		} else if (cmd == 0x3041) {
			// Soft reset command: do nothing
			return 0;
		} else {
			// Unsupported command
			LOG_WRN("Unsupported command: 0x%04x", cmd);
			return -EIO;
		}
	}

	// Invalid message sequence (e.g., missing write or wrong format)
	LOG_ERR("Invalid I2C message sequence");
	return -EIO;
}

// ----------------------------------------------------------------------------
// Emulator API: set temperature and humidity values

static int sht3xd_emul_api_set(const struct device *dev, uint16_t temp_raw, uint16_t hum_raw)
{
	struct sht3xd_emul_data *data = dev->data;

	if (!data) {
		return -EINVAL;
	}

	data->temp_raw = temp_raw;  // Set temperature raw value
	data->hum_raw  = hum_raw;   // Set humidity raw value

	return 0;
}

// ----------------------------------------------------------------------------
// Emulator's exposed API struct

static const struct sht3xd_emul_api sht3xd_emul_driver_api = {
	.set = sht3xd_emul_api_set, // Only one method: set()
};

// ----------------------------------------------------------------------------
// I2C emulator API for Zephyr's I2C emul framework

static struct i2c_emul_api sht3xd_emul_i2c_api = {
	.transfer = sht3xd_emul_transfer, // Handle all I2C transactions
};

// ----------------------------------------------------------------------------
// Initialization function for emulator instance

static int sht3xd_emul_init(const struct emul *target, const struct device *parent)
{
	struct sht3xd_emul_data *data = target->data;

	// Set emulator parameters
	data->emul.api = &sht3xd_emul_i2c_api;  // Register I2C transfer handler
	data->emul.addr = ((const struct sht3xd_emul_cfg *)target->cfg)->addr;
	data->emul.target = target;             // Back-reference to emulator
	data->i2c = parent;                     // Parent I2C device

	// Set default measurement values
	data->cmd_ready = false;
	data->temp_raw = 0x6666; // ~25°C
	data->hum_raw  = 0x8000; // ~50% RH

	// Optionally bind API to dev (commented out for now)
	// target->dev->api = &sht3xd_emul_driver_api;

	return 0;
}

// ----------------------------------------------------------------------------
// Macro to instantiate emulator instance from devicetree

#define SHT3XD_EMUL(n)                                                    \
    static struct sht3xd_emul_data sht3xd_emul_data_##n;                 /* Emulator state */ \
    static const struct sht3xd_emul_cfg sht3xd_emul_cfg_##n = {         /* Emulator config */ \
        .addr = DT_INST_REG_ADDR(n),                                    /* I2C address from DT */ \
    };                                                                   \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL,                                 /* Create dummy device */ \
            &sht3xd_emul_data_##n, &sht3xd_emul_cfg_##n,                \
            POST_KERNEL, I2C_INIT_PRIORITY + 1, &sht3xd_emul_driver_api); \
    EMUL_DT_INST_DEFINE(n, sht3xd_emul_init,                             /* Register emulator */ \
            &sht3xd_emul_data_##n, &sht3xd_emul_cfg_##n,                 \
            &sht3xd_emul_i2c_api, &sht3xd_emul_driver_api);

// Instantiate emulator for each enabled instance in devicetree
DT_INST_FOREACH_STATUS_OKAY(SHT3XD_EMUL)

