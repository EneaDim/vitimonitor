# 🍇 **Vitimonitor** - Nodo Agricolo Sicuro per Viticoltura

**Vitimonitor** è un sistema integrato progettato per il monitoraggio intelligente e sicuro delle coltivazioni vitivinicole. Combinando hardware avanzato, software sicuro e soluzioni di connettività, il sistema fornisce un monitoraggio in tempo reale dei parametri vitali del vigneto, garantendo la sicurezza dei dati e ottimizzando la gestione agricola.

## 🌱 Obiettivo del Progetto

Il progetto **Vitimonitor** si propone di costruire un sistema che combina sensori avanzati, sicurezza hardware e soluzioni di comunicazione affidabili per il monitoraggio della coltivazione dell'uva. Il sistema si concentra su:

- **Raccolta dati sensoriali locali**: Umidità, temperatura, luce, GPS, nutrienti, e altro ancora.
- **Sicurezza dei dati**: Firma digitale dei dati con identità hardware basata su OpenTitan, per garantire integrità e sicurezza.
- **Aggiornamenti firmware sicuri**: Possibilità di aggiornamenti over-the-air (OTA), verificabili tramite OpenTitan.
- **Comunicazione affidabile**: Comunicazione tramite LoRa, WiFi o LTE, con crittografia e firma dei dati.
- **Analisi e dashboard**: Analisi avanzate e visualizzazione dei dati per supportare decisioni agricole ottimizzate.

## 🔧 Struttura del Progetto

Il progetto **Vitimonitor** è composto da diverse componenti hardware e software, integrate in un'architettura solida e scalabile.

### Componenti principali:

- **Hardware sensori (PCB)**: Design e sviluppo dei PCB per sensori di umidità, temperatura, luce, GPS, e altri parametri, con focus su modularità e affidabilità.
  
- **Firmware Zephyr RTOS**: Il firmware per il nodo sensore è basato su **Zephyr**, un RTOS open source, che permette la gestione dei sensori, la comunicazione sicura e l'implementazione di logiche di monitoraggio avanzate.

- **Sicurezza hardware**: Utilizzo di **OpenTitan** per garantire l'autenticità e l'integrità dei dati. OpenTitan fornisce la base sicura per il boot del sistema e la gestione delle chiavi crittografiche.

- **Backend e frontend**: Soluzione di backend con **FastAPI** per la gestione dei dati e un'interfaccia frontend costruita con **Streamlit** per visualizzare i dati in tempo reale, con dashboard personalizzate per manager, enologi e operatori.

- **Comunicazione multi-tecnologia**: Supporto per LoRa, WiFi e LTE per garantire connettività anche in ambienti remoti, con possibilità di personalizzare le modalità di comunicazione a seconda delle necessità del vigneto.

## 🧩 Funzionalità del Sistema

- **Monitoraggio Ambientale**: Rilevamento continuo di parametri come temperatura, umidità del suolo e dell'aria, luminosità, e posizione GPS.
  
- **Sicurezza e Integrità dei Dati**: I dati raccolti vengono firmati digitalmente tramite OpenTitan, garantendo che le informazioni siano sempre verificate e non alterabili.
  
- **Aggiornamenti Firmware OTA**: Possibilità di aggiornare il firmware del nodo in modo sicuro via OTA, con verifica della firma digitale.
  
- **Dashboard Interattiva**: Pannelli di controllo personalizzati per diversi utenti, come manager, enologi e operatori, per analizzare i dati in tempo reale e prendere decisioni informate.

- **Prevenzione Malattie e Ottimizzazione Irrigazione**: Analisi per la prevenzione di malattie fungine come la peronospora e per ottimizzare l'irrigazione, evitando stress idrico e garantendo la salute delle piante.

- **Certificazione di Qualità**: Garanzia della tracciabilità dei dati e conformità alle certificazioni di qualità per garantire la trasparenza e l'affidabilità dei processi.

## 🛠️ Tecnologie Utilizzate

- **Zephyr RTOS**: Sistema operativo real-time per dispositivi embedded, che permette un'elevata modularità e scalabilità, perfetto per applicazioni IoT.
  
