# Riassunto progetto Nodo Agricolo Sicuro per Viticoltura

---

## 1. Obiettivo del progetto

- Costruire un nodo agricolo sicuro, hardware + software, per monitorare la coltivazione dell’uva destinata alla produzione di vino.
- Il nodo deve:
- Raccogliere dati sensoriali locali (umidità, temperatura, luce, GPS, nutrienti, ecc.)
- Firmare digitalmente i dati tramite un’identità hardware basata su OpenTitan, garantendo integrità e sicurezza
- Consentire aggiornamenti firmware over-the-air (OTA) verificabili
- Comunicare con backend tramite LoRa, WiFi o LTE
- Offrire analisi dedicate e dashboard per supportare la gestione della vite

---

## 2. Problemi vitivinicoli che il sistema aiuta a risolvere

- Prevenzione di malattie fungine (peronospora) tramite monitoraggio microclimatico e umidità fogliare
- Ottimizzazione dell’irrigazione per evitare stress idrico o eccesso d’acqua
- Supporto alla maturazione e raccolta con dati fenolici e clorofilliani
- Garanzia di integrità, tracciabilità e sicurezza dei dati per certificazioni di qualità
- Facilitazione di interventi da remoto e aggiornamenti del sistema

---

## 3. Competitor principali e i loro punti di forza

| Competitor | Offerta principale| Punti di forza | Limiti|
|---------------------|---------------------------------------------|----------------------------------|-------------------------------------|
| **Arable Labs Mark 3** | Sensori multiparametro, LTE, lunga autonomia | Ampia raccolta dati| Manca sicurezza hardware avanzata |
| **CropX Pro Sensor**| Precisione suolo, analisi cloud| Precisione suolo, analisi cloud | Costi abbonamento, sicurezza firmware limitata |
| **Teralytic** | Sensori nutrienti NPK, reporting personalizzato | Nutrienti avanzati, reporting| Prezzo elevato, sicurezza base|

---

## 4. Valore aggiunto del tuo sistema

- Sicurezza hardware avanzata (OpenTitan, firma digitale, OTA verificabili)
- Connettività multi-tecnologia (LoRa, WiFi, LTE)
- Dashboard dedicate e analisi specifiche per la viticoltura
- Modularità sensoriale e scalabilità del sistema
- Prezzo competitivo con margini interessanti su hardware + SaaS

---

## 5. Sensoristica dedicata per viticoltura

### Sensori base fondamentali

- Umidità e temperatura del suolo
- pH e conducibilità elettrica (EC) del suolo
- Temperatura e umidità dell’aria
- Velocità e direzione del vento
- Precipitazioni
- Radiazione luminosa (PAR) e UV
- GPS

### Sensori avanzati specialistici

- Sensori radiometrici e fluorometrici per vigore vegetativo e maturazione
- Spettrofotometri per analisi chimiche (fenoli, zuccheri)
- Sensori geoelettrici e spettroradiometrici per terroir e suolo
- Sensori nutrienti NPK
- Sensori di umidità fogliare per prevenzione malattie
- Sensori CO2 atmosferica e pressione barometrica

---

## 6. Ulteriori valori aggiunti da integrare

- Blockchain privata per immutabilità e certificazioni dati
- Intelligenza artificiale e machine learning a bordo per analisi in tempo reale
- Integrazione con droni e robot agricoli per monitoraggio e interventi automatizzati
- Modularità e facilità di espansione plug-and-play
- Gestione energetica autonoma con energie rinnovabili
- Dashboard con supporto decisionale integrato e analisi economiche
- Supporto a certificazioni DOC, biologiche, sostenibilità
- Monitoraggio ambientale e biodiversità

---

## 7. Computing consigliato per l’analisi dati

- Architettura ibrida:
- Edge computing sul nodo per analisi immediate e sicurezza
- Fog computing su gateway locale per aggregazione e analisi più complesse
- Cloud computing per analisi storiche, AI/ML avanzata, report e integrazione ERP
- Questo bilancia latenza, costi, sicurezza e scalabilità

---

# Documento Requisiti di Sistema (SRS) – AgriTrust Node

## 1. Introduzione

**AgriTrust Node** è un sistema embedded progettato per l’agricoltura di precisione, con particolare attenzione alla sicurezza e affidabilità. Integra il Root of Trust OpenTitan per garantire autenticità, integrità e riservatezza dei dati raccolti da sensori ambientali, e supporta comunicazioni sicure verso backend remoti per analisi e tracciabilità.

