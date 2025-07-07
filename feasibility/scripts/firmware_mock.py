import time
import json
import requests
import os
import paho.mqtt.client as mqtt
from sensor_mock import SensorMock

class FirmwareMock:
    """
    Simula un firmware che raccoglie dati da più sensori distinti
    e li invia ciclicamente al backend o via MQTT.
    """
    def __init__(self, backend_url='http://localhost:8000/data'):
        self.backend_url = backend_url

        # Configura MQTT
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

        # Crea sensori distinti
        self.sensors = [
            SensorMock(sensor_id="sensor_1", zone="zone_north"),
            SensorMock(sensor_id="sensor_2", zone="zone_north"),
            SensorMock(sensor_id="sensor_3", zone="zone_south"),
            SensorMock(sensor_id="sensor_4", zone="zone_south"),
        ]

    def send_data(self, data):
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

    def run(self):
        while True:
            for sensor in self.sensors:
                data = {
                    'sensor_id': sensor.sensor_id,
                    'zone': sensor.zone,
                    'temperature': sensor.read_temperature(),
                    'humidity': sensor.read_humidity(),
                    'luminosity': sensor.read_luminosity(),
                    'gps': sensor.read_gps(),
                }
                data['signature'] = sensor.sign_data(data)
                self.send_data(data)
            time.sleep(5)


if __name__ == "__main__":
    fw = FirmwareMock()
    fw.run()

