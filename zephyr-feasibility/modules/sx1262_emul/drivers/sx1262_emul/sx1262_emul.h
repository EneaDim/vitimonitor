// Protezione contro inclusioni multiple del file header
#ifndef SX1262_EMUL_H_
#define SX1262_EMUL_H_

// Include delle API Zephyr per device, SPI e emulatori
#include <zephyr/device.h>        // Gestione generica dei device
#include <zephyr/drivers/spi.h>   // API per comunicazione SPI
#include <zephyr/drivers/emul.h>  // API per creare emulatori di periferiche

// Se stiamo usando C++, evita problemi con il name mangling
#ifdef __cplusplus
extern "C" {
#endif

// ------------------------
// Struttura di configurazione della periferica
// ------------------------
struct sx1262_config {
    const struct device *spi_bus;      // Riferimento al bus SPI
    struct spi_config spi_cfg;         // Configurazione SPI (frequenza, modalità, ecc.)
};

// ------------------------
// Struttura dati di runtime della periferica/emulatore
// ------------------------
struct sx1262_data {
    // Buffer per dati trasmessi e ricevuti (emulazione)
    uint8_t tx_buf[256];               // Dati trasmessi dall'host alla periferica
    uint8_t rx_buf[256];               // Dati ricevuti dalla periferica all'host
    size_t tx_len;                     // Numero di byte trasmessi
    size_t rx_len;                     // Numero di byte ricevuti
};

// ------------------------
// API esposte dal driver/emulatore SX1262
// ------------------------
struct sx1262_emul_driver_api {
    int (*send)(const struct device *dev, const uint8_t *data, size_t len);     // API di invio SPI
    int (*recv)(const struct device *dev, uint8_t *data, size_t max_len);       // API di ricezione SPI
};

// ------------------------
// Funzioni pubbliche del driver
// ------------------------
int sx1262_send(const struct device *dev, const uint8_t *data, size_t len);     // Invio dati su SPI
int sx1262_recv(const struct device *dev, uint8_t *data, size_t max_len);       // Ricezione dati da SPI

// Fine del blocco extern "C" per C++
#ifdef __cplusplus
}
#endif

// Fine delle protezioni contro inclusioni multiple
#endif // SX1262_EMUL_H_

