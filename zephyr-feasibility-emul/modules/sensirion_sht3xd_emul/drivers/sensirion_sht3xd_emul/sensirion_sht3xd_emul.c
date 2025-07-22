#define DT_DRV_COMPAT sensirion_sht3xd_emul

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sht3xd_emul, CONFIG_I2C_LOG_LEVEL);

#include <zephyr/device.h>
#include <zephyr/drivers/emul.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/i2c_emul.h>
#include <zephyr/sys/byteorder.h>
#include <string.h>
#include <errno.h>

#include "sensirion_sht3xd_emul.h"

/* CRC-8 calculation as per Sensirion datasheet */
static uint8_t sht3xd_crc(uint8_t *data, size_t len)
{
	uint8_t crc = 0xFF;
	for (size_t i = 0; i < len; i++) {
		crc ^= data[i];
		for (uint8_t b = 0; b < 8; b++) {
			crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
		}
	}
	return crc;
}

/* I2C transfer handler */
static int sht3xd_emul_transfer(const struct emul *target,
				struct i2c_msg *msgs,
				int num_msgs,
				int addr)
{
	const struct sht3xd_emul_cfg *cfg = target->cfg;
	struct sht3xd_emul_data *data = target->data;

	if (cfg->addr != addr) {
		LOG_ERR("I2C address mismatch: expected 0x%02x, got 0x%02x", cfg->addr, addr);
		return -EIO;
	}

	if (num_msgs == 1 && !(msgs[0].flags & I2C_MSG_READ)) {
		// Write command
		if (msgs[0].len != 2) {
			LOG_ERR("Invalid command length");
			return -EIO;
		}
		memcpy(data->cmd_buf, msgs[0].buf, 2);
		data->cmd_ready = true;
		return 0;
	}

	if (num_msgs == 1 && (msgs[0].flags & I2C_MSG_READ) && data->cmd_ready) {
		uint16_t cmd = sys_get_be16(data->cmd_buf);
		data->cmd_ready = false;

		if (cmd == 0x2C06) {
			// Measurement command: return temp + humidity
			uint16_t temp = data->temp_raw;
			uint16_t hum = data->hum_raw;

			msgs[0].buf[0] = temp >> 8;
			msgs[0].buf[1] = temp & 0xFF;
			msgs[0].buf[2] = sht3xd_crc((uint8_t *)&temp, 2);

			msgs[0].buf[3] = hum >> 8;
			msgs[0].buf[4] = hum & 0xFF;
			msgs[0].buf[5] = sht3xd_crc((uint8_t *)&hum, 2);

			return 0;
		} else if (cmd == 0x2400) {
			// Read status register (OK)
			uint16_t status = 0x0000;
			msgs[0].buf[0] = status >> 8;
			msgs[0].buf[1] = status & 0xFF;
			msgs[0].buf[2] = sht3xd_crc((uint8_t *)&status, 2);
			return 0;
		} else if (cmd == 0x3041) {
			// Soft reset, nothing to return
			return 0;
		} else {
			LOG_WRN("Unsupported command: 0x%04x", cmd);
			return -EIO;
		}
	}

	LOG_ERR("Invalid I2C message sequence");
	return -EIO;
}

/* API method implementation */
static int sht3xd_emul_api_set(const struct device *dev, uint16_t temp_raw, uint16_t hum_raw)
{
	struct sht3xd_emul_data *data = dev->data;

	if (!data) {
		return -EINVAL;
	}

	data->temp_raw = temp_raw;
	data->hum_raw = hum_raw;

	return 0;
}

/* Exposed API struct */
static const struct sht3xd_emul_api sht3xd_emul_driver_api = {
	.set = sht3xd_emul_api_set,
};

/* I2C emulation API struct */
static struct i2c_emul_api sht3xd_emul_i2c_api = {
	.transfer = sht3xd_emul_transfer,
};

/* Emulator init function */
static int sht3xd_emul_init(const struct emul *target, const struct device *parent)
{
	struct sht3xd_emul_data *data = target->data;

	data->emul.api = &sht3xd_emul_i2c_api;
	data->emul.addr = ((const struct sht3xd_emul_cfg *)target->cfg)->addr;
	data->emul.target = target;
	data->i2c = parent;

	data->cmd_ready = false;
	data->temp_raw = 0x6666; // ~25°C
	data->hum_raw  = 0x8000; // ~50% RH

	// Bind the API to the device (used for Zephyr-style API access)
	//target->dev->api = &sht3xd_emul_driver_api;

	return 0;
}

/* Instantiation macro for each emulator node */
#define SHT3XD_EMUL_DEFINE(inst)                                           \
	static struct sht3xd_emul_data sht3xd_emul_data_##inst;              \
	static const struct sht3xd_emul_cfg sht3xd_emul_cfg_##inst = {       \
		.addr = DT_INST_REG_ADDR(inst),                                 \
	};                                                                    \
	EMUL_DT_INST_DEFINE(inst,                                            \
	                    sht3xd_emul_init,                                \
	                    &sht3xd_emul_data_##inst,                        \
	                    &sht3xd_emul_cfg_##inst,                         \
	                    &sht3xd_emul_i2c_api,                            \
	                    &sht3xd_emul_driver_api);

DT_INST_FOREACH_STATUS_OKAY(SHT3XD_EMUL_DEFINE)
