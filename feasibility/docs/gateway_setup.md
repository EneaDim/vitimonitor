# Configurazione del Gateway per il Backend FastAPI

Questo documento descrive i passaggi per configurare un gateway (come un Raspberry Pi o simili) per eseguire il backend FastAPI, con PostgreSQL per il database e Redis per la gestione della cache.

## 1. Preparazione del Gateway

### 1.1 Verifica che il gateway supporti Python

Il gateway deve essere in grado di eseguire Python. Assicurati che Python sia installato nel dispositivo.

Puoi verificare se Python è installato con:

python3 --version

Se non è installato, installa Python con:

sudo apt update
sudo apt install python3 python3-pip

### 1.2 Accedi al Gateway

Accedi al tuo gateway tramite SSH (se è un dispositivo remoto) o tramite terminale fisico (se hai accesso diretto).

ssh pi@<IP_DEL_GATEWAY>

## 2. Creazione dell'Ambiente Virtuale Python

Per isolare le dipendenze, è consigliato creare un ambiente virtuale Python.

### 2.1 Crea l'ambiente virtuale

python3 -m venv backend_env

### 2.2 Attiva l'ambiente virtuale

source backend_env/bin/activate

### 2.3 Installa le dipendenze

Installa FastAPI, Uvicorn, PostgreSQL, Redis e altre librerie necessarie.

pip install fastapi uvicorn psycopg2 asyncpg redis

## 3. Configurazione del Database PostgreSQL

### 3.1 Installa PostgreSQL

Se il tuo gateway non ha PostgreSQL, puoi installarlo con:

sudo apt install postgresql postgresql-contrib

### 3.2 Avvia PostgreSQL

Verifica che PostgreSQL sia in esecuzione:

sudo systemctl start postgresql

Puoi anche verificare lo stato del servizio:

sudo systemctl status postgresql

### 3.3 Configura il Database

Accedi a PostgreSQL come superutente (`postgres`) per configurare il database:

sudo -u postgres psql

Per creare un nuovo utente e concedergli i permessi, esegui:

CREATE USER user WITH PASSWORD 'password';
GRANT ALL PRIVILEGES ON DATABASE sensordata TO user;

Esci dalla shell di PostgreSQL:

\q

## 4. Configurazione di Redis

### 4.1 Installa Redis

Se Redis non è installato sul gateway, puoi installarlo con:

sudo apt install redis-server

### 4.2 Avvia Redis

Avvia il servizio Redis:

sudo systemctl start redis

Verifica che Redis sia in esecuzione:

sudo systemctl status redis

### 4.3 Configura Redis

Puoi configurare Redis per il caching dei dati, modificando il file di configurazione di Redis se necessario:

sudo nano /etc/redis/redis.conf

Modifica le impostazioni come richiesto, quindi riavvia Redis:

sudo systemctl restart redis

## 5. Esecuzione del Backend FastAPI

### 5.1 Avvia il server FastAPI

Una volta che tutto è configurato, puoi avviare il server FastAPI:

uvicorn backend:app --host 0.0.0.0 --port 8000

### 5.2 Esegui il server come un servizio

Per far partire automaticamente il backend all'avvio del sistema, configura un servizio di sistema con `systemd`. Crea un file di servizio come segue:

sudo nano /etc/systemd/system/backend.service

Aggiungi il seguente contenuto:

[Unit]
Description=FastAPI backend service
After=network.target

[Service]
User=pi
WorkingDirectory=/home/pi/backend
ExecStart=/home/pi/backend/backend_env/bin/uvicorn backend:app --host 0.0.0.0 --port 8000
Restart=always

[Install]
WantedBy=multi-user.target

Ricarica i servizi di sistema e avvia il servizio:

sudo systemctl daemon-reload
sudo systemctl enable backend.service
sudo systemctl start backend.service

## 6. Monitoraggio e Gestione

Una volta che il backend è in esecuzione, puoi monitorarlo con `systemctl` per visualizzare i log o fermarlo:

Visualizzare i log:

sudo journalctl -u backend.service -f

Fermare il servizio:

sudo systemctl stop backend.service

Riavviare il servizio:

sudo systemctl restart backend.service