---

## 2. Scopo

Fornire agli operatori agricoli, startup agri-tech e cooperative un nodo IoT sicuro che permetta monitoraggio ambientale, aggiornamenti remoti e auditing affidabile dei dati, proteggendo il sistema da attacchi informatici e manomissioni.

---

## 3. Requisiti di sistema

### 3.1 Requisiti funzionali

| Codice | Descrizione |
|--------|-------------|
| **RF1** | Il sistema deve raccogliere dati da sensori ambientali quali umidità, temperatura, luminosità e posizione GPS con frequenza configurabile (default 5 minuti). |
| **RF2** | Deve eseguire il boot solo se il firmware è verificato e firmato tramite Root of Trust OpenTitan (secure boot). |
| **RF3** | I dati raccolti devono essere firmati digitalmente con chiavi crittografiche gestite esclusivamente da OpenTitan. |
| **RF4** | Il sistema deve inviare i dati firmati a un backend remoto tramite protocolli di rete selezionabili: LoRaWAN, WiFi, LTE. |
| **RF5** | Deve supportare aggiornamenti firmware over-the-air (FOTA), con verifica della firma digitale da parte di OpenTitan prima dell’installazione. |
| **RF6** | Deve mantenere un registro locale sicuro, immutabile e crittografato degli eventi di sistema e delle trasmissioni dati (logging). |
| **RF7** | Deve permettere la configurazione remota sicura di parametri di raccolta dati, frequenza e protocolli di rete. |
| **RF8** | Il sistema deve rilevare e segnalare anomalie hardware/software via messaggi firmati al backend. |

---

### 3.2 Requisiti non funzionali

| Codice | Descrizione |
|--------|-------------|
| **RNF1** | La durata della batteria integrata deve garantire almeno 48 ore di funzionamento continuo senza ricarica. |
| **RNF2** | Il sistema deve operare correttamente in un intervallo di temperatura ambientale compreso tra -10°C e +50°C, con umidità fino al 90% senza condensa. |
| **RNF3** | Il tempo massimo di latenza tra la lettura dei dati sensoriali e la trasmissione al backend non deve superare i 5 minuti. |
| **RNF4** | La connettività di rete deve essere resiliente a interruzioni temporanee e garantire il ripristino automatico della comunicazione. |
| **RNF5** | L’interfaccia utente (web o app mobile) deve essere intuitiva e accessibile con livelli di autorizzazione differenziati (amministratore, operatore, viewer). |
| **RNF6** | Il sistema deve essere scalabile per integrare ulteriori sensori o moduli di comunicazione senza modifiche sostanziali al firmware. |
| **RNF7** | Il sistema deve essere progettato per un’installazione rapida sul campo da personale con competenze tecniche base. |

---

### 3.3 Requisiti di sicurezza

| Codice | Descrizione |
|--------|-------------|
| **RS1** | Il boot deve essere protetto da un meccanismo di secure boot basato su OpenTitan, che verifica l’integrità e autenticità del firmware. |
| **RS2** | Tutti i dati trasmessi devono essere firmati digitalmente utilizzando chiavi private protette da OpenTitan, garantendo autenticità e integrità. |
| **RS3** | Deve essere implementato un meccanismo di rilevamento tentativi di manomissione hardware (tamper detection), con allarme e blocco sicuro in caso di violazione. |
| **RS4** | Le chiavi crittografiche devono essere generate e conservate esclusivamente nel dominio sicuro di OpenTitan, senza mai uscire dall’hardware. |
| **RS5** | Il sistema deve effettuare il logging immutabile degli eventi di sistema e delle comunicazioni, con firme digitali che ne garantiscano la non alterabilità. |
| **RS6** | Gli aggiornamenti firmware devono essere firmati e verificati prima dell’applicazione, per prevenire installazioni di software malevoli. |
| **RS7** | La comunicazione tra nodo e backend deve essere cifrata con protocolli standard (es. TLS, LoRaWAN encryption). |
| **RS8** | Deve essere garantito un processo di gestione sicura delle chiavi, inclusa la possibilità di rotazione periodica. |

---

### 3.4 Requisiti di interfaccia

