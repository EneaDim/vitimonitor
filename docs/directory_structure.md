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

