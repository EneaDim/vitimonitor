import time
import json
import requests
import os
import paho.mqtt.client as mqtt
from sensor_mock import SensorMock

class FirmwareMock:
    def __init__(self, backend_url='http://localhost:8000/data'):
        self.sensor = SensorMock()
        self.backend_url = backend_url

        # Leggi variabili ambiente per MQTT
        self.use_mqtt = os.getenv('USE_MQTT', '0') == '1'
        self.mqtt_broker = os.getenv('MQTT_BROKER', 'localhost')
        self.mqtt_port = int(os.getenv('MQTT_PORT', 1883))
        self.mqtt_topic = os.getenv('MQTT_TOPIC', 'sensor/data')

        if self.use_mqtt:
            self.mqtt_client = mqtt.Client()
            try:
                self.mqtt_client.connect(self.mqtt_broker, self.mqtt_port)
                self.mqtt_client.loop_start()
                print(f"Connesso a MQTT Broker {self.mqtt_broker}:{self.mqtt_port}")
            except Exception as e:
                print(f"Errore connessione MQTT: {e}")
                self.use_mqtt = False  # fallback a HTTP POST

    def run(self):
        while True:
            data = {
                'temperature': self.sensor.read_temperature(),
                'humidity': self.sensor.read_humidity(),
                'luminosity': self.sensor.read_luminosity(),
                'gps': self.sensor.read_gps(),
            }
            data['signature'] = self.sensor.sign_data(data)

            if self.use_mqtt:
                try:
                    payload = json.dumps(data)
                    self.mqtt_client.publish(self.mqtt_topic, payload)
                    print(f"MQTT: Pubblicati dati su {self.mqtt_topic}: {payload}")
                except Exception as e:
                    print(f"Errore pubblicazione MQTT: {e}")
            else:
                try:
                    response = requests.post(self.backend_url, json=data)
                    print(f"HTTP POST: Inviati dati: {data}, Response: {response.status_code}")
                except Exception as e:
                    print(f"Errore invio dati al backend: {e}")

            time.sleep(5)

if __name__ == "__main__":
    fw = FirmwareMock()
    fw.run()