| Codice | Descrizione |
|--------|-------------|
| **RI1** | Il sistema deve fornire un’interfaccia web responsive per configurazione, monitoraggio e diagnostica, accessibile tramite autenticazione sicura. |
| **RI2** | Deve offrire API RESTful e/o MQTT per l’invio e la ricezione di dati tra nodo e backend. |
| **RI3** | Deve supportare protocolli di comunicazione LoRaWAN, WiFi 802.11 b/g/n e LTE Cat-M1 o NB-IoT, configurabili via software. |
| **RI4** | Deve integrare notifiche push o e-mail configurabili per segnalazioni di eventi critici o anomalie. |

---

## 4. Vincoli

- Utilizzo obbligatorio di OpenTitan come Root of Trust per sicurezza hardware.
- Supporto iniziale per sensori standard di umidità, temperatura, luminosità e GPS.
- Alimentazione primaria a batteria, con possibilità di estensione tramite pannello solare.
- Compatibilità con backend cloud standard (es. AWS IoT, Azure IoT Hub) tramite API.

---

## 5. Glossario

- **OpenTitan:** progetto open source per Root of Trust hardware sicuro.
- **Root of Trust:** componente hardware/software affidabile che garantisce sicurezza delle operazioni crittografiche.
- **FOTA:** Firmware Over The Air, aggiornamenti firmware da remoto.
- **Tamper detection:** rilevamento di manomissioni fisiche o tentativi di compromissione hardware.
- **API:** Application Programming Interface, interfaccia per comunicare tra software.

---

# Descrizione Architetturale del Sistema AgriTrust Node

---

## 1. Componente Root of Trust (OpenTitan SoC)

- È il cuore della sicurezza hardware, incaricato di garantire il boot sicuro del sistema.
- Gestisce le chiavi crittografiche private, le firme digitali e verifica l’integrità del firmware e degli aggiornamenti.
- È isolato dal resto del sistema per evitare che le chiavi escano dall’ambiente sicuro.
- Coordina il processo di attestazione, firmando i dati prima della loro trasmissione.

---

## 2. Microcontrollore / Host Processor

- Esegue l’applicazione agricola principale, cioè il software che interagisce con i sensori e gestisce la comunicazione.
- È responsabile di raccogliere i dati dai sensori (umidità, temperatura, luminosità, GPS) a intervalli configurabili.
- Chiede a OpenTitan di firmare i dati prima di inviarli al backend.
- Gestisce la rete (LoRaWAN, WiFi, LTE) per la trasmissione dati.
- Implementa la logica per aggiornamenti firmware, configurazione remota, e logging degli eventi.

---

## 3. Sensori Ambientali

- Sono dispositivi collegati al microcontrollore che misurano vari parametri utili all’agricoltura di precisione.
- Tipicamente includono sensori di umidità, temperatura, luminosità e modulo GPS per la posizione geografica.

---

## 4. Modulo Comunicazione

- Il nodo è dotato di uno o più moduli per la comunicazione wireless, scelti in base al contesto:
- LoRaWAN per copertura su grandi distanze e basso consumo.
- WiFi per aree con infrastruttura disponibile.
- LTE (Cat-M1 o NB-IoT) per copertura cellulare dedicata.
- La comunicazione è cifrata e i dati trasmessi sono firmati da OpenTitan per garantirne autenticità e integrità.

---

## 5. Alimentazione

- Il sistema funziona principalmente con batteria, garantendo almeno 48h di autonomia.
- È possibile integrare pannelli solari per estendere la durata operativa in ambienti remoti.

---

## 6. Backend Remoto

- Riceve i dati firmati, li verifica tramite le chiavi pubbliche corrispondenti a OpenTitan.
- Archivia i dati per analisi, tracciabilità e auditing.
- Permette la configurazione remota del nodo (frequenza lettura sensori, parametri di rete, aggiornamenti firmware).

---

## 7. Interfaccia Utente

- Accessibile via web o app mobile, permette la gestione del sistema, monitoraggio in tempo reale e visualizzazione dello stato.
- Offre livelli di accesso differenziati e notifiche di allarme o anomalie.

---

# Flusso dati e controllo (semplificato)

1. All’accensione, OpenTitan esegue secure boot del microcontrollore e verifica firmware.
2. Il microcontrollore legge i sensori e invia i dati a OpenTitan per la firma digitale.
3. I dati firmati vengono trasmessi via modulo di comunicazione al backend.
4. Il nodo registra localmente gli eventi in un log immutabile.
5. Aggiornamenti firmware vengono scaricati, verificati da OpenTitan e installati in sicurezza.
6. L’interfaccia utente consente di configurare parametri e monitorare lo stato del nodo.

