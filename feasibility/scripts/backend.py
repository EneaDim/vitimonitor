# scripts/backend.py

import os
import sqlite3
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from typing import Dict
import threading
import json
import paho.mqtt.client as mqtt

# ========== Parametri ==========
USE_MQTT = os.getenv("USE_MQTT", "0") == "1"
DB_FILE = os.getenv("DB_FILE", "sensordata.db")
MQTT_BROKER = os.getenv("MQTT_BROKER", "localhost")
MQTT_PORT = int(os.getenv("MQTT_PORT", 1883))
MQTT_TOPIC = os.getenv("MQTT_TOPIC", "sensor/data")

app = FastAPI()

# ========== Modelli ==========
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

# ========== DB ==========
conn = sqlite3.connect(DB_FILE, check_same_thread=False)
c = conn.cursor()
c.execute('''
CREATE TABLE IF NOT EXISTS sensor_data (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sensor_id TEXT,
    zone TEXT,
    temperature REAL,
    humidity_air REAL,
    humidity_soil REAL,
    luminosity REAL,
    lat REAL,
    lon REAL,
    signature TEXT,
    manual BOOLEAN DEFAULT 0,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
)
''')
conn.commit()

# ========== Firma mock ==========
def verify_signature(data: dict) -> bool:
    return data.get('signature', '').startswith('signature_')

# ========== Salvataggio ==========
def save_to_db(sensor_data: SensorData, manual: bool = False):
    c.execute('''
    INSERT INTO sensor_data (sensor_id, zone, temperature, humidity_air, humidity_soil, luminosity, lat, lon, signature, manual)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    ''', (
        sensor_data.sensor_id,
        sensor_data.zone,
        sensor_data.temperature,
        sensor_data.humidity_air,
        sensor_data.humidity_soil,
        sensor_data.luminosity,
        sensor_data.gps.lat,
        sensor_data.gps.lon,
        sensor_data.signature,
        manual
    ))
    conn.commit()

# ========== MQTT ==========
def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        sensor_data = SensorData(**payload)
        if verify_signature(payload):
            save_to_db(sensor_data)
            print(f"✅ MQTT: Salvato {sensor_data.sensor_id}")
        else:
            print("⚠️  MQTT: Firma non valida.")
    except Exception as e:
        print(f"❌ MQTT: Errore: {e}")

def start_mqtt_listener():
    client = mqtt.Client()
    client.on_message = on_message
    client.connect(MQTT_BROKER, MQTT_PORT)
    client.subscribe(MQTT_TOPIC)
    client.loop_forever()

if USE_MQTT:
    thread = threading.Thread(target=start_mqtt_listener, daemon=True)
    thread.start()
    print(f"📡 MQTT su {MQTT_BROKER}:{MQTT_PORT} topic '{MQTT_TOPIC}'")

# ========== API ==========
@app.get("/")
async def root():
    return {"message": "Backend attivo"}

@app.post("/data")
async def receive_data(sensor_data: SensorData):
    if not verify_signature(sensor_data.dict()):
        raise HTTPException(status_code=400, detail="Invalid signature")
    save_to_db(sensor_data)
    return {"status": "success"}

@app.get("/data")
async def get_all_data():
    c.execute('''
    SELECT sensor_id, zone, temperature, humidity_air, humidity_soil, luminosity, lat, lon, signature, manual, timestamp
    FROM sensor_data ORDER BY timestamp DESC
    ''')
    rows = c.fetchall()
    result = [
        {
            "sensor_id": r[0],
            "zone": r[1],
            "temperature": r[2],
            "humidity_air": r[3],
            "humidity_soil": r[4],
            "luminosity": r[5],
            "gps": {"lat": r[6], "lon": r[7]},
            "signature": r[8],
            "manual": bool(r[9]),
            "timestamp": r[10]
        } for r in rows
    ]
    return {"data": result}

@app.post("/add_manual_measure")
async def add_manual_measure(sensor_data: SensorData):
    # Firma non obbligatoria per misure manuali
    save_to_db(sensor_data, manual=True)
    return {"status": "manual data saved"}

@app.get("/status")
async def status():
    c.execute("SELECT COUNT(*) FROM sensor_data")
    count = c.fetchone()[0]
    return {"status": "ok", "records": count}

