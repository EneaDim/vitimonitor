import random
from datetime import datetime, timedelta

class SensorMock:
    """
    Simula un singolo sensore identificabile, con zona opzionale.
    Genera valori realistici con occasionali anomalie.
    """
    def __init__(self, sensor_id: str, zone: str = None):
        self.sensor_id = sensor_id
        self.zone = zone or "default"

    def read_temperature(self):
        base = 20 + random.uniform(-5, 5)
        # 5% probabilità di anomalia
        if random.random() < 0.05:
            return random.choice([-10, 50])
        return base

    def read_humidity(self):
        base = 50 + random.uniform(-10, 10)
        if random.random() < 0.05:
            return random.choice([0, 100])
        return base

    def read_luminosity(self):
        base = 300 + random.uniform(-50, 50)
        if random.random() < 0.05:
            return random.choice([0, 200000])
        return base

    def read_gps(self):
        return {
            'lat': 45.0 + random.uniform(-0.01, 0.01),
            'lon': 9.0 + random.uniform(-0.01, 0.01)
        }

    def sign_data(self, data):
        return f"signature_{self.sensor_id}"

    def timestamp(self):
        # Falsa asincronia: ogni sensore è “indietro” fino a 3 secondi
        lag = random.randint(0, 3)
        return (datetime.utcnow() - timedelta(seconds=lag)).isoformat()