- **OpenTitan**: Tecnologia di sicurezza hardware open-source che garantisce l'integrità e la riservatezza dei dati, fondamentale per il nostro sistema sicuro di raccolta e trasmissione dati.

- **Backend FastAPI**: Framework Python per la creazione di API RESTful per la gestione dei dati sensoriali e la comunicazione con il frontend.
  
- **Frontend Streamlit**: Framework per la creazione di dashboard interattive in tempo reale, per una visualizzazione semplice e immediata dei dati raccolti.

- **LoRa, WiFi, LTE**: Diversi moduli di comunicazione per garantire la connettività a lungo raggio e la trasmissione sicura dei dati.

## 🎯 Requisiti di Sistema

### Requisiti Funzionali

- Rilevamento continuo di dati ambientali (umidità, temperatura, luminosità, GPS).
- Firma digitale dei dati con OpenTitan.
- Comunicazione sicura dei dati tramite LoRa, WiFi, LTE.
- Supporto per aggiornamenti firmware OTA verificabili.
- Analisi dei dati per prevenzione malattie, ottimizzazione irrigazione e monitoraggio della qualità.

### Requisiti Non Funzionali

- Durata della batteria di almeno 48 ore di funzionamento continuo.
- Resilienza alle condizioni ambientali difficili (temperatura da -10°C a +50°C, umidità fino al 90%).
- Latenza di trasmissione dei dati inferiore ai 5 minuti.
- Sistema scalabile per l'integrazione di ulteriori sensori o moduli di comunicazione.

### Requisiti di Sicurezza

- Secure boot basato su OpenTitan.
- Firmatura digitale di tutti i dati trasmessi.
- Protezione hardware avanzata contro tentativi di manomissione.
- Comunicazione cifrata tra il nodo e il backend.

## 🔧 Sviluppo del Sistema

Il progetto è suddiviso in diverse fasi di sviluppo:

1. **Progettazione Hardware e PCB**: Progettazione dei PCB per i sensori e il sistema di comunicazione, con particolare attenzione alla qualità dei componenti e all'affidabilità del sistema.

2. **Sviluppo Firmware Zephyr**: Implementazione del firmware utilizzando Zephyr RTOS, con gestione dei sensori, sicurezza dei dati tramite OpenTitan, e comunicazione con il backend.

3. **Backend e API**: Sviluppo del backend con FastAPI per gestire le richieste API, salvare i dati nel database e gestire la comunicazione con il frontend.

4. **Frontend e Dashboard**: Creazione di dashboard interattive utilizzando Streamlit, personalizzate per diversi ruoli come manager, enologo e operatore.

5. **Test e Verifica**: Test di sicurezza, affidabilità e performance del sistema in ambienti reali, con test su larga scala per garantire che tutte le funzionalità siano operative.

## 💡 Valore Aggiunto

- **Sicurezza Avanzata**: Utilizzo di OpenTitan per proteggere i dati e garantire l'integrità dei processi di raccolta e trasmissione dei dati.
  
- **Analisi Avanzate**: Dashboard dedicate e strumenti di analisi per supportare le decisioni agricole in tempo reale.

- **Integrazione Facile**: Modularità del sistema che permette l'integrazione di nuovi sensori o dispositivi, con scalabilità per future espansioni.

- **Affidabilità e Robustezza**: Sistema progettato per operare in condizioni difficili, con una durata della batteria ottimizzata e capacità di lavorare in ambienti remoti.

## 📚 Documentazione

Tutta la documentazione di progetto, inclusa la descrizione tecnica, il design dei PCB, e le specifiche dei firmware, è disponibile nelle cartelle corrispondenti del repository. Segui le istruzioni nel **README.md** per configurare e avviare il sistema.

---

**Vitimonitor** è la soluzione definitiva per il monitoraggio e la gestione sicura dei vigneti, combinando tecnologia avanzata, sicurezza hardware e un'analisi intelligente dei dati. Con il nostro sistema, i viticoltori possono prendere decisioni più informate, migliorando la qualità del vino e ottimizzando le risorse.

