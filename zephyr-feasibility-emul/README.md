# 🌿 Progetto IoT Agricolo con LoRa e Zephyr RTOS

## 📝 Descrizione

Questo progetto dimostrativo raccoglie **dati ambientali simulati** (temperatura e umidità) tramite un **sensore SHT3x-D emulato** e li elabora utilizzando **Zephyr RTOS**. È pensato per testare l’integrazione software in ambienti embedded, con un occhio allo sviluppo modulare e alla portabilità tra **simulazione su PC** e **dispositivi reali** come **RAK3172** (STM32WLE5) o **ESP32-S3**.

Il sistema è progettato per essere compatibile con **architetture multi-threaded**, con **blinking LED** e **log seriali** che mostrano i dati in tempo reale.

---

## 🧪 Sensore Emulato

- **SHT3x-D (Sensirion)**: emulazione completa della comunicazione I²C, supporta i comandi standard come misura, soft reset e lettura dello stato.
- Dati simulati: temperatura e umidità variabili.
- Supporto alla CRC secondo specifica Sensirion.
- Integrabile nel sistema tramite Devicetree.

---

## ⚙️ Funzionalità

- Acquisizione periodica dei dati da sensore emulato.
- Log dei dati su seriale in formato leggibile (Celsius e % RH).
- Blinking di un LED virtuale per simulare attività visibile.
- Architettura a thread separati: uno per il LED, uno per il sensore.
- Compatibilità completa con Zephyr `native_sim` per sviluppo e test su PC.

---

## 📦 Compatibilità Hardware

| Piattaforma | Stato | Note |
|-------------|-------|------|
| `native_sim` | ✅ Funziona | Test e sviluppo su PC |
| `ESP32-S3` | 🧪 In fase di porting | Necessario impostare `ESP_IDF_PATH` |

---

## 🔧 Personalizzazione

- **Intervallo di Lettura Sensore**
Il tempo tra le misure è configurabile nel codice sorgente tramite una variabile definita (`sensor_read_interval`).

- **Emulazione LED**
Il LED è configurato come `gpio-emul` e lampeggia per segnalare che il sistema è attivo. Compatibile con `led0` via alias Devicetree.

- **Valori Simulati**
I valori di temperatura e umidità possono essere generati casualmente o impostati manualmente con la funzione `sht3xd_emul_api_set()`.

---

## 🗂 Struttura del Progetto
zephyr-feasibility-emul/
├── src/ # Codice principale (main.c)
├── modules/sensirion_sht3xd_emul/ # Emulatore custom SHT3x
├── boards/ # Overlay Devicetree (esp32s3, native_sim)
├── prj.conf # Opzioni di configurazione Zephyr
├── CMakeLists.txt # File di build principale
└── README.md # Descrizione del progetto


---

## 🧪 Output Atteso

Durante l’esecuzione su `native_sim`, il sistema produce un log simile al seguente:
[00:00:01.010,000] <inf> main: Emulated Temperature: 24.16 C, Humidity: 53.45 %
[00:00:01.020,000] <inf> main: Emulated LED Blink: On


---

## ⚖️ Licenza

Distribuito sotto licenza **MIT**.
Consulta il file `LICENSE` per i dettagli.

---

## 🤝 Contributi

Il progetto è in continua evoluzione.
È possibile contribuire con:
- Altri emulatori I²C/SPI/ADC
- Backend per trasmissione LoRaWAN o MQTT
- Interfacce per configurazione remota o logging avanzato

Apri una issue o una pull request per collaborare!


