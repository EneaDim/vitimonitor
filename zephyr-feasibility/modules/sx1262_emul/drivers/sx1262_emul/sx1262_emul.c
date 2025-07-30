// Definisce l'identificativo del driver per l'emulatore nel devicetree
#define DT_DRV_COMPAT sx1262_emul

// Include il file header locale dell’emulatore SX1262
#include "sx1262_emul.h"

// Include di sistema e Zephyr
#include <zephyr/device.h>          // API per gestire i device Zephyr
#include <zephyr/drivers/spi.h>     // API per il bus SPI
#include <zephyr/drivers/emul.h>    // Supporto agli emulatori di device
#include <zephyr/init.h>            // Macro per l'inizializzazione dei device
#include <zephyr/logging/log.h>     // Logging Zephyr
#include <string.h>                 // Funzioni standard di stringa

// Include POSIX per invio pacchetti UDP (solo lato emulatore)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Registra un modulo di log con nome "sx1262_emul" e livello definito in prj.conf
LOG_MODULE_REGISTER(sx1262_emul, CONFIG_SPI_LOG_LEVEL);

// Se SPI_MODE_0 non è definito, lo definisce come 0 (default)
#ifndef SPI_MODE_0
#define SPI_MODE_0 0
#endif

// IP e porta di destinazione per i pacchetti UDP (localhost)
#define UDP_DEST_IP   "127.0.0.1"
#define UDP_DEST_PORT 17000

// ------------------------
// Funzione helper per inviare pacchetti via UDP
// ------------------------
static int udp_send_packet(const uint8_t *data, size_t len)
{
    int sockfd;
    struct sockaddr_in dest_addr;
    int ret;

    // Crea un socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        LOG_ERR("UDP socket creation failed");
        return -1;
    }

    // Inizializza l'indirizzo del destinatario
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(UDP_DEST_PORT);
    inet_pton(AF_INET, UDP_DEST_IP, &dest_addr.sin_addr);

    // Invia il pacchetto via UDP
    ret = sendto(sockfd, data, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (ret < 0) {
        LOG_ERR("UDP sendto failed");
        close(sockfd);
        return -1;
    }

    // Chiude il socket
    close(sockfd);
    return 0;
}

// ------------------------
// API reali: invio su SPI
// ------------------------
int sx1262_send(const struct device *dev, const uint8_t *data, size_t len)
{
    const struct sx1262_config *cfg = dev->config;
    const struct spi_config *spi_cfg = &cfg->spi_cfg;

    // Crea buffer SPI di trasmissione
    struct spi_buf tx_buf = { .buf = (uint8_t *)data, .len = len };
    struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };

    // Scrive via SPI
    return spi_write(cfg->spi_bus, spi_cfg, &tx_set);
}

// ------------------------
// API reali: ricezione su SPI
// ------------------------
int sx1262_recv(const struct device *dev, uint8_t *data, size_t max_len)
{
    const struct sx1262_config *cfg = dev->config;
    const struct spi_config *spi_cfg = &cfg->spi_cfg;

    // Crea buffer SPI di ricezione
    struct spi_buf rx_buf = { .buf = data, .len = max_len };
    struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

    // Legge via SPI
    return spi_read(cfg->spi_bus, spi_cfg, &rx_set);
}

// ------------------------
// Struttura API del driver SX1262
// ------------------------
static const struct sx1262_emul_driver_api sx1262_emul_driver_api = {
    .send = sx1262_send,
    .recv = sx1262_recv,
};

// ------------------------
// Inizializzazione driver reale
// ------------------------
static int sx1262_real_init(const struct device *dev)
{
    struct sx1262_data *data = dev->data;
    const struct sx1262_config *cfg = dev->config;

    // Inizializza i buffer a 0
    data->tx_len = 0;
    data->rx_len = 0;

    // Configura SPI (1 MHz, 8 bit, MSB first, modalità 0)
    ((struct sx1262_config *)cfg)->spi_cfg = (struct spi_config){
        .frequency = 1000000,
        .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_0,
        .slave = DT_REG_ADDR(DT_DRV_INST(0)),
        .cs = NULL,
    };

    return 0;
}

// ------------------------
// Emulazione SPI (chiamata ogni transazione SPI)
// ------------------------
static int sx1262_emul_io(const struct emul *emul,
                         const struct spi_config *spi_cfg,
                         const struct spi_buf_set *tx_bufs,
                         const struct spi_buf_set *rx_bufs)
{
    struct sx1262_data *data = emul->data;

    // Gestisce trasmissione SPI
    if (tx_bufs && tx_bufs->count > 0) {
        const struct spi_buf *buf = &tx_bufs->buffers[0];

        // Copia i dati ricevuti
        memcpy(data->tx_buf, buf->buf, buf->len);
        data->tx_len = buf->len;

        LOG_INF("EMUL TX: %d bytes", buf->len);

        // Se il comando è TX, invia dati via UDP
        uint8_t *data = (uint8_t *)buf->buf;

        int udp_ret = udp_send_packet(data, buf->len);
        if (udp_ret == 0) {
            LOG_INF("EMUL UDP sent %d bytes", buf->len);
        } else {
            LOG_ERR("EMUL UDP send failed");
        }
    }

    // Gestisce ricezione SPI
    if (rx_bufs && rx_bufs->count > 0) {
        const struct spi_buf *buf = &rx_bufs->buffers[0];

        // Scrive dati fittizi (0x42) nel buffer di ricezione
        memset(buf->buf, 0x42, buf->len);
        data->rx_len = buf->len;

        LOG_INF("EMUL RX: %d bytes", buf->len);
    }

    return 0;
}

// API dell’emulatore SPI
static const struct spi_emul_api sx1262_emul_spi_api = {
    .io = sx1262_emul_io,
};

// ------------------------
// Inizializzazione emulatore
// ------------------------
static int sx1262_emul_init(const struct emul *emul, const struct device *parent)
{
    struct sx1262_data *data = emul->data;

    // Azzeramento dei dati
    data->tx_len = 0;
    data->rx_len = 0;
    memset(data->tx_buf, 0, sizeof(data->tx_buf));
    memset(data->rx_buf, 0, sizeof(data->rx_buf));
    return 0;
}

// ------------------------
// Macro per istanziare driver reale + emulatore
// ------------------------
#define SX1262_EMUL(n)                                                              \
    static struct sx1262_data sx1262_data_##n;                                      \
    static struct sx1262_config sx1262_cfg_##n = {                                  \
        .spi_bus = DEVICE_DT_GET(DT_INST_BUS(n)),                                   \
    };                                                                              \
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

// Applica la macro per ogni istanza nel devicetree con stato "okay"
DT_INST_FOREACH_STATUS_OKAY(SX1262_EMUL)

