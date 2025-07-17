# ========= Variabili =========
VENV_DIR := .venv
PYTHON := $(VENV_DIR)/bin/python
UVICORN := $(VENV_DIR)/bin/uvicorn
STREAMLIT := $(VENV_DIR)/bin/streamlit
APP := scripts.backend:app
HOST := 0.0.0.0
PORT := 8000
COLOR := \033[38;5;214m
RESET := \033[0m
ECHO := @echo "$(COLOR)▶$(RESET)"
ORANGE := \033[38;5;208m
NC := \033[0m

# ======== Configurazione ========
SRC_DIR := scripts
DB_FILE := sensordata.db
CONFIG_FILE := config.yml
TEST_FILE := $(SRC_DIR)/test_backend.py
PYTEST := $(VENV_DIR)/bin/pytest
SCRIPT := $(SRC_DIR)/setup_postgres.sh

# ========= Configurazione variabili direttamente nel Makefile =========
# Variabili per configurazione PostgreSQL e MQTT
DB_URL := "postgresql://postgrep:postgrep@localhost/sensordata"
DB_USER := "postgrep"
DB_PASSWORD := "postgrep"
DB_NAME := "sensordata"
SUPERUSER_NAME := "admin_user"
SUPERUSER_PASSWORD := "newpassword"
USE_MQTT := 1
MQTT_BROKER := "localhost"
MQTT_PORT := 1883
MQTT_TOPIC := "sensor/data"

.PHONY: help run-all firmware curlroot curlgetdata curlstatus lint test clean cleandb frontend init venv freeze gen-config

# Aiuto
help:
	@echo ""
	@echo "$(COLOR)Comandi disponibili:$(RESET)"
	@echo ""
	@echo "$(COLOR)⚙️  Setup & Ambiente:$(RESET)"
	@echo "  $(COLOR)make init$(RESET)             - Crea venv, installa pacchetti, setup cartelle e config.yml"
	@echo "  $(COLOR)make venv$(RESET)             - Mostra comando per attivare l’ambiente virtuale"
	@echo "  $(COLOR)make freeze$(RESET)           - Aggiorna requirements.txt"
	@echo ""
	@echo "$(COLOR)🚀 Avvio servizi:$(RESET)"
	@echo "  $(COLOR)make backend$(RESET)          - Avvia il backend FastAPI con Uvicorn"
	@echo "  $(COLOR)make firmware$(RESET)         - Avvia il firmware mock (legge config.yml)"
	@echo "  $(COLOR)make frontend$(RESET)         - Avvia il frontend Streamlit"
	@echo "  $(COLOR)make run-all$(RESET)          - Avvia backend, firmware e frontend in tmux"
	@echo ""
	@echo "$(COLOR)🧰 Debug & Test:$(RESET)"
	@echo "  $(COLOR)make lint$(RESET)             - Linting del codice Python con flake8"
	@echo "  $(COLOR)make test$(RESET)             - Esegue i test automatici"
	@echo "  $(COLOR)make curlroot$(RESET)         - Chiamata curl a / (root)"
	@echo "  $(COLOR)make curlgetdata$(RESET)      - Chiamata curl GET a /data"
	@echo "  $(COLOR)make curlstatus$(RESET)       - Chiamata curl GET a /status"
	@echo ""
	@echo "$(COLOR)📝 Configurazione:$(RESET)"
	@echo "  $(COLOR)make show-config$(RESET)      - Mostra il contenuto corrente di config.yml"
	@echo "  $(COLOR)make update-config$(RESET)    - Aggiorna un valore in config.yml (es. KEY=send_interval VALUE=2)"
	@echo ""
	@echo "$(COLOR)🧹 Pulizia:$(RESET)"
	@echo "  $(COLOR)make clean$(RESET)            - Rimuove file temporanei e cache"
	@echo "  $(COLOR)make cleandb$(RESET)          - Elimina il file del database SQLite"
	@echo ""

setup-postgres:
	$(ECHO) Configurando PostgreSQL...
	@sudo -u postgres psql -c "SELECT 1 FROM pg_database WHERE datname = '$(DB_NAME)'" | grep -q 1 || sudo -u postgres psql -c "CREATE DATABASE $(DB_NAME);"
	@sudo -u postgres psql -c "SELECT 1 FROM pg_roles WHERE rolname = '$(DB_USER)'" | grep -q 1 || sudo -u postgres psql -c "CREATE ROLE $(DB_USER) WITH LOGIN PASSWORD '$(DB_PASSWORD)';"
	@sudo -u postgres psql -c "SELECT 1 FROM pg_roles WHERE rolname = '$(SUPERUSER_NAME)'" | grep -q 1 || sudo -u postgres psql -c "CREATE ROLE $(SUPERUSER_NAME) WITH LOGIN PASSWORD '$(SUPERUSER_PASSWORD)' SUPERUSER;"
	@sudo -u postgres psql -d $(DB_NAME) -c "GRANT CONNECT ON DATABASE $(DB_NAME) TO $(DB_USER);"
	@sudo -u postgres psql -d $(DB_NAME) -c "GRANT USAGE, CREATE ON SCHEMA public TO $(DB_USER);"
	@sudo -u postgres psql -d $(DB_NAME) -c "GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO $(DB_USER);"
	@sudo -u postgres psql -d $(DB_NAME) -c "GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO $(DB_USER);"
	@sudo -u postgres psql -d $(DB_NAME) -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO $(DB_USER);"
	@sudo -u postgres psql -d $(DB_NAME) -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO $(DB_USER);"
	@sudo -u postgres psql -d $(DB_NAME) -c "GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO $(DB_USER);"
	@sudo -u postgres psql -d $(DB_NAME) -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT SELECT, INSERT, UPDATE, DELETE ON TABLES TO $(DB_USER);"
	@sudo -u postgres psql -d $(DB_NAME) -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT USAGE, SELECT, UPDATE ON SEQUENCES TO $(DB_USER);"
	@echo "PostgreSQL configurato correttamente."

delete-db:
	@echo "Verificando l'esistenza del database $(DB_NAME)..."
	@DB_EXISTS=$(shell sudo -u postgres psql -tAc "SELECT 1 FROM pg_database WHERE datname='$(DB_NAME)'") && \
	if [ -z "$$DB_EXISTS" ]; then \
		echo "Il database $(DB_NAME) non esiste."; \
	else \
		echo "Reassegnando la proprietà degli oggetti all'utente admin_user..."; \
		sudo -u postgres psql -d $(DB_NAME) -c "REASSIGN OWNED BY $(DB_USER) TO admin_user;" && \
		echo "Proprietà degli oggetti riassegnate."; \
		echo "Rimuovendo i privilegi dell'utente $(DB_USER)..."; \
		sudo -u postgres psql -d $(DB_NAME) -c "DROP OWNED BY $(DB_USER);" && \
		echo "Privilegi revocati."; \
		echo "Rimuovendo gli oggetti associati all'utente $(DB_USER)..."; \
		sudo -u postgres psql -d $(DB_NAME) -c "DROP OWNED BY $(DB_USER) CASCADE;" && \
		echo "Oggetti associati rimossi."; \
		echo "Eliminando il ruolo $(DB_USER)..."; \
		sudo -u postgres psql -c "DROP ROLE IF EXISTS $(DB_USER);" && \
		echo "Ruolo $(DB_USER) eliminato."; \
		echo "Eliminando il database $(DB_NAME)..."; \
		sudo -u postgres psql -c "DROP DATABASE IF EXISTS $(DB_NAME);" && \
		echo "Database $(DB_NAME) eliminato con successo."; \
	fi

# Inizializza ambiente virtuale
init:
	@echo "$(ORANGE)🔧 Inizializzazione progetto...$(NC)"
	@if [ ! -d $(VENV_DIR) ]; then \
		echo "$(ORANGE)📦 Creo ambiente virtuale $(VENV_DIR)$(NC)"; \
		python3 -m venv $(VENV_DIR); \
	fi
	@echo "$(ORANGE)📥 Attivo $(VENV_DIR) e installo dipendenze$(NC)"
	@. $(VENV_DIR)/bin/activate && pip install --upgrade pip && pip install -r requirements.txt
	@mkdir -p data scripts && touch scripts/__init__.py
	@$(MAKE) gen-config
	@echo "$(ORANGE)✅ Setup completato$(NC)"

venv:
	@echo "$(ORANGE)✨ Attiva ambiente virtuale con:$(NC)"
	@echo "source $(VENV_DIR)/bin/activate"

freeze:
	@echo "$(ORANGE)📦 Aggiorno requirements.txt$(NC)"
	@. $(VENV_DIR)/bin/activate && pip freeze > requirements.txt

run-all:
	$(ECHO) Avvio tutti i servizi in sessione tmux...
	@tmux new-session -d -s app-backend 'make backend'
	@tmux split-window -h -t app-backend 'sleep 3 && make firmware'
	@tmux split-window -v -t app-backend 'make frontend'
	@tmux select-layout -t app-backend tiled
	@tmux attach -t app-backend

show-config:
	$(ECHO) Contenuto configurazione attuale:
	@cat $(CONFIG_FILE) | sed 's/^/    /'

update-config:
ifndef KEY
	$(error Devi specificare KEY=nome_chiave e VALUE=valore, es: make update-config KEY=send_interval VALUE=10)
endif
ifndef VALUE
	$(error Devi specificare VALUE=valore, es: make update-config KEY=send_interval VALUE=10)
endif
	$(ECHO) Aggiorno $(CONFIG_FILE): $(KEY) -> $(VALUE)
	@$(PYTHON) -c "import yaml; \
	with open('$(CONFIG_FILE)') as f: cfg=yaml.safe_load(f); \
	cfg['$(KEY)']='$(VALUE)'; \
	yaml.dump(cfg,open('$(CONFIG_FILE)','w'))"
	$(ECHO) Configurazione aggiornata: $(KEY)=$(VALUE)

# Avvio FastAPI backend
backend:
	$(ECHO) Avvio FastAPI backend...
	DB_URL=$(DB_URL) \
	DB_USER=$(DB_USER) \
	DB_PASSWORD=$(DB_PASSWORD) \
	DB_NAME=$(DB_NAME) \
	SUPERUSER_NAME=$(SUPERUSER_NAME) \
	SUPERUSER_PASSWORD=$(SUPERUSER_PASSWORD) \
	USE_MQTT=$(USE_MQTT) \
	MQTT_BROKER=$(MQTT_BROKER) \
	MQTT_PORT=$(MQTT_PORT) \
	MQTT_TOPIC=$(MQTT_TOPIC) \
	$(UVICORN) $(APP) --reload --host $(HOST) --port $(PORT)

firmware:
	$(ECHO) Avvio firmware mock...
	$(PYTHON) $(SRC_DIR)/firmware_mock.py --config $(CONFIG_FILE)

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

