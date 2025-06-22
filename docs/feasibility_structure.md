project-root/
│
├── firmware/
│   ├── zephyr-app/                   # Codice firmware Zephyr MCU
│   │   ├── src/
│   │   ├── include/
│   │   ├── prj.conf                  # Config Zephyr
│   │   └── CMakeLists.txt
│   └── mocks/
│       └── sensor_simulation.c      # Moduli mock per simulare sensori in assenza FPGA
│
├── backend/
│   ├── app/
│   │   ├── main.py                   # Entry point API backend
│   │   ├── mqtt_client.py            # Client MQTT o socket
│   │   ├── signature_verifier.py    # Modulo verifica firme digitali (mock o reale)
│   │   └── database.py               # DB layer (SQLite)
│   ├── requirements.txt              # Dipendenze Python
│   └── tests/
│
├── dashboard/
│   ├── public/
│   ├── src/
│   ├── package.json
│   └── README.md
│
└── docs/
    ├── architecture.md
    ├── setup_guide.md
    └── test_plan.md

