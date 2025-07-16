import os
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
database = Database(DB_URL)

# ========== Connessione a Redis per caching ==========
redis_client = redis.StrictRedis(host='localhost', port=6379, db=0)

# ========== Creazione dell'istanza FastAPI ==========
app = FastAPI()

# ========== Creazione del modello ==========
class GPSModel(BaseModel):
    lat: float
    lon: float

class SensorData(BaseModel):
    sensor_id: str
    zone: str
    temperature: float
    humidity_air: float
    humidity_soil: float
    luminosity: float
    gps: GPSModel
    signature: str

# ========== Connessione al database ==========
@app.on_event("startup")
async def startup():
    # Prima configura il database
    await configure_database()

    # Connetti al database e crea le tabelle
    await database.connect()
    await create_tables()

@app.on_event("shutdown")
async def shutdown():
    await database.disconnect()

# ========== Funzioni per creare tabelle e indici ==========
# ========== Funzione per creare tabelle e indici ==========
async def create_tables():
    # Usa connessione asincrona di `databases` per eseguire la query
    query = '''
    CREATE TABLE IF NOT EXISTS sensor_data (
        id SERIAL PRIMARY KEY,
        sensor_id TEXT,
        zone TEXT,
        temperature REAL,
        humidity_air REAL,
        humidity_soil REAL,
        luminosity REAL,
        lat REAL,
        lon REAL,
        signature TEXT,
        manual BOOLEAN DEFAULT FALSE,
        timestamp TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
    );
    '''
    async with database.transaction():  # Usa una transazione asincrona per eseguire la query
        await database.execute(query)
        await database.execute('CREATE INDEX IF NOT EXISTS idx_sensor_id ON sensor_data(sensor_id);')
        await database.execute('CREATE INDEX IF NOT EXISTS idx_timestamp ON sensor_data(timestamp);')
        await database.execute('CREATE INDEX IF NOT EXISTS idx_zone ON sensor_data(zone);')

# ========== Funzione per configurare il database e l'utente ==========
# ========== Funzione per configurare il database e l'utente ==========
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

    # Aggiungi questi privilegi aggiuntivi per garantire che l'utente abbia il permesso completo sullo schema
    cur.execute(f"GRANT USAGE, CREATE ON SCHEMA public TO {DB_USER};")
    cur.execute(f"GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO {DB_USER};")
    cur.execute(f"ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT SELECT, INSERT, UPDATE, DELETE ON TABLES TO {DB_USER};")
    cur.execute(f"ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT SELECT, INSERT, UPDATE, DELETE ON SEQUENCES TO {DB_USER};")

    cur.close()
    conn.close()

    print(f"Configurazione completata con successo per il database {DB_NAME}, l'utente {DB_USER}, e i permessi")

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
    cur.execute(f"GRANT ALL PRIVILEGES ON DATABASE {DB_NAME} TO {DB_USER}")
    cur.execute(f"GRANT USAGE ON SCHEMA public TO {DB_USER};")
    cur.execute(f"GRANT CREATE ON SCHEMA public TO {DB_USER};")
    cur.execute(f"GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO {DB_USER};")
    cur.execute(f"ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO {DB_USER};")
    
    # Permessi per operare su tabelle esistenti e future
    cur.execute(f"GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO {DB_USER};")
    cur.execute(f"ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT SELECT, INSERT, UPDATE, DELETE ON TABLES TO {DB_USER};")

    cur.close()
    conn.close()

    print(f"Configurazione completata con successo per il database {DB_NAME}, l'utente {DB_USER}, e i permessi")


