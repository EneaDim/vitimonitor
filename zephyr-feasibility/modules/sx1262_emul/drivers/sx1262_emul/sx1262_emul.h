#ifndef SX1262_EMUL_H_
#define SX1262_EMUL_H_

#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/emul.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sx1262_config {
    const struct device *spi_bus;
    struct spi_config spi_cfg;
};

struct sx1262_data {
    // Buffer di simulazione/emulazione
    uint8_t tx_buf[256];
    uint8_t rx_buf[256];
    size_t tx_len;
    size_t rx_len;
};

// API driver
struct sx1262_emul_driver_api {
    int (*send)(const struct device *dev, const uint8_t *data, size_t len);
    int (*recv)(const struct device *dev, uint8_t *data, size_t max_len);
};

int sx1262_send(const struct device *dev, const uint8_t *data, size_t len);
int sx1262_recv(const struct device *dev, uint8_t *data, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif // SX1262_EMUL_H_

