# Documento Requisiti di Sistema (SRS) – AgriTrust Node

## 1. Introduzione

**AgriTrust Node** è un sistema embedded progettato per l’agricoltura di precisione, con particolare attenzione alla sicurezza e affidabilità. Integra il Root of Trust OpenTitan per garantire autenticità, integrità e riservatezza dei dati raccolti da sensori ambientali, e supporta comunicazioni sicure verso backend remoti per analisi e tracciabilità.

## 2. Scopo

Fornire agli operatori agricoli, startup agri-tech e cooperative un nodo IoT sicuro che permetta monitoraggio ambientale, aggiornamenti remoti e auditing affidabile dei dati, proteggendo il sistema da attacchi informatici e manomissioni.

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

### 3.4 Requisiti di interfaccia

| Codice | Descrizione |
|--------|-------------|
| **RI1** | Il sistema deve fornire un’interfaccia web responsive per configurazione, monitoraggio e diagnostica, accessibile tramite autenticazione sicura. |
| **RI2** | Deve offrire API RESTful e/o MQTT per l’invio e la ricezione di dati tra nodo e backend. |
| **RI3** | Deve supportare protocolli di comunicazione LoRaWAN, WiFi 802.11 b/g/n e LTE Cat-M1 o NB-IoT, configurabili via software. |
| **RI4** | Deve integrare notifiche push o e-mail configurabili per segnalazioni di eventi critici o anomalie. |

## 4. Vincoli

- Utilizzo obbligatorio di OpenTitan come Root of Trust per sicurezza hardware.
- Supporto iniziale per sensori standard di umidità, temperatura, luminosità e GPS.
- Alimentazione primaria a batteria, con possibilità di estensione tramite pannello solare.
- Compatibilità con backend cloud standard (es. AWS IoT, Azure IoT Hub) tramite API.

## 5. Glossario

- **OpenTitan:** progetto open source per Root of Trust hardware sicuro.
- **Root of Trust:** componente hardware/software affidabile che garantisce sicurezza delle operazioni crittografiche.
- **FOTA:** Firmware Over The Air, aggiornamenti firmware da remoto.
- **Tamper detection:** rilevamento di manomissioni fisiche o tentativi di compromissione hardware.
- **API:** Application Programming Interface, interfaccia per comunicare tra software.