# ========== Funzione per salvare i dati nel DB ==========
async def save_to_db(sensor_data: SensorData, manual: bool = False):
    query = '''
    INSERT INTO sensor_data (sensor_id, zone, temperature, humidity_air, humidity_soil, luminosity, lat, lon, signature, manual)
    VALUES (:sensor_id, :zone, :temperature, :humidity_air, :humidity_soil, :luminosity, :lat, :lon, :signature, :manual)
    '''
    await database.execute(query, {
        'sensor_id': sensor_data.sensor_id,
        'zone': sensor_data.zone,
        'temperature': sensor_data.temperature,
        'humidity_air': sensor_data.humidity_air,
        'humidity_soil': sensor_data.humidity_soil,
        'luminosity': sensor_data.luminosity,
        'lat': sensor_data.gps.lat,
        'lon': sensor_data.gps.lon,
        'signature': sensor_data.signature,
        'manual': manual
    })

# ========== Funzioni per monitorare MQTT ==========
def on_message(client, userdata, msg):
    payload = json.loads(msg.payload.decode())
    print(f"Messaggio ricevuto: {payload}")

def start_mqtt_listener():
    client = mqtt.Client()
    client.on_message = on_message
    client.connect(MQTT_BROKER, MQTT_PORT)
    client.subscribe(MQTT_TOPIC)
    client.loop_forever()

@app.on_event("startup")
async def start_mqtt():
    if USE_MQTT:
        thread = threading.Thread(target=start_mqtt_listener, daemon=True)
        thread.start()
        print("MQTT listener avviato")

# ========== Funzione per validare la firma ==========
def verify_signature(data: dict) -> bool:
    return data.get('signature', '').startswith('signature_')

# ========== Funzioni per le API ==========
@app.get("/")
async def read_root():
    """
    Root endpoint, returns a welcome message.
    """
    return {"message": "Welcome to the sensor data API"}

@app.get("/data")
async def get_data(
    start_date: str = Query(..., description="Data di inizio in formato 'YYYY-MM-DD'"),
    end_date: str = Query(..., description="Data di fine in formato 'YYYY-MM-DD'"),
    start_time: str = Query("00:00:00", description="Orario di inizio in formato 'HH:MM:SS'"),
    end_time: str = Query("23:59:59", description="Orario di fine in formato 'HH:MM:SS'")
):
    """
    Endpoint per recuperare i dati filtrati dal database in base a un intervallo di data e orario.
    """
    try:
        # Combina la data con l'orario
        start_datetime_str = f"{start_date} {start_time}"
        end_datetime_str = f"{end_date} {end_time}"

        # Converte le stringhe in datetime
        start_datetime = datetime.strptime(start_datetime_str, '%Y-%m-%d %H:%M:%S')
        end_datetime = datetime.strptime(end_datetime_str, '%Y-%m-%d %H:%M:%S')

        # Query per ottenere i dati dal database
        query = """
        SELECT * FROM sensor_data
        WHERE timestamp >= :start_datetime
        AND timestamp <= :end_datetime
        ORDER BY timestamp DESC;
        """

        # Esegui la query con i parametri
        rows = await database.fetch_all(query, values={'start_datetime': start_datetime, 'end_datetime': end_datetime})

        return {"status": "success", "data": [dict(row) for row in rows]}

    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error fetching data: {str(e)}")

@app.get("/status")
async def get_status():
    """
    Endpoint to return the status of the backend system.
    """
    try:
        status = "OK" if await database.is_connected() else "Database connection failed"
        return {"status": status, "message": "Backend running"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error checking status: {str(e)}")

@app.post("/data")
async def receive_sensor_data(sensor_data: SensorData):
    """
    This endpoint receives sensor data and saves it to the database.
    """
    try:
        # Optionally verify the signature of the data
        if not verify_signature(sensor_data.dict()):
            raise HTTPException(status_code=400, detail="Invalid signature")

        # Save the sensor data to the database
        await save_to_db(sensor_data)

        return {"status": "success", "message": "Data received and saved successfully"}

    except Exception as e:
        raise HTTPException(status_code=500, detail=f"An error occurred: {str(e)}")

# ========== Funzione principale per eseguire l'applicazione ==========
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)

