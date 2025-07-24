# 📐 Requisiti Tecnici del Sistema Vitimonitor

Documento di specifica tecnica per la progettazione hardware, backend e frontend del sistema integrato per il monitoraggio viticolo.

---

## 🧱 1. Hardware

### 1.1 Nodo Sensore

| Componente         | Descrizione                              |
|--------------------|------------------------------------------|
| Microcontrollore   | ESP32 (dual core, Wi-Fi, BT, basso consumo) |
| Comunicazione      | LoRa SX1276 o RFM95W (868 MHz EU)        |
| Alimentazione      | Batteria Li-Ion 3.7V + pannello solare   |
| PCB                | Custom, IP65, dimensioni compatte        |

### 1.2 Sensori Supportati

| Sensore                        | Tipo/Modello suggerito              |
|-------------------------------|-------------------------------------|
| Temperatura/UR aria           | DHT22 / SHT31 / BME280              |
| Umidità suolo (3 profondità)  | Capacitivo o resistivo calibrato    |
| Luminosità                    | BH1750 / TSL2561 / Sensore PAR      |
| Leaf Wetness (opzionale)      | Decagon LWS                         |
| Temperatura grappolo (opt.)   | Termocoppia tipo K o NTC su cavo    |

---

## 📶 2. Gateway LoRaWAN

| Opzione               | Specifiche                            |
|-----------------------|----------------------------------------|
| Acquisto gateway      | The Things Indoor Gateway / RAK7258    |
| Gateway custom        | ESP32 + LoRa concentratore + LTE/Wi-Fi |
| Backend connesso      | The Things Stack (TTN) o ChirpStack    |

---

## ☁️ 3. Backend

| Componente        | Tecnologia suggerita                       |
|-------------------|--------------------------------------------|
| Database          | InfluxDB (time-series) o PostgreSQL        |
| API               | RESTful API o GraphQL                      |
| Agronomic Engine  | Calcolo GDD, ET0, modelli rischio          |
| Notifiche         | Firebase Cloud Messaging / Email alerts    |

---

## 💻 4. Frontend Web (PC)

| Utente Target     | Enologo, Manager, Agronomo                  |
|-------------------|---------------------------------------------|
| Tecnologie        | React / Next.js / TailwindCSS               |
| Funzionalità      | Mappe, Dashboard, Cronologia, Alert         |
| Sicurezza         | Login, livelli accesso, crittografia        |

---

## 📱 5. App Mobile (Campo)

| Utente Target     | Operatore tecnico in vigna                  |
|-------------------|---------------------------------------------|
| Tecnologie        | Flutter / React Native                      |
| Funzionalità      | Sensori vicini, alert, inserimento note     |
| Offline mode      | Sì, con sync dati non appena connesso       |

---

## 📊 6. Dashboard & Analytics

| Visualizzazione         | Grafici interattivi, mappe, filtri temporali |
| Modelli predittivi      | Fitopatie, fenologia, vendemmia               |
| Esportazione dati       | CSV, Excel, API                               |
| Alert intelligenti      | Soglie dinamiche e contesto (meteo, fase)     |

---

## 🔐 7. Sicurezza & Scalabilità

- Crittografia dei dati LoRaWAN
- HTTPS per tutti i servizi
- Ruoli e permessi differenziati
- Scalabilità cloud-native (Docker/Kubernetes optional)

---

## 🧪 8. MVP Target

| Modulo       | Obiettivo minimo                                |
|--------------|--------------------------------------------------|
| Hardware     | 3 nodi sensori + 1 gateway TTN                   |
| Backend      | Raccolta dati, calcolo GDD, ET0                  |
| Web App      | Dashboard con mappa e grafici base               |
| Mobile App   | Alert + visualizzazione locale + note            |

---

## 🗓️ Roadmap Prossimi Step

1. Disegno PCB nodo sensore e primo prototipo
2. Scelta gateway e test LoRaWAN con TTN
3. Backend mock + visualizzazioni minime
4. App mobile testabile (Flutter)
5. Test in campo e feedback con 2–3 aziende

