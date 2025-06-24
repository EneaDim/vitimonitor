# 🍇 Sistema di Monitoraggio Sensoristico per il Settore Vitivinicolo

Sistema completo per la raccolta, gestione e visualizzazione dei dati ambientali da sensori in vigneti, basato su **FastAPI**, **MQTT/HTTP**, **SQLite** e con **frontend realizzato in Streamlit** per una fruizione semplice e interattiva dei dati.

---

## 🧱 Architettura del Sistema

**Flusso generale:**

- I sensori (mock o reali) inviano dati al backend tramite HTTP o MQTT
- Il backend salva i dati in un database SQLite
- Il frontend, sviluppato con Streamlit, visualizza i dati in tempo reale, con aggiornamento configurabile (polling attivabile/disattivabile)

**Componenti:**

- Sensori (fisici o simulati)
- Backend FastAPI
- Database SQLite
- Frontend interattivo Streamlit
- Makefile per automatizzare comandi e configurazioni

---

## 🔧 1. Sensori (Mock o Reali)

I sensori raccolgono:

- Temperatura
- Umidità
- Luminosità
- Coordinate GPS (lat, lon)
- Firma crittografica

**Modalità di invio:**

- HTTP POST verso `/data`
- MQTT sul topic configurabile (default: `sensor/data`)

**Mock disponibile:**

- `scripts/firmware_mock.py`  
Simula i dati dei sensori e li invia periodicamente (ogni 5 secondi)

---

## 🚀 2. Backend (FastAPI)

**File principale:** `scripts/backend.py`  
Il backend riceve, valida e memorizza i dati in SQLite.

### Endpoints REST:

| Endpoint  | Metodo | Descrizione                              |
|-----------|--------|----------------------------------------|
| `/`       | GET    | Verifica se il backend è attivo        |
| `/data`   | GET    | Ritorna tutti i dati raccolti           |
| `/data`   | POST   | Riceve dati dai sensori (via HTTP)     |
| `/status` | GET    | Stato del sistema + numero di record   |

### Configurazione:

- Tramite variabili d’ambiente `.env` o nel `Makefile`:
- `USE_MQTT=0|1`
- `DB_FILE=sensordata.db`
- `MQTT_BROKER`, `MQTT_PORT`, `MQTT_TOPIC`

---

## 📦 3. Database (SQLite)

- File locale: `sensordata.db`
- Usato dal backend per memorizzare tutti i dati raccolti
- Accessibile dal frontend (in lettura) se configurato

---

## 🖥️ 4. Frontend (Streamlit)

**File:** `scripts/frontend_streamlit.py`  
Interfaccia web realizzata con **Streamlit** per una visualizzazione semplice, moderna e interattiva dei dati raccolti.

Caratteristiche:

- Visualizzazione in tempo reale dei dati con aggiornamento automatico opzionale (polling)
- Grafici per temperatura, umidità, luminosità
- Mappa GPS per la geolocalizzazione dei dati
- Filtri personalizzabili: data/ora, tipo di dato, posizione
- Toggle per abilitare/disabilitare aggiornamento automatico
- Modalità di accesso ai dati tramite API REST o lettura diretta dal database

---

## 🧪 5. Testing

**File:** `scripts/test_backend.py`  
Test automatici con `pytest` per:

- Verifica funzionamento degli endpoint
- Validazione dei dati ricevuti
- Gestione degli errori (es. firma errata, dati incompleti)
- Controllo stato generale del sistema

---

## ⚙️ 6. Makefile

Automatizza avvio, test, pulizia e simulazione firmware.

### Comandi principali:

| Comando         | Descrizione                        |
|-----------------|----------------------------------|
| `make run`      | Avvia il backend FastAPI          |
| `make firmware` | Avvia il firmware mock (MQTT/HTTP) |
| `make test`     | Esegue i test automatici         |
| `make lint`     | Controllo stile con `flake8`     |
| `make curlgetdata` | Test manuale con `curl` su `/data` |
| `make cleandb`  | Elimina il database               |
| `make clean`    | Pulisce cache e file temporanei  |

---

## ⚙️ 7. Configurabilità (UI e Sistema)

| Funzionalità         | Dove si configura           | Opzioni disponibili           |
|----------------------|-----------------------------|------------------------------|
| Protocollo sensori   | `.env`, `Makefile`           | `USE_MQTT=0|1`                |
| Polling frontend     | Streamlit UI (toggle)        | ON / OFF                     |
| Fonte dati frontend  | Variabile `USE_API`          | API REST / Accesso diretto DB |
| Filtri visivi        | Interfaccia Streamlit        | Data, tipo dato, posizione    |
| Modalità test/mock   | `firmware_mock.py`           | POST o MQTT                  |

---

## 💡 Idee future

- Notifiche e alert automatici (es. valori critici)
- Integrazione con app mobile
- Analisi predittiva (modelli meteo, irrigazione)
- Multiutente e autenticazione (admin, agronomo, tecnico)
- Deploy su cloud o edge (Raspberry, VPS, ecc.)
- Esportazione CSV/PDF dei dati raccolti

---

## ✅ Conclusione

Questo sistema fornisce una base modulare, personalizzabile e completa per digitalizzare e monitorare il settore vitivinicolo, con un **frontend Streamlit già sviluppato** che garantisce usabilità, modularità e possibilità di estensione futura su più piattaforme.

