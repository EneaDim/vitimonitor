import os
import logging
import json
import psycopg2
from fastapi import FastAPI, HTTPException, Query
from pydantic import BaseModel
from typing import List
import threading
import redis
from databases import Database
import paho.mqtt.client as mqtt
from datetime import datetime

# ========== Parametri ==========
DB_URL = os.getenv("DB_URL")                         # Connessione al DB
DB_USER = os.getenv("DB_USER")                       # Nome utente cliente
DB_PASSWORD = os.getenv("DB_PASSWORD")               # Password utente cliente
DB_NAME = os.getenv("DB_NAME")                       # Nome del database
SUPERUSER_NAME = os.getenv("SUPERUSER_NAME")         # Superuser di PostgreSQL
SUPERUSER_PASSWORD = os.getenv("SUPERUSER_PASSWORD") # Password del superuser
USE_MQTT = os.getenv("USE_MQTT")                     # Se abilitare MQTT
MQTT_BROKER = os.getenv("MQTT_BROKER")               # MQTT Broker address
MQTT_PORT = int(os.getenv("MQTT_PORT"))              # MQTT Broker port
MQTT_TOPIC = os.getenv("MQTT_TOPIC")                 # MQTT topic

# ========== Connessione al database PostgreSQL ==========
database = Database(DB_URL)  # Connessione al database asincrono

# ========== Connessione a Redis per caching ==========
redis_client = redis.StrictRedis(host='localhost', port=6379, db=0)  # Connessione a Redis

# ========== Creazione dell'istanza FastAPI ==========
app = FastAPI()  # Istanza dell'app FastAPI

# ========== Creazione del modello per la validazione dei dati ==========
# Il modello SensorData rappresenta la struttura dei dati del sensore
class SensorData(BaseModel):
    sensor_id: str  # ID del sensore
    zone: str  # Zona del sensore
    temperature: float  # Temperatura rilevata
    humidity_air: float  # Umidità dell'aria
    humidity_soil: float  # Umidità del suolo
    luminosity: float  # Luminosità
    signature: str  # Firma del dato
    manual: bool = False  # Indica se la misura è manuale (default False)

# ========== Connessione al database ==========
# Funzione che viene eseguita all'avvio dell'app
@app.on_event("startup")
async def startup():
    # Configura il database e connette l'app
    await configure_database()

    # Connette al database e crea le tabelle necessarie
    await database.connect()
    await create_tables()

# Funzione che viene eseguita alla chiusura dell'app
@app.on_event("shutdown")
async def shutdown():
    await database.disconnect()  # Disconnette la connessione al database

# ========== Funzioni per creare tabelle e indici nel database ==========
# Funzione per creare le tabelle nel database
async def create_tables():
    query = '''
    CREATE TABLE IF NOT EXISTS sensor_data (
        id SERIAL PRIMARY KEY,
        sensor_id TEXT,
        zone TEXT,
        temperature REAL,
        humidity_air REAL,
        humidity_soil REAL,
        luminosity REAL,
        signature TEXT,
        manual BOOLEAN DEFAULT FALSE,
        timestamp TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
    );
    '''
    async with database.transaction():  # Usa una transazione asincrona per eseguire la query
        await database.execute(query)  # Esegui la query per creare la tabella
        await database.execute('CREATE INDEX IF NOT EXISTS idx_sensor_id ON sensor_data(sensor_id);')  # Indice per sensor_id
        await database.execute('CREATE INDEX IF NOT EXISTS idx_timestamp ON sensor_data(timestamp);')  # Indice per timestamp
        await database.execute('CREATE INDEX IF NOT EXISTS idx_zone ON sensor_data(zone);')  # Indice per zona

# ========== Funzione per configurare il database e l'utente ==========
# Funzione per configurare il database e l'utente se non esistono
async def configure_database():
    conn = psycopg2.connect(dbname="postgres", user=SUPERUSER_NAME, password=SUPERUSER_PASSWORD, host="localhost")
    conn.autocommit = True
    cur = conn.cursor()

    # Creazione del database se non esiste
    cur.execute(f"SELECT 1 FROM pg_catalog.pg_database WHERE datname = '{DB_NAME}'")
    if not cur.fetchone():
        print(f"Creando il database {DB_NAME}")
        cur.execute(f"CREATE DATABASE {DB_NAME}")

    # Creazione dell'utente se non esiste
    cur.execute(f"SELECT 1 FROM pg_roles WHERE rolname='{DB_USER}'")
    if not cur.fetchone():
        print(f"Creando l'utente {DB_USER}")
        cur.execute(f"CREATE USER {DB_USER} WITH PASSWORD '{DB_PASSWORD}'")

    # Concessione dei permessi
    cur.execute(f"GRANT CONNECT ON DATABASE {DB_NAME} TO {DB_USER};")
    cur.execute(f"GRANT USAGE, CREATE ON SCHEMA public TO {DB_USER};")
    cur.execute(f"GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO {DB_USER};")
    cur.execute(f"GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO {DB_USER};")

    # Permessi per operare su tabelle esistenti e future
    cur.execute(f"ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO {DB_USER};")
    cur.execute(f"ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO {DB_USER};")

    # Aggiungi privilegi per l'utente
    cur.execute(f"GRANT USAGE, CREATE ON SCHEMA public TO {DB_USER};")
    cur.execute(f"GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO {DB_USER};")
    cur.execute(f"ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT SELECT, INSERT, UPDATE, DELETE ON TABLES TO {DB_USER};")

    cur.close()
    conn.close()

    print(f"Configurazione completata con successo per il database {DB_NAME}, l'utente {DB_USER}, e i permessi")

