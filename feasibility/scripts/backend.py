from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from typing import Dict, List
import sqlite3

app = FastAPI()

class SensorData(BaseModel):
    temperature: float
    humidity: float
    luminosity: float
    gps: Dict[str, float]
    signature: str

# Connessione SQLite
conn = sqlite3.connect('sensordata.db', check_same_thread=False)
c = conn.cursor()
c.execute('''
    CREATE TABLE IF NOT EXISTS sensor_data (
        temperature REAL,
        humidity REAL,
        luminosity REAL,
        lat REAL,
        lon REAL,
        signature TEXT,
        timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
    )
''')
conn.commit()

# Firma digitale mock
def verify_signature(data: dict) -> bool:
    return data.get('signature') == 'dummy_signature'

# Endpoint POST per ricevere dati
@app.post("/data")
async def receive_data(sensor_data: SensorData):
    if not verify_signature(sensor_data.dict()):
        raise HTTPException(status_code=400, detail="Invalid signature")
    
    c.execute('''
        INSERT INTO sensor_data (temperature, humidity, luminosity, lat, lon, signature)
        VALUES (?, ?, ?, ?, ?, ?)
    ''', (
        sensor_data.temperature,
        sensor_data.humidity,
        sensor_data.luminosity,
        sensor_data.gps['lat'],
        sensor_data.gps['lon'],
        sensor_data.signature
    ))
    conn.commit()
    return {"status": "success"}

# 🆕 Endpoint GET per leggere tutti i dati
@app.get("/data")
async def get_all_data():
    c.execute("SELECT temperature, humidity, luminosity, lat, lon, signature, timestamp FROM sensor_data ORDER BY timestamp DESC")
    rows = c.fetchall()
    result = [
        {
            "temperature": r[0],
            "humidity": r[1],
            "luminosity": r[2],
            "gps": {"lat": r[3], "lon": r[4]},
            "signature": r[5],
            "timestamp": r[6]
        } for r in rows
    ]
    return {"data": result}

# 🆕 Endpoint GET per status
@app.get("/status")
async def status():
    try:
        c.execute("SELECT COUNT(*) FROM sensor_data")
        count = c.fetchone()[0]
        return {
            "status": "ok",
            "records": count
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"DB error: {str(e)}")

# 🆕 Endpoint GET base
@app.get("/")
async def root():
    return {"message": "Backend attivo!"}

