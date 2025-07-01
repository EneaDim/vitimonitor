import streamlit as st

def tutorial_section():
    st.title("Tutorial Interattivo: Come funziona il sistema vitivinicolo")

    st.markdown("""
    Questo tutorial ti guida passo passo nel funzionamento completo del sistema di monitoraggio:
    dal firmware mock, al backend, fino al frontend e manutenzione.
    """)

    step = st.radio("Seleziona un passo:", [
        "1. Avviare il Backend",
        "2. Avviare il Firmware Mock",
        "3. Avviare il Frontend",
        "4. Testare le API con comandi curl",
        "5. Manutenzione e Qualità del Codice",
        "6. Flusso dati complessivo"
    ])

    if step == "1. Avviare il Backend":
        st.header("1. Avviare il Backend")
        st.write("""
        Il backend FastAPI gestisce la ricezione e l’archiviazione dei dati sensoriali.
        Per avviarlo, usa il comando:
        """)
        if st.button("Mostra comando make run"):
            st.code("make run", language="bash")
        st.write("Assicurati che sia in esecuzione prima di inviare dati o usare il frontend.")

    elif step == "2. Avviare il Firmware Mock":
        st.header("2. Avviare il Firmware Mock")
        st.write("""
        Questo modulo simula i sensori e invia dati al backend.
        Puoi avviarlo con:
        """)
        if st.button("Mostra comando make firmware"):
            st.code("make firmware", language="bash")
        st.write("Usalo per testare il sistema senza hardware reale.")

    elif step == "3. Avviare il Frontend":
        st.header("3. Avviare il Frontend")
        st.write("""
        La dashboard Streamlit ti permette di visualizzare e filtrare i dati raccolti.
        Avviala con:
        """)
        if st.button("Mostra comando make frontend"):
            st.code("make frontend", language="bash")
        st.write("Interagisci con grafici, tabelle e mappe in tempo reale.")

    elif step == "4. Testare le API con comandi curl":
        st.header("4. Testare le API con comandi curl")
        st.write("Puoi verificare lo stato e i dati del backend tramite questi comandi:")
        if st.button("Mostra comandi curl"):
            st.code("""
make curlroot      # Testa l'endpoint root
make curlgetdata  # Ottieni dati sensoriali
make curlstatus   # Verifica lo stato del backend
            """, language="bash")

    elif step == "5. Manutenzione e Qualità del Codice":
        st.header("5. Manutenzione e Qualità del Codice")
        st.write("""
        Mantieni il sistema stabile e pulito con:
        """)
        if st.button("Mostra comandi manutenzione"):
            st.code("""
make lint      # Controlla stile e potenziali errori Python
make test      # Esegui i test automatici
make clean     # Pulisci file temporanei e cache
make cleandb   # Elimina il database SQLite (attenzione, dati persi!)
            """, language="bash")

    elif step == "6. Flusso dati complessivo":
        st.header("6. Flusso dati complessivo")
        st.write("""
        Il flusso dati nel sistema funziona così:

        1. Firmware mock genera dati sensoriali (temperatura, umidità, luminosità, GPS).  
        2. I dati sono inviati al backend tramite HTTP POST o MQTT.  
        3. Il backend FastAPI li riceve, salva nel database e li rende disponibili via API.  
        4. Il frontend Streamlit richiama periodicamente queste API per aggiornare tabelle, grafici e mappe.  
        5. L’utente può interagire, filtrare e analizzare i dati raccolti.

        Usa i comandi `make` per gestire ogni componente.
        """)
        if st.button("Mostra riepilogo comandi"):
            st.code("""
make run         # Avvia backend FastAPI
make firmware    # Avvia firmware mock
make frontend    # Avvia frontend Streamlit
make curlroot    # Test endpoint root
make curlgetdata # Ottenere dati sensori
make curlstatus  # Verifica stato backend
make lint        # Linting codice Python
make test        # Esecuzione test automatici
make clean       # Pulizia file temporanei
make cleandb     # Reset database SQLite
            """, language="bash")

# Per integrare nel tuo script principale, chiama tutorial_section() dove vuoi mostrarlo
if __name__ == "__main__":
    tutorial_section()

