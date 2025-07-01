# Riassunto Progetto Zephyr con QEMU, Zbus e MQTT

## Discussione

1. **Obiettivo iniziale**: emulare un firmware su QEMU che gira Zephyr, con sensori multipli (UART, SPI, I2C, ADC) e connessione remota via MQTT.
2. **Strategia**:
   - Uso di QEMU per emulare l’hardware e Zephyr come OS.
   - Emulazione sensori tramite thread mock che pubblicano dati su Zbus (message bus interno).
   - Un altro thread sottoscrive dati dal bus e li invia via MQTT verso un broker.
3. **Codice fornito**: programma in C per Zephyr che implementa
   - canale Zbus per dati sensori,
   - thread di produzione dati mock,
   - thread di pubblicazione MQTT,
   - gestione MQTT e connessione.
4. **Configurazioni**:
   - `prj.conf` con le feature di rete, MQTT e Zbus abilitate
   - `CMakeLists.txt` per build
   - overlay device per QEMU RISC-V e Cortex-M (override console)
5. **Funzionamento generale**:
   - Il thread mock produce dati simulati di sensori e li pubblica sul canale Zbus
   - Il thread MQTT si connette a un broker remoto e ascolta i dati dal canale
   - Appena riceve dati, li invia come payload JSON sul topic MQTT
6. **Estensioni possibili**:
   - Aggiungere driver reali per UART, SPI, I2C, ADC per leggere sensori reali o simulati
   - Espandere il canale Zbus con più tipi di dati per più sensori
   - Integrare sicurezza TLS per MQTT
   - Fare il porting su board reale usando gli stessi driver e codice applicativo

---

Questo progetto permette di simulare un sistema embedded con sensori multipli e connessione cloud tramite MQTT, usando Zephyr e QEMU come ambiente di sviluppo e test.

