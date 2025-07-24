# ¿ Vitimonitor: Sistema Integrato per la Viticoltura di Precisione

Vitimonitor è una piattaforma hardware e software progettata per supportare aziende vitivinicole nel monitoraggio ambientale e nella gestione agronomica di precisione.
Il sistema consente di raccogliere, analizzare e visualizzare dati in tempo reale dal vigneto per prendere decisioni basate su dati oggettivi e predittivi.

---

## ¿ Obiettivi del Progetto

- **Monitorare in tempo reale il vigneto** con sensori ambientali (suolo, aria, luce).
- **Fornire supporto decisionale** ad agronomi, enologi e tecnici.
- **Ridurre i rischi agronomici** (malattie, stress idrico, maturazione squilibrata).
- **Ottimizzare risorse e interventi** (irrigazione, trattamenti, vendemmia).
- **Supportare la tracciabilità e sostenibilità** nella produzione.

---

## ¿ Architettura del Sistema

### 1. **Hardware**
- **Nodi sensore** basati su ESP32 + moduli LoRaWAN
- Sensori ambientali:
- Temperatura e umidità dell¿aria
- Umidità del suolo (profilo multilivello: 10/30/60 cm)
- Luminosità (lux + PAR opzionale)
- Leaf Wetness Sensor (opzionale)
- Microsonde termiche per grappolo (opzionale)
- **Alimentazione** a pannello solare + batteria
- **Gateway LoRaWAN**:
- Scheda custom o gateway commerciale (The Things Network)
- Connessione Internet: LTE / Ethernet / Wi-Fi

---

### 2. **Software**

#### Backend
- Acquisizione e normalizzazione dati
- Database time-series (InfluxDB, TimescaleDB)
- API REST/GraphQL per i client
- Algoritmi agronomici e modelli predittivi (es. GDD, ET0, fitopatologie)

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
- Ricezione alert mirati

---

## ¿ Funzionalità Smart (fasi successive)

- Modelli predittivi per rischio peronospora, oidio, botrite
- Previsione fenologica e data ottimale di vendemmia
- AI per analisi visiva di grappoli (foto da operatore o droni)
- Integrazione con droni/satelliti per mappa NDVI/NDRE
- Tracciabilità blockchain-ready (vendemmia ¿ bottiglia)

---

## ¿ Target Utente

| Ruolo | Funzionalità Principali |
|-------------------|-------------------------------------------------|
| **Manager aziendale** | Analisi performance, confronto tra annate |
| **Enologo**| Pianificazione vendemmia, maturazione uva |
| **Agronomo** | Analisi fenologia, rischio malattie, irrigazione|
| **Tecnico di campo** | Verifica sensori, note, alert, gestione zona |

---

## ¿ Valore Aggiunto

- Decisioni basate su dati, non solo esperienza
- Maggiore qualità dell¿uva ¿ miglior vino
- Riduzione costi e trattamenti inutili
- Innovazione concreta, scalabile, sostenibile

---

## ¿ Prossimi Step

1. Prototipo scheda nodo sensore
2. Scelta/integrazione gateway
3. MVP Web + App mobile
4. Validazione con aziende test
5. Estensione moduli agronomici/AI

