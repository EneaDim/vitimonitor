# 📐 Vitimonitor – Specifica Tecnica Avanzata del Sistema per la Viticoltura di Precisione

Questo documento rappresenta la specifica tecnica estesa del sistema **Vitimonitor**, una piattaforma distribuita IoT pensata per il monitoraggio ambientale e la gestione agronomica nei vigneti.

Contiene tutte le informazioni necessarie per l’implementazione hardware, firmware, architettura di backend, interfacce API, frontend e flussi dati.

---

## 🔁 1. Architettura Generale del Sistema

  [ NODI SENSORI ] ---> [ LoRaWAN ] ---> [ GATEWAY ] ---> [ BACKEND ] ---> [ DASHBOARD / APP ]
       ESP32                SX1276         RAK/Custom        API + DB         React / Flutter

- Sistema distribuito di nodi IoT ambientali a basso consumo
- Comunicazione LPWAN con topologia a stella
- Backend cloud-based per raccolta, storage, modellazione e visualizzazione
- Interfacce utente multi-piattaforma: PC e mobile

---

## 🧱 2. Nodo Sensore – Dettagli Elettronici

### 2.1 MCU: ESP32-C3

| Caratteristica     | Specifica                           |
|--------------------|--------------------------------------|
| Architettura       | RISC-V 32-bit, 160 MHz               |
| Memoria            | 400 KB SRAM, 4 MB Flash              |
| Connettività       | Wi-Fi 2.4 GHz, BLE 5.0, SPI, I2C     |
| Sleep Mode         | Deep Sleep: <10 µA                   |
| Programmazione     | ESP-IDF v5.x / Arduino Core          |

> ✅ Scelto per basso consumo, compatibilità open, stack LoRa + Wi-Fi, aggiornabilità OTA

---

### 2.2 Modulo LoRa

| Componente     | Specifica                              |
|----------------|-----------------------------------------|
| Chip           | Semtech SX1276 / Hope RFM95W           |
| Frequenza      | 868 MHz (EU868, Classe A)              |
| Antenna        | SMA esterna, 2.1 dBi omnidirezionale   |
| Configurazione | SpreadFactor: 7–12, BW: 125 kHz         |
| Potenza TX     | Fino a +20 dBm (regolabile)            |
| Interfaccia    | SPI + IRQ + RST                        |

---

### 2.3 Sensori Ambientali

| Parametro                  | Sensore        | Interfaccia | Note Tecniche                                        |
|----------------------------|----------------|-------------|------------------------------------------------------|
| Temperatura e umidità aria | SHT31-DIS      | I2C         | ±2% RH, ±0.3 °C, compensazione temperatura interna   |
| Umidità suolo (3 quote)    | Sensore capacitivo | ADC     | Calibrato, lettura multi-profondità (10–30–60 cm)   |
| Luminosità                 | BH1750 / TSL2561 | I2C        | Misura Lux, visibile + IR, integrabile con PAR      |
| Leaf Wetness (opzionale)  | Decagon LWS     | Analogico   | Superficie resistiva con curva di calibrazione       |
| Temperatura grappolo (opt.)| NTC 10k         | ADC         | Cavo schermato, lettura puntuale                     |

---

## 📦 3. Enclosure & PCB

- PCB 4-layer, FR4, trattamento ENIG
- Box IP65 con passacavo e supporto palo
- Connettori Molex impermeabili
- Interfaccia di programmazione esterna (UART/USB-C)
- Sensori disaccoppiati per evitare errori di lettura (e.g. radiazione solare)

---

## 📶 4. Gateway LoRaWAN

| Componente       | Specifica                                   |
|------------------|---------------------------------------------|
| Gateway          | RAK7258 / The Things Indoor Gateway         |
| Alternativa DIY  | ESP32 + LoRa concentratore SX1302 + LTE     |
| Backend compatibile | The Things Stack (v3) / ChirpStack       |
| Connettività     | Wi-Fi / Ethernet / LTE                      |
| Sicurezza        | Forwarding criptato + chiavi OTAA/ABP       |

---

## ☁️ 5. Backend

### Tecnologie core

