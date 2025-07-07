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

.PHONY: help run firmware curlroot curlgetdata curlstatus lint test clean cleandb frontend init venv freeze gen-config

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

backend:
	$(ECHO) Avvio FastAPI backend...
	DB_FILE=$(DB_FILE) $(UVICORN) $(APP) --reload --host $(HOST) --port $(PORT)

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

