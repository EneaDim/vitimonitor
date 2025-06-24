# ========= Variabili =========
PYTHON := .venv/bin/python
UVICORN := .venv/bin/uvicorn
STREAMLIT := .venv/bin/streamlit
APP := scripts.backend:app
HOST := 0.0.0.0
PORT := 8000
COLOR := \033[38;5;214m
RESET := \033[0m
ECHO := @echo "$(COLOR)▶$(RESET)"

# ======== Configurazione ========
SRC_DIR := scripts
DB_FILE := sensordata.db
USE_MQTT ?= 1
MQTT_BROKER := localhost
MQTT_PORT := 1883
MQTT_TOPIC := sensor/data
TEST_FILE := $(SRC_DIR)/test_backend.py
PYTEST := .venv/bin/pytest

.PHONY: help run firmware curlroot curlgetdata curlstatus lint test clean cleandb frontend

help:
	@echo ""
	@echo "$(COLOR)Available commands:$(RESET)"
	@echo "  $(COLOR)make run$(RESET)          - Avvia il backend FastAPI con Uvicorn"
	@echo "  $(COLOR)make firmware$(RESET)     - Avvia il firmware mock (con POST o MQTT)"
	@echo "  $(COLOR)make frontend$(RESET)     - Avvia il frontend Streamlit"
	@echo "  $(COLOR)make curlroot$(RESET)     - Chiamata curl a / (root)"
	@echo "  $(COLOR)make curlgetdata$(RESET)  - Chiamata curl GET a /data"
	@echo "  $(COLOR)make curlstatus$(RESET)   - Chiamata curl GET a /status"
	@echo "  $(COLOR)make lint$(RESET)         - Linting del codice Python con flake8"
	@echo "  $(COLOR)make test$(RESET)         - Esegue i test automatici"
	@echo "  $(COLOR)make clean$(RESET)        - Rimuove file temporanei e cache"
	@echo "  $(COLOR)make cleandb$(RESET)      - Elimina il file del database SQLite"
	@echo ""

tutorial:
	$(ECHO) Avvio frontend Streamlit...
	$(STREAMLIT) run $(SRC_DIR)/tutorial.py

run:
	$(ECHO) Avvio FastAPI backend...
	USE_MQTT=$(USE_MQTT) DB_FILE=$(DB_FILE) $(UVICORN) $(APP) --reload --host $(HOST) --port $(PORT)

firmware:
	$(ECHO) Avvio firmware mock POST o MQTT...
	USE_MQTT=$(USE_MQTT) MQTT_BROKER=$(MQTT_BROKER) MQTT_PORT=$(MQTT_PORT) MQTT_TOPIC=$(MQTT_TOPIC) $(PYTHON) $(SRC_DIR)/firmware_mock.py

frontend:
	$(ECHO) Avvio frontend Streamlit...
	$(STREAMLIT) run $(SRC_DIR)/frontend.py

curlroot:
	$(ECHO) Chiamata curl a /root
	@curl -s http://localhost:$(PORT)/ -w "\n"

curlgetdata:
	$(ECHO) Chiamata curl GET a /data
	@curl -s http://localhost:$(PORT)/data -w "\n"

curlstatus:
	$(ECHO) Chiamata curl GET a /status
	@curl -s http://localhost:$(PORT)/status -w "\n"

lint:
	$(ECHO) Linting del codice...
	$(PYTHON) -m flake8 $(SRC_DIR)

test:
	$(ECHO) Esecuzione test...
	$(PYTHON) $(TEST_FILE) && echo "$(COLOR)✅ Test superati!$(RESET)" || (echo "$(COLOR)❌ Test falliti!$(RESET)"; exit 1)

cleandb:
	$(ECHO) Pulizia database...
	rm -f $(DB_FILE)

clean:
	$(ECHO) Pulizia file temporanei...
	find . -type d -name "__pycache__" -exec rm -r {} +
	find . -type f -name "*.pyc" -delete
	rm -rf .pytest_cache
	rm -rf test_$(DB_FILE)