- **Linguaggio**: Python (FastAPI) + Node.js
- **Database**: PostgreSQL per analisi storiche
- **API**: RESTful per raccolta e dashboard, GraphQL per query dinamiche
- **MQTT broker**: Mosquitto o EMQX per raccolta real-time
- **Elaborazione**: Job async per modelli agronomici

### Funzionalità agronomiche

- GDD (Growing Degree Days)
- ET₀ (evapotraspirazione potenziale)
- Previsione rischio fitopatogeni
- Aggregazione per microzona

---

## 💻 6. Frontend Web

| Stack           | Dettagli                                  |
|-----------------|--------------------------------------------|
| Framework       | React / Next.js + TailwindCSS              |
| Sicurezza       | Login JWT + ruoli (Admin, Agronomo, Tecnico)|
| Mappe           | Leaflet + mappe shapefile o GeoJSON        |
| Grafici         | Chart.js / ApexCharts, supporto realtime   |
| Export          | CSV, JSON, Excel                           |

---

## 📱 7. App Mobile (Flutter)

| Funzionalità             | Dettagli                                           |
|--------------------------|----------------------------------------------------|
| Visualizzazione sensori  | Elenco e posizione GPS nodi                        |
| Allarmi                  | Push via Firebase                                  |
| Note da campo            | Testo, foto, classificazione                       |
| Modalità offline         | Cache SQLite + sync al rientro in rete             |
| Autenticazione           | OAuth2 / Firebase                                  |

---

## 🔐 8. Sicurezza

- Crittografia LoRaWAN AES-128 (livello MAC)
- HTTPS su tutti gli endpoint web e API
- Ruoli accesso: separazione per microzona
- Audit trail per modifiche e interazioni utenti
- Protezione OTA (firmware signing)

---

## 📊 9. Dashboard & Modelli

- Widget personalizzabili
- Analisi temporalizzate per giorno/settimana/stagione
- Modelli previsioni:
  - Peronospora
  - Oidio
  - Botrite
  - Fase fenologica stimata
- Alert dinamici in base a:
  - Soglie
  - Cambi meteo
  - Fase vegetativa

---

## 🚀 10. MVP Target

| Componente   | Requisito                                    |
|--------------|-----------------------------------------------|
| Nodi         | ≥3 con sensori aria, suolo, luce             |
| Gateway      | 1 (TTN Indoor o custom)                      |
| Backend      | API + DB + modello GDD e ET₀                 |
| Web App      | Dashboard con grafici + mappe + allarmi      |
| Mobile App   | Note + visualizzazione + notifiche           |

---

## 🧭 11. Roadmap

1. Prototipo PCB nodo → test in campo
2. Gateway operativo con TTN
3. Backend MQTT + API + modelli iniziali
4. Dashboard React (v0.1) con mappe e grafici
5. App mobile base (Flutter) con alert e note
6. Test stagionale su 2–3 vigneti
7. Sviluppo modelli predittivi avanzati
8. Integrazione con droni/satelliti (NDVI, NDRE)
9. Supporto AI e tracciabilità

---

## 🧬 12. Stack Tecnologico

| Layer         | Tecnologia                                      |
|---------------|-------------------------------------------------|
| MCU           | ESP32-C3                                        |
| Radio         | LoRa 868 MHz                                    |
| Gateway       | RAK / Custom ESP32 concentratore                |
| Backend       | FastAPI + MQTT + PostgreSQL                     |
| Dashboard     | React / Next.js                                 |
| Mobile        | Flutter                                         |
| Cloud         | Docker, Nginx, GitHub Actions, Firebase         |
| Sicurezza     | LoRa AES128, HTTPS, RBAC                        |

---

## 🧪 13. Requisiti per Collaboratori Tecnici

- Esperienza con ESP32/LoRa (ESP-IDF, low power design)
- Familiarità con protocolli IoT (MQTT, HTTP, OTA)
- Conoscenza database time-series
- Esperienza in UI (React/Flutter) e API (REST/GraphQL)
- Comprensione base agronomia (GDD, ET₀, patologie vite)

---

> 📎 File associati: `schema_pcb.pdf`, `firmware.md`, `backend_architecture.md`, `mqtt_protocol.md`, `agronomic_models.md`

