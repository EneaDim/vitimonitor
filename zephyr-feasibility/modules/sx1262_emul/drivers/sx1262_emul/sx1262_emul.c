#define DT_DRV_COMPAT sx1262_emul

#include "sx1262_emul.h"

#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/emul.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <string.h>

// Livello di log configurabile via prj.conf
LOG_MODULE_REGISTER(sx1262_emul, CONFIG_SPI_LOG_LEVEL);

// Fallback per SPI_MODE_0 se non definito
#ifndef SPI_MODE_0
#define SPI_MODE_0 0
#endif

// --- API reali ---

int sx1262_send(const struct device *dev, const uint8_t *data, size_t len)
{
    const struct sx1262_config *cfg = dev->config;
    const struct spi_config *spi_cfg = &cfg->spi_cfg;

    struct spi_buf tx_buf = { .buf = (uint8_t *)data, .len = len };
    struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };

    return spi_write(cfg->spi_bus, spi_cfg, &tx_set);
}

int sx1262_recv(const struct device *dev, uint8_t *data, size_t max_len)
{
    const struct sx1262_config *cfg = dev->config;
    const struct spi_config *spi_cfg = &cfg->spi_cfg;

    struct spi_buf rx_buf = { .buf = data, .len = max_len };
    struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

    return spi_read(cfg->spi_bus, spi_cfg, &rx_set);
}

// --- Driver API ---
static const struct sx1262_emul_driver_api sx1262_emul_driver_api = {
    .send = sx1262_send,
    .recv = sx1262_recv,
};

// --- Inizializzazione driver reale ---
static int sx1262_real_init(const struct device *dev)
{
    struct sx1262_data *data = dev->data;
    const struct sx1262_config *cfg = dev->config;

    data->tx_len = 0;
    data->rx_len = 0;

    ((struct sx1262_config *)cfg)->spi_cfg = (struct spi_config){
        .frequency = 1000000,
        .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_0,
        .slave = DT_REG_ADDR(DT_DRV_INST(0)),
        .cs = NULL,
    };

    return 0;
}

// --- Emulazione SPI ---
static int sx1262_emul_io(const struct emul *emul,
                                  const struct spi_config *spi_cfg,
                                  const struct spi_buf_set *tx_bufs,
                                  const struct spi_buf_set *rx_bufs)
{
    struct sx1262_data *data = emul->data;

    if (tx_bufs && tx_bufs->count > 0) {
        const struct spi_buf *buf = &tx_bufs->buffers[0];
        memcpy(data->tx_buf, buf->buf, buf->len);
        data->tx_len = buf->len;
        LOG_INF("EMUL TX: %d bytes", buf->len);
    }

    if (rx_bufs && rx_bufs->count > 0) {
        struct spi_buf *buf = &rx_bufs->buffers[0];
        memset(buf->buf, 0x42, buf->len); // Dummy data
        data->rx_len = buf->len;
        LOG_INF("EMUL RX: %d bytes", buf->len);
    }

    return 0;
}

static const struct spi_emul_api sx1262_emul_spi_api = {
    .io = sx1262_emul_io,
};

static int sx1262_emul_init(const struct emul *emul, const struct device *parent)
{
    struct sx1262_data *data = emul->data;
    data->tx_len = 0;
    data->rx_len = 0;
    return 0;
}

// --- Istanziazione DRIVER + EMULATORE ---
#define SX1262_EMUL(n)                                                              \
    static struct sx1262_data sx1262_data_##n;                                      \
    static struct sx1262_config sx1262_cfg_##n = {                                  \
        .spi_bus = DEVICE_DT_GET(DT_INST_BUS(n)),                                   \
    };                                                                               \
                                                                                    \
    DEVICE_DT_INST_DEFINE(n,                                                        \
        sx1262_real_init, NULL,                                                     \
        &sx1262_data_##n, &sx1262_cfg_##n,                                          \
        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,                            \
        &sx1262_emul_driver_api);                                                   \
                                                                                    \
    EMUL_DT_INST_DEFINE(n,                                                          \
        sx1262_emul_init,                                                           \
        &sx1262_data_##n, &sx1262_cfg_##n,                                          \
        &sx1262_emul_spi_api, &sx1262_emul_driver_api);

// Istanzia tutti i nodi OK nel devicetree
DT_INST_FOREACH_STATUS_OKAY(SX1262_EMUL)

