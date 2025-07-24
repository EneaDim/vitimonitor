# 💡 Zephyr Feasibility Project

## 📘 Descrizione

Questo progetto ha lo scopo di verificare la **fattibilità di una simulazione ad alto livello** utilizzando **Zephyr RTOS**. L’intero comportamento del sistema — inclusi la simulazione del sensore e il controllo di un LED — è implementato nel codice applicativo (`main.c`), senza definire driver personalizzati.

È un esempio minimalista ed educativo utile per:
- Valutare il funzionamento di Zephyr su diverse piattaforme.
- Simulare il comportamento di periferiche in modo controllato.
- Analizzare la portabilità tra ambienti `native_sim` e `esp32s3`.

---

## 📂 Struttura del Progetto
zephyr-feasibility/
├── CMakeLists.txt # Configurazione CMake per la build
├── Makefile # Comandi abbreviati per build e flash
├── README.md # Descrizione del progetto (questo file)
├── boards/ # Overlay Devicetree per le board supportate
│   ├── esp32s3_devkitc.overlay
│   └── native_sim.overlay
├── prj.conf # Configurazione Zephyr RTOS (log, thread, ecc.)
├── src/
    └── main.c # Applicazione principale con logica emulata


---

## ⚙️ Caratteristiche Principali

- **Simulazione LED**
- Il LED è gestito via Devicetree (`led0`) e controllato ciclicamente da un thread.
- Per `native_sim`, viene usato un controller GPIO emulato.

- **Simulazione sensore ambientale**
- La lettura di temperatura e umidità è simulata all’interno del codice.
- I valori variano in modo pseudo-casuale per test di logging e visualizzazione.

- **Logging**
- I valori simulati vengono stampati a intervalli regolari tramite il sistema di log di Zephyr.
- Supporta stampa in virgola mobile (se abilitata da `CONFIG_CBPRINTF_FP_SUPPORT`).

- **Thread separati**
- Due thread distinti: uno per il LED e uno per la generazione e stampa dei dati ambientali.

- **Compatibilità multipiattaforma**
- Testato su `native_sim` per sviluppo e debug rapido.
- Supporto preliminare per `esp32s3_devkitc` (con setup documentato in `docs/`).

---

## 📑 Documentazione

- `docs/setup_zephyr.md`: guida all'installazione di Zephyr SDK e toolchain.
- `docs/esp32_setup.md`: guida alla configurazione e compilazione per ESP32-S3.

---

## 🎯 Obiettivi del progetto

✅ Verificare la capacità di Zephyr di gestire emulazione ad alto livello
✅ Eseguire logging multithread e controllo di periferiche simulate
✅ Testare overlay Devicetree semplificati per `native_sim` ed ESP32
✅ Fornire un template base per progetti futuri più complessi

---

## ⚖️ Licenza

Distribuito sotto licenza **MIT**.
Consulta il file `LICENSE` (non incluso in questa versione) per i dettagli.

---

## ✍️ Autore

Prototipo sviluppato da [Enea Dimatteo](https://github.com/eneadim)
per l’infrastruttura **AgriTrust** (sperimentazione sensori e IoT agricolo).


