import random

class SensorMock:
    def read_temperature(self):
        return 20 + random.uniform(-5, 5)

    def read_humidity(self):
        return 50 + random.uniform(-10, 10)

    def read_luminosity(self):
        return 300 + random.uniform(-50, 50)

    def read_gps(self):
        return {'lat': 45.0 + random.uniform(-0.01, 0.01), 'lon': 9.0 + random.uniform(-0.01, 0.01)}

    def sign_data(self, data):
        return "dummy_signature"

