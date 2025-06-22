# Feasibility Study: Sistema di Acquisizione e Gestione Dati Sensori

## Obiettivo  
Verificare la fattibilità di un sistema embedded/software che acquisisce dati da sensori simulati, li trasmette tramite firmware mock, e li memorizza/gestisce in un backend FastAPI con database SQLite.

---

## Componenti Principali e Flusso Dati

1. **Sensor Mock (sensori simulati)**  
   - Modulo Python che genera dati sensori casuali o fissi (temperatura, umidità, luminosità, GPS).  
   - Fornisce anche una firma digitale “dummy” per autenticare i dati.

2. **Firmware Mock**  
   - Simula il firmware MCU che ogni tot secondi legge i dati dal sensore mock.  
   - Prepara il payload JSON con i dati sensori + firma.  
   - Invia i dati tramite HTTP POST direttamente all’endpoint `/data` del backend (alternativa MQTT rimandata o opzionale).

3. **Backend FastAPI**  
   - Espone API REST per ricevere dati sensori (`POST /data`), restituire dati memorizzati (`GET /data`), e fornire stato sistema (`GET /status`).  
   - Verifica la firma digitale (dummy) per autenticare i dati in ingresso.  
   - Memorizza i dati in SQLite con timestamp.

4. **Makefile**  
   - Automatizza il lancio del backend e l’esecuzione di richieste curl di prova.  
   - Aiuta a testare rapidamente il sistema e verificarne il funzionamento.

---

## Flusso Esempio

- Ogni 5 secondi, il firmware mock genera un nuovo set di dati sensori.  
- Questi dati vengono firmati e inviati via HTTP POST al backend.  
- Il backend verifica la firma, salva i dati su DB e risponde con conferma.  
- Puoi interrogare il backend per leggere i dati memorizzati o controllare lo stato (numero di record) tramite chiamate GET.  
- Tutto il sistema è costruito con moduli facilmente sostituibili (mock → firmware reale → IP FPGA) e tecnologie standard (FastAPI, SQLite, Python).

---

## Prossimi Passi

- Integrare MQTT se serve la comunicazione più realistica.  
- Sviluppare la parte FPGA/IP OpenTitan per sostituire il mock hardware.  
- Espandere il backend con autenticazione reale e dashboard frontend.

---
# Miglioramenti suggeriti per la feasibility

## 1. Firma digitale reale e sicurezza
- Implementare una firma digitale autentica (es. con chiavi asimmetriche) per garantire integrità e autenticità dei dati.
- Proteggere le API con autenticazione (token JWT, OAuth2) per evitare accessi non autorizzati.

## 2. Gestione errori e logging
- Migliorare la gestione degli errori nel backend, con messaggi chiari e log strutturati.
- Aggiungere logging centralizzato per monitorare richieste, errori e performance.

## 3. Database e scalabilità
- Valutare un database più robusto (PostgreSQL, TimescaleDB) rispetto a SQLite per ambienti di produzione e grandi volumi di dati.
- Implementare paginazione e filtri nelle API di lettura per gestire grandi quantità di dati.

## 4. Test automatici
- Scrivere test unitari e di integrazione per backend, mock e firmware per garantire stabilità con CI/CD.
- Simulare casi di errore, dati invalidi e sovraccarico.

## 5. MQTT e comunicazione
- Usare MQTT per la comunicazione firmware → backend per un sistema più reale e reattivo.
- Gestire la connessione MQTT in modo resiliente con riconnessioni automatiche.

## 6. Dashboard e visualizzazione
- Integrare un frontend web/dashboard per monitorare in tempo reale i dati dei sensori.
- Aggiungere alert o segnalazioni automatiche su anomalie (es. valori fuori soglia).

## 7. Configurabilità
- Rendere configurabili gli intervalli di lettura, endpoint e topic MQTT tramite file di configurazione o variabili d’ambiente.

---

Se vuoi posso aiutarti a implementare uno o più di questi miglioramenti! Vuoi partire da qualcosa in particolare?

