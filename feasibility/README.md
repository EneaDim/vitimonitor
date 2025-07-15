# VitiMonitor - Sistema di Monitoraggio dei Vigneti

VitiMonitor è una piattaforma per il monitoraggio in tempo reale dei vigneti, che raccoglie dati dai sensori ambientali distribuiti nelle zone di un vigneto. La piattaforma fornisce un dashboard interattivo, gestisce anomalie nei dati e permette di generare report dettagliati. Il sistema è composto da un backend in **FastAPI**, un frontend in **Streamlit**, e un firmware mock che simula i sensori.

## Struttura del Sistema

Il sistema è suddiviso in vari componenti, ognuno dei quali si occupa di una parte specifica del monitoraggio e dell'elaborazione dei dati:

- **Backend (FastAPI)**: gestisce la ricezione dei dati dai sensori e li salva nel database SQLite.
- **Frontend (Streamlit)**: visualizza i dati raccolti dai sensori in tempo reale e fornisce strumenti per l'analisi.
- **Firmware Mock**: simula il comportamento dei sensori, generando dati e inviandoli al backend.
- **Sensor Mock**: simula singoli sensori, generando letture di temperatura, umidità, luminosità e GPS.

## Requisiti

- Python >= 3.8
- FastAPI
- Uvicorn (per il backend)
- Streamlit (per il frontend)
- SQLite (per il database)
- MQTT (opzionale, per l'invio dei dati tramite MQTT)
- librerie come `requests`, `paho.mqtt`, `colorama`, `plotly`, `fpdf`, `pandas` e altre incluse nel file `requirements.txt`

## Installazione e Configurazione

### 1. Creazione dell'ambiente virtuale

Esegui il comando seguente per creare e attivare l'ambiente virtuale:
```bash
make init
```

Questo comando:
- Crea un ambiente virtuale `.venv`
- Installa tutte le dipendenze elencate nel file `requirements.txt`
- Crea le cartelle necessarie e il file di configurazione `config.yml`

### 2. Attivazione dell'ambiente virtuale

Per attivare l'ambiente virtuale, usa il comando:

```bash
make venv
```

### 3. Aggiornamento dei pacchetti

Se vuoi aggiornare i pacchetti installati e generare un nuovo `requirements.txt`, usa:

```bash
make freeze
```

### 4. Configurazione del sistema

Il sistema è configurabile tramite il file `config.yml`. Puoi visualizzare la configurazione corrente con:

```bash
make show-config 
```

Per aggiornare un valore nel file di configurazione, usa il comando:

```bash
make update-config KEY=<nome_chiave> VALUE=<valore>
```
Ad esempio, per aggiornare l'intervallo di invio dei dati a 10 secondi:
```bash
make update-config KEY=send_interval VALUE=10
```

### 5. Avvio dei servizi

Per avviare il backend, il firmware e il frontend separatamente, puoi utilizzare i seguenti comandi:

- **Avvia il backend (FastAPI)**:
    ```
    make backend
    ```

- **Avvia il firmware mock**:
    ```
    make firmware
    ```

- **Avvia il frontend (Streamlit)**:
    ```
    make frontend
    ```

Per avviare tutti i servizi insieme in una sessione tmux, puoi utilizzare il comando:

```bash
make run-all
```

Questo comando avvierà il backend, il firmware e il frontend in una sessione tmux separata.

### 6. Test

Per eseguire i test automatici del sistema, utilizza:

```bash
make test
```

### 7. Pulizia

Per rimuovere file temporanei e cache, utilizza:


```bash
make clean
```

Per eliminare il file del database SQLite, usa:


```bash
make cleandb
```

## Funzionalità del Sistema

### Backend

- **API di Ricezione Dati**: il backend riceve i dati dei sensori tramite una richiesta `POST` alla rotta `/data`. I dati vengono validati e memorizzati in un database SQLite.
- **API di Visualizzazione Dati**: è possibile ottenere tutti i dati raccolti tramite una richiesta `GET` alla rotta `/data`.
- **Gestione Anomalie**: il sistema può rilevare anomalie nei dati dei sensori, come temperature troppo alte o basse, e generare notifiche o segnalarle all'utente.

### Frontend

Il frontend utilizza Streamlit per creare una dashboard che visualizza i dati dei sensori in tempo reale, mostrando metriche come la temperatura, l'umidità dell'aria e del suolo, e la luminosità. Offre anche grafici e strumenti di analisi per identificare tendenze e anomalie nei dati.

### Firmware Mock

Il firmware mock simula il comportamento dei sensori, generando periodicamente dati e inviandoli al backend tramite richieste HTTP o MQTT, in base alla configurazione.

### Report e Pianificazione

Il sistema consente di generare report in formato PDF ed Excel contenenti informazioni su anomalie, attività pianificate e dati storici. È anche possibile pianificare attività correttive per gestire le anomalie rilevate.

## Configurazione Avanzata

Il file di configurazione `config.yml` contiene vari parametri che definiscono il comportamento del sistema, come:
- L'URL del backend
- La configurazione dei sensori e delle zone
- Le soglie per rilevare anomalie

Esempio di configurazione:

```yaml
backend_url: "http://localhost:8000/data"
use_mqtt: true
mqtt_broker: "localhost"
mqtt_port: 1883
mqtt_topic: "sensor/data"
sensors_per_zone: 2
send_interval: 5
zones:
  - "zone_north"
  - "zone_south"
soglie:
  temperature: [10, 35]
  humidity_air: [30, 70]
  humidity_soil: [20, 50]
```

## Conclusioni

VitiMonitor offre una soluzione potente per il monitoraggio e la gestione dei vigneti, combinando tecnologie moderne come FastAPI, Streamlit e MQTT per raccogliere, visualizzare e analizzare i dati dei sensori in tempo reale. Può essere facilmente esteso e personalizzato per soddisfare le esigenze specifiche di qualsiasi azienda agricola.
