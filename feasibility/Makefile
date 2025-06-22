# Variabili
PYTHON := .venv/bin/python
UVICORN := .venv/bin/uvicorn
APP := scripts.backend:app
HOST := 0.0.0.0
PORT := 8000
COLOR := \033[38;5;214m
RESET := \033[0m

.PHONY: help run firmware curlroot curlgetdata curlstatus lint test clean

help:
	@echo ""
	@echo "$(COLOR)Available commands:$(RESET)"
	@echo "  $(COLOR)make run$(RESET)          - Avvia il backend FastAPI con Uvicorn"
	@echo "  $(COLOR)make firmware$(RESET)     - Avvia il firmware mock che POSTa dati ogni 5s"
	@echo "  $(COLOR)make curlroot$(RESET)     - Chiamata curl a / (root) con newline"
	@echo "  $(COLOR)make curlgetdata$(RESET)  - Chiamata curl GET a /data con newline"
	@echo "  $(COLOR)make curlstatus$(RESET)   - Chiamata curl GET a /status con newline"
	@echo "  $(COLOR)make lint$(RESET)         - Linting del codice Python con flake8"
	@echo "  $(COLOR)make test$(RESET)         - Esegue eventuali test (da definire)"
	@echo "  $(COLOR)make clean$(RESET)        - Rimuove file temporanei e pycache"
	@echo ""

run:
	@echo "$(COLOR)▶ Avvio FastAPI backend...$(RESET)"
	$(UVICORN) $(APP) --reload --host $(HOST) --port $(PORT)

firmware:
	@echo "$(COLOR)▶ Avvio firmware mock (POST dati ogni 5s)...$(RESET)"
	$(PYTHON) scripts/firmware_mock.py

curlroot:
	@echo "$(COLOR)▶ Chiamata curl a / con newline$(RESET)"
	@curl -s http://localhost:8000/ -w "\n"

curlgetdata:
	@echo "$(COLOR)▶ Chiamata curl GET a /data con newline$(RESET)"
	@curl -s http://localhost:8000/data -w "\n"

curlstatus:
	@echo "$(COLOR)▶ Chiamata curl GET a /status con newline$(RESET)"
	@curl -s http://localhost:8000/status -w "\n"

lint:
	@echo "$(COLOR)▶ Linting del codice...$(RESET)"
	$(PYTHON) -m flake8 scripts

test:
	@echo "$(COLOR)▶ Esecuzione test...$(RESET)"
	$(PYTHON) scripts/test_backend.py

cleandb:
	@echo "$(COLOR)🧹 Pulizia database...$(RESET)"
	rm -rf sensordata.db

clean:
	@echo "$(COLOR)🧹 Pulizia file temporanei...$(RESET)"
	find . -type d -name "__pycache__" -exec rm -r {} +
	find . -type f -name "*.pyc" -delete
	rm -rf .pytest_cache
