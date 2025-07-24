#ifndef ZEPHYR_DRIVERS_SENSOR_ROHM_BH1750_EMUL_H_
#define ZEPHYR_DRIVERS_SENSOR_ROHM_BH1750_EMUL_H_

#include <zephyr/device.h>
#include <zephyr/drivers/i2c_emul.h>
#include <zephyr/drivers/emul.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rohm_bh1750_emul_api {
    int (*set)(const struct device *dev, uint16_t data_raw);
};

struct rohm_bh1750_emul_cfg {
    uint16_t addr;
};

struct rohm_bh1750_emul_data {
    struct i2c_emul emul;
    const struct rohm_bh1750_emul_cfg *cfg;
    struct device *i2c;
    uint16_t data_raw;    /* Latest light measurement raw value */
    bool powered_on;
    bool cmd_ready;
    uint8_t last_cmd;
};

/**
 * @brief Set the raw measurement value in the emulator
 *
 * @param dev Emulator device
 * @param data_raw Raw measurement value (16-bit)
 * @return 0 on success, negative error code on failure
 */
static inline int rohm_bh1750_emul_set_measurement(const struct device *dev,
                                                   uint16_t data_raw)
{
    const struct rohm_bh1750_emul_api *api = (const struct rohm_bh1750_emul_api *)dev->api;

    if (!api || !api->set) {
        return -ENOTSUP;
    }

    return api->set(dev, data_raw);
}

/**
 * @brief Fetch a light measurement sample from the emulator and convert to lux
 *
 * @param emul Pointer to the emulator instance
 * @param lux Pointer to store converted lux value (can be NULL)
 * @return 0 on success, negative error code on failure
 */
int rohm_bh1750_emul_sample_fetch(const struct emul *emul, float *lux);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_DRIVERS_SENSOR_ROHM_BH1750_EMUL_H_ */

