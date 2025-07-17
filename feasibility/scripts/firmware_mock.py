import time
import json
import yaml
import argparse
import requests
import os
from sensor_mock import SensorMock
import paho.mqtt.client as mqtt
from colorama import Fore, Style, init

init(autoreset=True)

class FirmwareMock:
    def __init__(self, config):
        self.config = config
        self.backend_url = config.get("backend_url", "http://localhost:8000/data")

        self.use_mqtt = config.get("use_mqtt", False)
        self.mqtt_broker = config.get("mqtt_broker", "localhost")
        self.mqtt_port = config.get("mqtt_port", 1883)
        self.mqtt_topic = config.get("mqtt_topic", "sensor/data")

        self.sensors_per_zone = config.get("sensors_per_zone", 2)
        self.send_interval = config.get("send_interval", 5)
        self.zones = config.get("zones", ["zone_north", "zone_south"])

        self.soglie = config.get("soglie", {})

        if self.use_mqtt:
            self.mqtt_client = mqtt.Client()
            try:
                self.mqtt_client.connect(self.mqtt_broker, self.mqtt_port)
                self.mqtt_client.loop_start()
                print(f"{Fore.GREEN}✅ Connesso a MQTT Broker {self.mqtt_broker}:{self.mqtt_port}")
            except Exception as e:
                print(f"{Fore.RED}⚠️ Errore connessione MQTT: {e}")
                self.use_mqtt = False

        self.sensors = []
        for zone in self.zones:
            for i in range(1, self.sensors_per_zone + 1):
                sensor_id = f"{zone}_sensor_{i}"
                self.sensors.append(SensorMock(sensor_id=sensor_id, zone=zone))

        print(f"{Fore.CYAN}🚀 Creati {len(self.sensors)} sensori su {len(self.zones)} zone.")

    def check_anomaly(self, metric, value):
        min_val, max_val = self.soglie.get(metric, (None, None))
        if min_val is not None and value < min_val:
            return True
        if max_val is not None and value > max_val:
            return True
        return False

    def send_data(self, data):
        anomalies = []
        for metric in ["temperature", "humidity_air", "humidity_soil", "luminosity"]:
            if self.check_anomaly(metric, data.get(metric)):
                anomalies.append(metric)

        if self.use_mqtt:
            payload = json.dumps(data)
            self.mqtt_client.publish(self.mqtt_topic, payload)
        else:
            requests.post(self.backend_url, json=data)

        if anomalies:
            print(f"{Fore.RED}⚠️ {data['sensor_id']} ANOMALIA su: {', '.join(anomalies)} | Timestamp: {data['timestamp']}")
        else:
            print(f"{Fore.GREEN}✅ {data['sensor_id']} OK | Timestamp: {data['timestamp']}")

    def run(self):
        while True:
            for sensor in self.sensors:
                data = {
                    'sensor_id': sensor.sensor_id,
                    'zone': sensor.zone,
                    'temperature': sensor.read_temperature(),
                    'humidity_air': sensor.read_humidity_air(),
                    'humidity_soil': sensor.read_humidity_soil(),
                    'luminosity': sensor.read_luminosity(),
                    #'gps': sensor.read_gps(),
                    'manual' : False,
                    'timestamp': sensor.timestamp()
                }
                data['signature'] = sensor.sign_data(data)
                self.send_data(data)
            time.sleep(self.send_interval)

def load_config(path):
    if not os.path.exists(path):
        print(f"{Fore.YELLOW}⚠️ Config file '{path}' non trovato. Uso configurazione di default.")
        return {}
    with open(path, 'r') as f:
        return yaml.safe_load(f)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Firmware Mock - Sensori multipli")
    parser.add_argument("--config", default="config.yml", help="Percorso file di configurazione YAML")
    args = parser.parse_args()

    config = load_config(args.config)
    fw = FirmwareMock(config)
    fw.run()

