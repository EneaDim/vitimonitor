import time
import json
import requests
from sensor_mock import SensorMock

class FirmwareMock:
    def __init__(self, backend_url='http://localhost:8000/data'):
        self.sensor = SensorMock()
        self.backend_url = backend_url

    def run(self):
        while True:
            data = {
                'temperature': self.sensor.read_temperature(),
                'humidity': self.sensor.read_humidity(),
                'luminosity': self.sensor.read_luminosity(),
                'gps': self.sensor.read_gps(),
            }
            data['signature'] = self.sensor.sign_data(data)
            try:
                response = requests.post(self.backend_url, json=data)
                print(f"Sent data: {data}, Response: {response.status_code}")
            except Exception as e:
                print(f"Errore invio dati al backend: {e}")
            time.sleep(5)

if __name__ == "__main__":
    fw = FirmwareMock()
    fw.run()
