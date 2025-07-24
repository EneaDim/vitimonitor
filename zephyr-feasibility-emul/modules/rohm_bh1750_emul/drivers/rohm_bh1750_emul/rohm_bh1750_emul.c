#define DT_DRV_COMPAT rohm_bh1750_emul

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(rohm_bh1750_emul, CONFIG_I2C_LOG_LEVEL);

#include <zephyr/device.h>
#include <zephyr/drivers/emul.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/i2c_emul.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/random/random.h>
#include <string.h>
#include <errno.h>

#include "rohm_bh1750_emul.h"

static int rohm_bh1750_emul_transfer(const struct emul *target,
                                     struct i2c_msg *msgs, int num_msgs, int addr)
{
    const struct rohm_bh1750_emul_cfg *cfg = target->cfg;
    struct rohm_bh1750_emul_data *data = target->data;

    if (cfg->addr != addr) {
        LOG_ERR("I2C address mismatch");
        return -EIO;
    }

    // Handle command write (1-byte write)
    if (num_msgs == 1 && !(msgs[0].flags & I2C_MSG_READ)) {
        if (msgs[0].len != 1) {
            LOG_ERR("Invalid command length");
            return -EIO;
        }

        uint8_t cmd = msgs[0].buf[0];
        data->last_cmd = cmd;

        switch (cmd) {
            case 0x00: // Power down
                data->powered_on = false;
                data->cmd_ready = false;
                break;
            case 0x01: // Power on
                data->powered_on = true;
                break;
            case 0x07: // Reset (only works when powered on)
                if (data->powered_on) {
                    data->data_raw = 0;
                    data->cmd_ready = false;
                }
                break;
            // Measurement commands handled by sample_fetch, just mark ready here
            case 0x10: // Continuously H-Resolution Mode
            case 0x11: // Continuously H-Resolution Mode2
            case 0x13: // Continuously L-Resolution Mode
            case 0x20: // One Time H-Resolution Mode
            case 0x21: // One Time H-Resolution Mode2
            case 0x23: // One Time L-Resolution Mode
                if (!data->powered_on) {
                    LOG_ERR("Measurement command received while powered down");
                    return -EIO;
                }
                data->cmd_ready = true; // Indicate measurement requested
                break;
            default:
                LOG_WRN("Unsupported BH1750 command: 0x%02x", cmd);
                return -EIO;
        }

        return 0;
    }

    // Handle read (expecting 2 bytes)
    if (num_msgs == 1 && (msgs[0].flags & I2C_MSG_READ)) {
        if (!data->powered_on) {
            LOG_ERR("Read while powered down");
            return -EIO;
        }
        if (!data->cmd_ready) {
            LOG_ERR("Read requested but no measurement triggered");
            return -EIO;
        }
        if (msgs[0].len != 2) {
            LOG_ERR("Invalid read length, expected 2 bytes");
            return -EIO;
        }

        // Return the stored raw data MSB first
        msgs[0].buf[0] = data->data_raw >> 8;
        msgs[0].buf[1] = data->data_raw & 0xFF;

        // Clear cmd_ready flag for one-time modes (>= 0x20)
        if (data->last_cmd >= 0x20) {
            data->cmd_ready = false;
        }

        return 0;
    }

    LOG_ERR("Unsupported I2C message sequence");
    return -EIO;
}

static int rohm_bh1750_emul_api_set(const struct device *dev, uint16_t data_raw)
{
    struct rohm_bh1750_emul_data *data = dev->data;
    if (!data) return -EINVAL;

    data->data_raw = data_raw;
    data->cmd_ready = true;
    return 0;
}

static const struct rohm_bh1750_emul_api rohm_bh1750_emul_driver_api = {
    .set = rohm_bh1750_emul_api_set,
};

static struct i2c_emul_api rohm_bh1750_emul_i2c_api = {
    .transfer = rohm_bh1750_emul_transfer,
};

int rohm_bh1750_emul_sample_fetch(const struct emul *emul, float *lux)
{
    struct rohm_bh1750_emul_data *data = emul->data;

    if (!data->cmd_ready || !data->powered_on) {
        return -EIO;
    }

    // Generate random measurement here instead of in transfer()
    data->data_raw = 0x2000 + (sys_rand32_get() % 0x8000);

    // Convert to lux according to datasheet: lux = raw / 1.2
    if (lux) {
        *lux = data->data_raw / 1.2f;
    }

    return 0;
}

static int rohm_bh1750_emul_init(const struct emul *target, const struct device *parent)
{
    struct rohm_bh1750_emul_data *data = target->data;

    data->emul.api = &rohm_bh1750_emul_i2c_api;
    data->emul.addr = ((const struct rohm_bh1750_emul_cfg *)target->cfg)->addr;
    data->emul.target = target;
    data->i2c = (struct device *)parent;

    data->cmd_ready = false;
    data->powered_on = false;
    data->data_raw = 0x6666;  // Default dummy light level

    return 0;
}

#define ROHM_BH1750_EMUL(n) \
    static struct rohm_bh1750_emul_data rohm_bh1750_emul_data_##n; \
    static const struct rohm_bh1750_emul_cfg rohm_bh1750_emul_cfg_##n = { \
        .addr = DT_INST_REG_ADDR(n), \
    }; \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, \
        &rohm_bh1750_emul_data_##n, &rohm_bh1750_emul_cfg_##n, \
        POST_KERNEL, I2C_INIT_PRIORITY + 1, &rohm_bh1750_emul_driver_api); \
    EMUL_DT_INST_DEFINE(n, rohm_bh1750_emul_init, \
        &rohm_bh1750_emul_data_##n, &rohm_bh1750_emul_cfg_##n, \
        &rohm_bh1750_emul_i2c_api, &rohm_bh1750_emul_driver_api);

DT_INST_FOREACH_STATUS_OKAY(ROHM_BH1750_EMUL)

