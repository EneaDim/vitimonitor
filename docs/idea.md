# Progetto Nodo Agricolo Sicuro per Viticoltura

## 1. Obiettivo del progetto

L'obiettivo del progetto è la costruzione di un nodo agricolo sicuro, composto da hardware e software, per monitorare la coltivazione dell'uva destinata alla produzione di vino. Il sistema deve:

- Raccogliere dati sensoriali locali (umidità, temperatura, luce, GPS, nutrienti, ecc.).
- Firmare digitalmente i dati tramite un'identità hardware basata su OpenTitan, garantendo integrità e sicurezza.
- Consentire aggiornamenti firmware over-the-air (OTA) verificabili.
- Comunicare con un backend tramite LoRa, WiFi o LTE.
- Offrire analisi dedicate e dashboard per supportare la gestione della viticoltura.

## 2. Problemi risolti dal sistema

- Prevenzione di malattie fungine (peronospora) grazie al monitoraggio microclimatico e dell'umidità fogliare.
- Ottimizzazione dell'irrigazione per evitare stress idrico o eccesso d'acqua.
- Supporto alla maturazione e raccolta con dati fenolici e clorofilliani.
- Garanzia di integrità, tracciabilità e sicurezza dei dati per certificazioni di qualità.
- Facilitazione degli interventi da remoto e aggiornamenti del sistema.

## 3. Competitor principali e i loro punti di forza

| Competitor              | Offerta principale                       | Punti di forza                 | Limiti                                 |
|-------------------------|------------------------------------------|---------------------------------|----------------------------------------|
| **Arable Labs Mark 3**   | Sensori multiparametro, LTE, lunga autonomia | Ampia raccolta dati             | Manca sicurezza hardware avanzata      |
| **CropX Pro Sensor**     | Precisione suolo, analisi cloud          | Precisione suolo, analisi cloud | Costi abbonamento, sicurezza firmware limitata |
| **Teralytic**            | Sensori nutrienti NPK, reporting personalizzato | Nutrienti avanzati, reporting   | Prezzo elevato, sicurezza base         |

## 4. Valore aggiunto del sistema

- Sicurezza hardware avanzata (OpenTitan, firma digitale, OTA verificabili).
- Connettività multi-tecnologia (LoRa, WiFi, LTE).
- Dashboard dedicate e analisi specifiche per la viticoltura.
- Modularità sensoriale e scalabilità del sistema.
- Prezzo competitivo con margini interessanti su hardware + SaaS.

## 5. Sensoristica dedicata per viticoltura

### Sensori base fondamentali

- Umidità e temperatura del suolo.
- pH e conducibilità elettrica (EC) del suolo.
- Temperatura e umidità dell’aria.
- Velocità e direzione del vento.
- Precipitazioni.
- Radiazione luminosa (PAR) e UV.
- GPS.

### Sensori avanzati specialistici

- Sensori radiometrici e fluorometrici per vigore vegetativo e maturazione.
- Spettrofotometri per analisi chimiche (fenoli, zuccheri).
- Sensori geoelettrici e spettroradiometrici per terroir e suolo.
- Sensori nutrienti NPK.
- Sensori di umidità fogliare per prevenzione malattie.
- Sensori CO2 atmosferica e pressione barometrica.

## 6. Ulteriori valori aggiunti da integrare

- Blockchain privata per immutabilità e certificazioni dati.
- Intelligenza artificiale e machine learning a bordo per analisi in tempo reale.
- Integrazione con droni e robot agricoli per monitoraggio e interventi automatizzati.
- Modularità e facilità di espansione plug-and-play.
- Gestione energetica autonoma con energie rinnovabili.
- Dashboard con supporto decisionale integrato e analisi economiche.
- Supporto a certificazioni DOC, biologiche, sostenibilità.
- Monitoraggio ambientale e biodiversità.

## 7. Computing consigliato per l’analisi dati

- Architettura ibrida:
  - Edge computing sul nodo per analisi immediate e sicurezza.
  - Fog computing su gateway locale per aggregazione e analisi più complesse.
  - Cloud computing per analisi storiche, AI/ML avanzata, report e integrazione ERP.
- Questo bilancia latenza, costi, sicurezza e scalabilità.

## 8. Struttura del progetto

```
/agri-trust-node
│
├── fpga/                         # Design HDL FPGA (OpenTitan SoC + logica custom)
│   ├── src/                      # Codice sorgente VHDL/Verilog
│   ├── tb/                       # Testbench per simulazioni
│   ├── constraints/              # File di constraint (pinout, timing)
│   ├── scripts/                  # Script di build, sintesi, implementazione
│   └── README.md                 # Note specifiche FPGA
│
├── mcu_firmware/                 # Firmware MCU per sensori e comunicazione
│   ├── src/                      # Codice sorgente C/C++ o altro linguaggio embedded
│   ├── include/                  # Header files
│   ├── drivers/                  # Driver hardware (sensori, moduli comunicazione)
│   ├── tests/                    # Test funzionali e unitari
│   ├── docs/                     # Documentazione firmware
│   └── README.md                 # Note specifiche firmware MCU
│
├── hardware/                     # Design hardware elettronico e PCB
│   ├── schematics/               # Schemi elettrici (.sch, .kicad_sch, .pdf)
│   ├── pcb_layout/               # Layout PCB, file Gerber, drill files
│   ├── bom/                      # Bill of Materials (componenti, codici, fornitori)
│   ├── fab/                      # File per fabbricazione PCB (gerber compressi, pick & place)
│   ├── assembly/                 # Istruzioni assemblaggio, file SMT pick & place
│   ├── docs/                     # Datasheet componenti, note di progettazione, test
│   └── README.md                 # Indicazioni e guida hardware PCB
│
├── backend/                      # Backend server e servizi cloud
│   ├── api/                      # API REST/MQTT per comunicazione con nodi
│   ├── database/                 # Schema e script DB
│   ├── processing/               # Elaborazione dati, AI/ML, dashboard backend
│   ├── tests/                    # Test backend
│   ├── docs/                     # Documentazione backend
│   └── README.md
│
├── frontend/                     # Interfaccia web e app mobile
│   ├── src/                      # Codice sorgente frontend (React, Vue, Flutter, ecc.)
│   ├── assets/                   # Immagini, icone, stili CSS
│   ├── tests/                    # Test interfaccia utente
│   ├── docs/                     # Manuali utente, guide
│   └── README.md
│
├── docs/                         # Documentazione generale progetto, specifiche, roadmap
│
├── tools/                        # Utility, script di automazione e toolchain personalizzate
│
├── third_party/                  # Codice, librerie e firmware di terze parti
│
└── README.md                     # Panoramica generale progetto, setup iniziale, risorse utili
```


