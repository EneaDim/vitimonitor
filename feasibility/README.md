# 🍇 VitiMonitor - Monitoraggio e Analisi dei Dati dei Vigneti

**VitiMonitor** è un sistema integrato per il monitoraggio in tempo reale e l'analisi dei dati provenienti dai sensori nei vigneti. Il sistema raccoglie dati su temperatura, umidità dell'aria, umidità del suolo, luminosità e posizione GPS, e li elabora per fornire analisi avanzate sullo stato del vigneto, rilevamento di anomalie e pianificazione delle attività.

Questo sistema include:
- Un **backend FastAPI** per gestire le richieste API e salvare i dati nel database.
- Un **mock firmware** per simulare i sensori.
- Un **frontend Streamlit** che fornisce una visualizzazione interattiva dei dati.

## 🚀 Struttura del Progetto

Il progetto include i seguenti file principali:

- **`backend.py`**: Il backend FastAPI per gestire le richieste API, salvare i dati nel database e connettersi al broker MQTT per ricevere i dati dai sensori.
- **`sensor_mock.py`**: Il mock di sensori che genera dati simulati per temperatura, umidità, luminosità e posizione GPS.
- **`firmware_mock.py`**: Il firmware mock che simula l'acquisizione dei dati dai sensori e li invia al backend.
- **`frontend.py`**: Il frontend Streamlit che visualizza i dati in tempo reale e fornisce strumenti di analisi per manager, enologi e operatori.
- **`frontend_manager.py, frontend_enologo.py, frontend_operatore.py, frontend_log.py`**: Moduli frontend per visualizzare dati in base al ruolo dell'utente (Manager, Enologo, Operatore, Log).
  
## 📦 Prerequisiti

Assicurati di avere i seguenti strumenti installati:

- **Python 3.x**
- **pip** per installare i pacchetti Python
- **MQTT broker** (se usato)
- **Make** per eseguire i comandi nel Makefile

## ⚙️ Setup e Avvio del Progetto

### 1. Creazione dell'ambiente virtuale e installazione delle dipendenze

Per creare l'ambiente virtuale e installare tutte le dipendenze necessarie, segui le istruzioni nel Makefile per inizializzare l'ambiente e configurare il progetto.

Questo comando: `make init`
- Crea un ambiente virtuale.
- Installa le dipendenze da `requirements.txt`.
- Imposta le cartelle necessarie.

Ora attiva l'ambiente virtuale con : `source .venv/bin/activate`


### 2. Avvio dei Servizi

Una volta configurato l'ambiente, puoi avviare i vari servizi del sistema separatamente o tutti insieme.

- **Avviare il backend**: Avvia il servizio backend basato su FastAPI.
    - `make backend`
- **Avviare il firmware mock**: Simula il comportamento del firmware per raccogliere dati dai sensori.
    - `make firmware`
- **Avviare il frontend**: Avvia il servizio frontend utilizzando Streamlit.
    - `make frontend`
- **Avviare tutti i servizi contemporaneamente**: Avvia backend, firmware e frontend in una sessione tmux.
    - `make run-all`

### 3. Interagire con il Sistema

Il sistema è composto da tre principali interfacce utente:

- **👨‍💼 Manager**: Permette di visualizzare i KPI economici, la distribuzione delle misurazioni e il rischio nelle zone.
- **🍷 Enologo**: Consente di visualizzare l'indice di qualità per zona e le distribuzioni di variabili come la temperatura e l'umidità.
- **👷 Operatore**: Permette di gestire le anomalie rilevate, pianificare attività correttive e generare report.

Ogni interfaccia offre funzionalità specifiche:
- **Manager**: Analisi economiche, distribuzione dei parametri nel vigneto e valutazione del rischio.
- **Enologo**: Indice di qualità per zona, calcolo della salute delle piante in base ai parametri misurati.
- **Operatore**: Gestione delle anomalie, pianificazione delle attività e visualizzazione delle misurazioni manuali.

### 4. Test e Linting

Per eseguire i test automatici e il linting del codice, utilizza gli strumenti descritti nel Makefile per verificare il corretto funzionamento del sistema e la qualità del codice.

### 5. Pulizia del Progetto

Per rimuovere i file temporanei, le cache e altre risorse non necessarie, segui le istruzioni nel Makefile per pulire l'ambiente e ripristinare lo stato del progetto.

## 📝 Configurazione

Le configurazioni del progetto sono gestite tramite il file `config.yml`. Puoi visualizzare la configurazione corrente e modificarla direttamente nel file di configurazione. Il Makefile ti permette anche di aggiornare facilmente un valore nella configurazione, come il tempo di invio dei dati o altre impostazioni.

## 📈 Analisi e Report

Ogni utente può generare report Excel o PDF personalizzati basati sui dati delle anomalie o delle attività pianificate. Usa i bottoni nel frontend per scaricare il report richiesto.

---

**VitiMonitor** fornisce un sistema completo per il monitoraggio e la gestione dei dati dei vigneti. Grazie all'interfaccia grafica, ai servizi backend e alle simulazioni dei sensori, offre una soluzione potente per gestire in tempo reale la salute e l'economia del vigneto.

---

## 🗄️ Setup del Database PostgreSQL

Per configurare PostgreSQL come database per VitiMonitor, segui questi passi:

1. **Installare PostgreSQL**: `sudo apt-get install libpq-dev postgresql postgresql-contrib`

2. **Start PostgreSQL**: `sudo systemctl start postgresql`

3. **Setup** : `make setup-db`

If you want to run the postgress db in the terminal: `sudo -u postgres psql`

---