# ========== Funzione per salvare i dati nel DB ==========
# Funzione che salva i dati ricevuti nel database
async def save_to_db(sensor_data: SensorData):
    query = '''
    INSERT INTO sensor_data (sensor_id, zone, temperature, humidity_air, humidity_soil, luminosity, signature, manual)
    VALUES (:sensor_id, :zone, :temperature, :humidity_air, :humidity_soil, :luminosity, :signature, :manual)
    '''
    # Esegui l'inserimento dei dati nel database
    await database.execute(query, {
        'sensor_id': sensor_data.sensor_id,
        'zone': sensor_data.zone,
        'temperature': sensor_data.temperature,
        'humidity_air': sensor_data.humidity_air,
        'humidity_soil': sensor_data.humidity_soil,
        'luminosity': sensor_data.luminosity,
        'signature': sensor_data.signature,
        'manual': sensor_data.manual
    })

# ========== Funzioni per monitorare MQTT ==========
# Callback per ricevere i messaggi MQTT
def on_message(client, userdata, msg):
    payload = json.loads(msg.payload.decode())  # Decodifica il payload del messaggio
    print(f"Messaggio ricevuto: {payload}")

# Funzione per avviare il listener MQTT
def start_mqtt_listener():
    client = mqtt.Client()
    client.on_message = on_message  # Imposta il callback per i messaggi
    client.connect(MQTT_BROKER, MQTT_PORT)  # Connetti al broker MQTT
    client.subscribe(MQTT_TOPIC)  # Sottoscrivi al topic
    client.loop_forever()  # Inizia a ricevere messaggi in loop

# Funzione che avvia il listener MQTT all'avvio dell'app
@app.on_event("startup")
async def start_mqtt():
    if USE_MQTT:
        thread = threading.Thread(target=start_mqtt_listener, daemon=True)  # Avvia il listener in un thread separato
        thread.start()
        print("MQTT listener avviato")

# ========== Funzione per validare la firma ==========
# Funzione per validare la firma dei dati ricevuti
def verify_signature(data: dict) -> bool:
    return data.get('signature', '').startswith('signature_')  # Verifica che la firma inizi con 'signature_'

# ========== Funzioni per le API ==========
# Funzione per la root
@app.get("/")
async def read_root():
    """
    Endpoint di benvenuto.
    """
    return {"message": "Welcome to the sensor data API"}

# Funzione per ottenere i dati del sensore
@app.get("/data")
async def get_data(
    start_date: str = Query(None, description="Data di inizio in formato 'YYYY-MM-DD'"),
    end_date: str = Query(None, description="Data di fine in formato 'YYYY-MM-DD'"),
    start_time: str = Query("00:00:00", description="Orario di inizio in formato 'HH:MM:SS'"),
    end_time: str = Query("23:59:59", description="Orario di fine in formato 'HH:MM:SS'")
):
    """
    Endpoint per recuperare i dati filtrati dal database in base a un intervallo di data e orario.
    Se non sono specificate le date, verranno recuperati tutti i dati.
    """
    logging.debug(f"Received GET request for /data with params: start_date={start_date}, end_date={end_date}")
    try:
        # Se non sono specificate start_date e end_date, usa una data di default
        if not start_date or not end_date:
            start_datetime = datetime(2000, 1, 1, 0, 0, 0)
            end_datetime = datetime.now()
        else:
            start_datetime_str = f"{start_date} {start_time}"
            end_datetime_str = f"{end_date} {end_time}"
            start_datetime = datetime.strptime(start_datetime_str, '%Y-%m-%d %H:%M:%S')
            end_datetime = datetime.strptime(end_datetime_str, '%Y-%m-%d %H:%M:%S')

        # Query per ottenere i dati dal database
        query = """
        SELECT * FROM sensor_data
        WHERE timestamp >= :start_datetime
        AND timestamp <= :end_datetime
        ORDER BY timestamp DESC;
        """

        rows = await database.fetch_all(query, values={'start_datetime': start_datetime, 'end_datetime': end_datetime})

        return {"status": "success", "data": [dict(row) for row in rows]}

    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error fetching data: {str(e)}")

# Funzione per lo stato del sistema
@app.get("/status")
async def get_status():
    """
    Endpoint per restituire lo stato del sistema.
    """
    try:
        if await database.is_connected():
            status = "OK"
        else:
            status = "Database connection failed"
        return {"status": status, "message": "Backend running"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error checking status: {str(e)}")

# Funzione per aggiungere una misura manuale
@app.post("/add_manual_measure")
async def add_manual_measure(misura: SensorData):
    """
    Endpoint per aggiungere una misura manuale nel sistema.
    """
    logging.debug(f"Received POST request for /add_manual_measure: {misura.dict()}")
    try:
        # Valida i dati ricevuti
        if not all([misura.sensor_id, misura.zone]):
            raise HTTPException(status_code=400, detail="Missing required fields in manual measure data")

        # Salva i dati nel database
        await save_to_db(misura)

        return {"status": "success", "message": "Manual measure added successfully"}

    except Exception as e:
        raise HTTPException(status_code=500, detail=f"An error occurred: {str(e)}")

# Funzione per ricevere i dati dal sensore
@app.post("/data")
async def receive_sensor_data(sensor_data: SensorData):
    """
    Endpoint per ricevere i dati del sensore e salvarli nel database.
    """
    try:
        # Verifica la firma dei dati
        if not verify_signature(sensor_data.dict()):
            raise HTTPException(status_code=400, detail="Invalid signature")

        # Salva i dati nel database
        await save_to_db(sensor_data)

        return {"status": "success", "message": "Data received and saved successfully"}

    except Exception as e:
        raise HTTPException(status_code=500, detail=f"An error occurred: {str(e)}")

# ========== Funzione principale per eseguire l'applicazione ==========
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)

