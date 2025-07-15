# Progetto IoT LoRa con Zephyr RTOS

## Descrizione

Questo progetto raccoglie i dati da 4 sensori (temperatura, umidità aria, umidità suolo, luminosità), li comprime e li invia tramite LoRa utilizzando **Zephyr RTOS**. Il progetto è progettato per un dispositivo basato su **RAK3172** (LoRa STM32WLE5) e supporta la gestione del consumo energetico e la comunicazione LoRaWAN.

## Sensori Utilizzati

- **DHT22**: Sensore di temperatura e umidità aria.
- **Capacitive Soil Moisture Sensor**: Sensore di umidità del suolo.
- **BH1750**: Sensore di luminosità.

## Funzionalità

- Raccolta dei dati dai sensori.
- Compressione opzionale dei dati per risparmiare larghezza di banda.
- Invio dei dati tramite LoRa.
- Gestione del risparmio energetico con modalità di sleep.
- Intervallo configurabile per la lettura dei sensori.

## Configurazione del Progetto

### Requisiti

- **Zephyr RTOS** v2.6 o superiore.
- **CMake** e **Ninja** per la build.
- Hardware: **RAK3172** (o simili) per la comunicazione LoRa.

---

### Come Compilare e Caricare il Firmware

1. Clona il repository:
   ```bash
   git clone https://github.com/tuo-repo/loara-sensors-zephyr.git
   cd loara-sensors-zephyr
   ```
2. Configura il progetto per la tua board (ad esempio, se usi una board basata su RAK3172):
   ```bash
    west build -b rak3172_board
   ```
3. Carica il firmware sulla tua board:
   ```bash
   west flash
   ```
4. Monitoraggio della seriale per i log:
   ```bash
   west monitor
   ```

---

### Personalizzazione

- Compressione Dati: La compressione dei dati può essere abilitata o disabilitata modificando la variabile enable_compression nel codice.

- Intervallo di Lettura: L'intervallo di lettura dei sensori può essere modificato tramite la variabile sensor_read_interval.

---

## Licenza

Distribuito sotto la Licenza MIT. Vedi LICENSE per ulteriori informazioni.

---

