# 🚀 Sistema di Monitoraggio e Analisi per il Settore Vitivinicolo

**VitiMonitor** è un sistema integrato per il monitoraggio, l'analisi e la gestione dei parametri vitivinicoli, composto da:

✅ **Backend** in [FastAPI] — API RESTful e persistenza dati in SQLite  
✅ **Firmware mock** — simulazione dati sensori da file `config.yml`  
✅ **Frontend** in [Streamlit] — dashboard interattiva per visualizzazione e analisi

---

## 📦 Requisiti

- Python ≥ 3.9
- [pip](https://pip.pypa.io/)
- [tmux](https://github.com/tmux/tmux) (per `make run-all`)

---

## 🧰 Setup

### 1️⃣ Clona il repository
Clona il repository utilizzando il comando:

git clone <repo-url>  
cd <repo>

### 2️⃣ Inizializza l’ambiente
Esegui il comando:

make init

Questo comando:
- crea un virtualenv in `.venv`
- installa i pacchetti da `requirements.txt`
- crea le cartelle `data/` e `scripts/` (con `__init__.py`)
- genera un file `config.yml` di default

### 3️⃣ Attiva l’ambiente virtuale
Attiva l'ambiente virtuale con il comando:

make venv  
source .venv/bin/activate

---

## 🚀 Avvio dei servizi

Puoi avviare i servizi singolarmente oppure tutti insieme.

### Singoli servizi:
- **Backend FastAPI**:  
  Esegui il comando:
  make backend  
  Le API saranno disponibili su: [http://localhost:8000](http://localhost:8000)

- **Firmware mock**:  
  Esegui il comando:
  make firmware

- **Frontend Streamlit**:  
  Esegui il comando:
  make frontend

### Tutto insieme (in tmux):
Esegui il comando:
make run-all

Questo aprirà una sessione tmux con 3 pannelli: backend, firmware e frontend.

---

## 🖥️ Frontend

Il frontend Streamlit fornisce:
- Grafici in tempo reale delle metriche raccolte (ad esempio, temperatura, umidità)
- Analisi avanzata delle fusion score per il monitoraggio della qualità delle uve
- Raccomandazioni operative basate su soglie di qualità
- Annotazioni e filtri interattivi

Accessibile su: [http://localhost:8501](http://localhost:8501)

---

## 🧪 Test & Debug

### Linting del codice
Per eseguire il linting del codice, esegui:
make lint

### Test automatici
Per eseguire i test automatici, esegui:
make test

### Verifica API con curl
Per testare le API, esegui uno dei seguenti comandi:
make curlroot      # GET /  
make curlgetdata   # GET /data  
make curlstatus    # GET /status

---

## 📝 Configurazione

Il file `config.yml` definisce i parametri operativi del firmware mock.

Per mostrare il contenuto corrente:
make show-config

Per aggiornare una chiave in `config.yml`:
make update-config KEY=send_interval VALUE=10

---

## 🧹 Pulizia

- **Pulire file temporanei**:
  make clean

- **Resettare il database**:
  make cleandb

- **Aggiornare `requirements.txt`**:
  make freeze

---

## 📚 API Endpoints principali

| Metodo | Endpoint      | Descrizione                  |
|--------|---------------|-------------------------------|
| GET    | `/`           | Root                         |
| GET    | `/data`       | Dati sensori correnti        |
| GET    | `/status`     | Stato del sistema            |

---

## 📄 Struttura delle cartelle
.
├── data/              # File di dati
├── scripts/           # Codice backend, frontend e firmware
│ ├── backend.py       # Codice backend (FastAPI)
│ ├── frontend.py      # Codice frontend (Streamlit)
│ └── firmware_mock.py # Simulazione del firmware
│ └── sensor_mock.py   # Simulazione del sensore
├── config.yml         # File di configurazione
└── requirements.txt   # Dipendenze Python

