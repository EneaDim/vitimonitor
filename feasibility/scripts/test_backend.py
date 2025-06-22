import sys
import pytest
from fastapi.testclient import TestClient
from backend import app

client = TestClient(app)

def test_root():
    response = client.get("/")
    assert response.status_code == 200
    assert response.json() == {"message": "Backend attivo!"}

def test_post_and_get_data():
    sample_data = {
        "temperature": 25.0,
        "humidity": 40.0,
        "luminosity": 300.0,
        "gps": {"lat": 45.0, "lon": 9.0},
        "signature": "dummy_signature"
    }
    response_post = client.post("/data", json=sample_data)
    assert response_post.status_code == 200
    assert response_post.json() == {"status": "success"}

    response_get = client.get("/data")
    assert response_get.status_code == 200
    data = response_get.json()
    assert "data" in data
    assert len(data["data"]) > 0

def test_post_data_invalid_signature():
    invalid_data = {
        "temperature": 25.0,
        "humidity": 40.0,
        "luminosity": 300.0,
        "gps": {"lat": 45.0, "lon": 9.0},
        "signature": "wrong_signature"
    }
    response = client.post("/data", json=invalid_data)
    assert response.status_code == 400
    assert response.json() == {"detail": "Invalid signature"}

def test_status_endpoint():
    response = client.get("/status")
    assert response.status_code == 200
    json_data = response.json()
    assert "status" in json_data and json_data["status"] == "ok"
    assert "records" in json_data and isinstance(json_data["records"], int)

def test_post_data_missing_fields():
    incomplete_data = {
        "temperature": 25.0,
        "humidity": 40.0,
        # luminosity is missing here
        "gps": {"lat": 45.0, "lon": 9.0},
        "signature": "dummy_signature"
    }
    response = client.post("/data", json=incomplete_data)
    assert response.status_code == 422  # Unprocessable Entity

def test_get_data_empty():
    response = client.get("/data")
    assert response.status_code == 200
    json_data = response.json()
    assert "data" in json_data
    assert isinstance(json_data["data"], list)

if __name__ == "__main__":
    # Esegue pytest sul file stesso (o su tutta la cartella, se vuoi)
    result = pytest.main(["-q", "--disable-warnings"])

    if result == 0:
        print("\n✅ Tutti i test sono passati con successo!")
        sys.exit(0)
    else:
        print("\n❌ Alcuni test sono falliti.")
        sys.exit(1)

