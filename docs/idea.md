# 💡 Vitimonitor: Sistema Integrato per la Viticoltura di Precisione

**Vitimonitor** è una piattaforma hardware e software progettata per supportare le aziende vitivinicole nel monitoraggio ambientale e nella gestione agronomica di precisione.
Il sistema consente di raccogliere, analizzare e visualizzare dati in tempo reale dal vigneto, permettendo decisioni basate su dati oggettivi e predittivi.

---

## 🌱 Obiettivi del Progetto

- **Monitorare in tempo reale il vigneto** con sensori ambientali (suolo, aria, luce).
- **Fornire supporto decisionale** ad agronomi, enologi e tecnici.
- **Ridurre i rischi agronomici** (malattie, stress idrico, maturazione squilibrata).
- **Ottimizzare risorse e interventi** (irrigazione, trattamenti, vendemmia).
- **Supportare la tracciabilità e la sostenibilità** della produzione.

---

## 🔧 Architettura del Sistema

### 1. **Hardware**

- **Nodi sensore** basati su ESP32 + moduli LoRaWAN
- Sensori ambientali:
- Temperatura e umidità dell’aria
- Umidità del suolo (profilo multilivello: 10/30/60 cm)
- Luminosità (lux + PAR opzionale)
- Sensore Leaf Wetness (opzionale)
- Microsonde termiche per grappolo (opzionale)
- **Alimentazione** tramite pannello solare e batteria ricaricabile
- **Gateway LoRaWAN**:
- Scheda custom o gateway commerciale (es. The Things Network)
- Connessione Internet: LTE / Ethernet / Wi-Fi

---

### 2. **Software**

#### Backend

- Acquisizione e normalizzazione dati
- Database time-series (InfluxDB, TimescaleDB)
- API REST/GraphQL per i client
- Algoritmi agronomici e modelli predittivi (GDD, ET0, patologie)

#### Frontend Web

- Web app per PC (enologo, manager, agronomo)
- Dashboard con:
- Mappe georeferenziate
- Grafici storici e in tempo reale
- Allarmi e notifiche
- Gestione microzone

#### App Mobile

- App per operatori in campo
- Visualizzazione rapida dei sensori vicini
- Inserimento note, foto, osservazioni
- Ricezione di alert mirati

---

## 💡 Funzionalità Smart (sviluppi futuri)

- Modelli predittivi per rischio peronospora, oidio, botrite
- Previsione fenologica e data ottimale di vendemmia
- AI per analisi visiva dei grappoli (foto da operatori o droni)
- Integrazione con droni/satelliti per mappatura NDVI/NDRE
- Tracciabilità blockchain-ready (dalla vendemmia alla bottiglia)

---

## 📘 Target Utente

| Ruolo| Funzionalità Principali |
|--------------------|---------------------------------------------------------|
| **Manager aziendale** | Analisi delle performance, confronto tra annate|
| **Enologo** | Pianificazione vendemmia, analisi maturazione uva|
| **Agronomo**| Monitoraggio fenologia, rischio malattie, irrigazione|
| **Tecnico di campo** | Verifica sensori, gestione alert, note e osservazioni|

---

## 🎯 Valore Aggiunto

- Decisioni basate su dati oggettivi, non solo esperienza
- Maggiore qualità dell’uva → miglior vino
- Riduzione dei costi e trattamenti non necessari
- Innovazione concreta, scalabile e sostenibile

---

## 🚀 Prossimi Step

1. Prototipazione della scheda nodo sensore
2. Scelta e integrazione del gateway
3. MVP Web + App mobile
4. Validazione in campo con aziende pilota
5. Estensione dei moduli agronomici e AI


