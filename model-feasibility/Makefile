# ========= COLORI =========
ORANGE := \033[38;5;214m
RESET := \033[0m
ECHO := @echo "$(ORANGE)▶$(RESET)"

# ========= VARIABILI =========
PYTHON := .venv/bin/python
UVICORN := .venv/bin/uvicorn
STREAMLIT := .venv/bin/streamlit

APP := scripts.backend:app
HOST := 0.0.0.0
PORT := 8000

SRC_DIR := scripts
DB_FILE := sensordata.db
USE_MQTT ?= 1
MQTT_BROKER := localhost
MQTT_PORT := 1883
MQTT_TOPIC := sensor/data

TEST_FILE := $(SRC_DIR)/test_backend.py
PYTEST := .venv/bin/pytest

TMUX_SESSION := iot-app

# ========= TARGETS =========

help:
	@echo ""
	@echo "$(ORANGE)Available commands:$(RESET)"
	@echo "  init           - Setup ambiente virtuale e dipendenze"
	@echo "  venv           - Come attivare l'ambiente virtuale"
	@echo "  freeze         - Aggiorna requirements.txt"
	@echo "  backend        - Avvia backend FastAPI"
	@echo "  firmware       - Avvia firmware mock (POST o MQTT)"
	@echo "  frontend       - Avvia frontend Streamlit"
	@echo "  test           - Esegui test automatici"
	@echo "  lint           - Linting codice Python"
	@echo "  clean          - Pulisce file temporanei"
	@echo "  cleandb        - Rimuove file database"
	@echo "  kill-backend   - Termina backend FastAPI"
	@echo "  kill-firmware  - Termina firmware mock"
	@echo "  stop-all       - Termina tutto e chiudi tmux"
	@echo "  run-all        - Avvia backend, firmware e frontend in tmux splittata"
	@echo ""

init:
	@echo "$(ORANGE)🔧 Setup progetto...$(RESET)"
	@if [ ! -d .venv ]; then \
		echo "$(ORANGE)📦 Creo ambiente virtuale .venv$(RESET)"; \
		python3 -m venv .venv; \
	fi
	@echo "$(ORANGE)📥 Installo dipendenze$(RESET)"
	@. .venv/bin/activate && pip install --upgrade pip && pip install -r requirements.txt
	@mkdir -p $(SRC_DIR) && touch $(SRC_DIR)/__init__.py
	@echo "$(ORANGE)✅ Setup completato$(RESET)"

venv:
	@echo "$(ORANGE)✨ Attiva ambiente virtuale con:$(RESET)"
	@echo "source .venv/bin/activate"

freeze:
	@echo "$(ORANGE)📦 Aggiorno requirements.txt$(RESET)"
	@. .venv/bin/activate && pip freeze > requirements.txt

backend:
	$(ECHO) Avvio backend FastAPI...
	USE_MQTT=$(USE_MQTT) DB_FILE=$(DB_FILE) $(UVICORN) $(APP) --reload --host $(HOST) --port $(PORT)

firmware:
	$(ECHO) Avvio firmware mock...
	USE_MQTT=$(USE_MQTT) MQTT_BROKER=$(MQTT_BROKER) MQTT_PORT=$(MQTT_PORT) MQTT_TOPIC=$(MQTT_TOPIC) $(PYTHON) $(SRC_DIR)/firmware_mock.py

frontend:
	$(ECHO) Avvio frontend Streamlit ...
	$(STREAMLIT) run $(SRC_DIR)/frontend.py --server.headless true

test:
	$(ECHO) Esecuzione test...
	$(PYTHON) $(TEST_FILE) && echo "$(ORANGE)✅ Test superati!$(RESET)" || (echo "$(ORANGE)❌ Test falliti!$(RESET)"; exit 1)

lint:
	$(ECHO) Linting codice...
	$(PYTHON) -m flake8 $(SRC_DIR)

clean:
	$(ECHO) Pulizia file temporanei...
	find . -type d -name "__pycache__" -exec rm -r {} +
	find . -type f -name "*.pyc" -delete
	rm -rf .pytest_cache test_$(DB_FILE)

cleandb:
	$(ECHO) Rimuovo file database...
	rm -f $(DB_FILE)

kill-backend:
	$(ECHO) Terminazione backend FastAPI...
	@if pgrep -f "uvicorn $(APP)" > /dev/null; then \
		pkill -f "uvicorn $(APP)"; \
		echo "$(ORANGE)Backend terminato$(RESET)"; \
	else \
		echo "$(ORANGE)Nessun backend attivo$(RESET)"; \
	fi

kill-firmware:
	$(ECHO) Terminazione firmware mock...
	@if pgrep -f "$(SRC_DIR)/firmware_mock.py" > /dev/null; then \
		pkill -f "$(SRC_DIR)/firmware_mock.py"; \
		echo "$(ORANGE)Firmware mock terminato$(RESET)"; \
	else \
		echo "$(ORANGE)Nessun firmware attivo$(RESET)"; \
	fi

stop-all: kill-backend kill-firmware
	$(ECHO) Terminazione sessione tmux...
	@if tmux has-session -t $(TMUX_SESSION) 2>/dev/null; then \
		tmux kill-session -t $(TMUX_SESSION); \
		echo "$(ORANGE)Sessione tmux terminata$(RESET)"; \
	else \
		echo "$(ORANGE)Nessuna sessione tmux attiva$(RESET)"; \
	fi

run-all:
	$(ECHO) Avvio tutti i componenti in sessione tmux splittata...
	@if tmux has-session -t $(TMUX_SESSION) 2>/dev/null; then \
		echo "$(ORANGE)Sessione tmux '$(TMUX_SESSION)' già esistente. Usa 'make stop-all' prima$(RESET)"; \
		exit 1; \
	fi; \
	tmux new-session -d -s $(TMUX_SESSION) "make backend"; \
	tmux split-window -v -t $(TMUX_SESSION) "make firmware"; \
	# Per mantenere la finestra frontend aperta, esegui il comando dentro bash interattivo
	tmux split-window -h -t $(TMUX_SESSION) "bash -c 'make frontend; exec bash'"; \
	tmux select-layout -t $(TMUX_SESSION) tiled; \
	tmux attach-session -t $(TMUX_SESSION)

