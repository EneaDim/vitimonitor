import random
from datetime import datetime, timedelta
import pandas as pd
import streamlit as st
import requests
from fpdf import FPDF
from common import check_thresholds
import time

def render(df, backend_url):
    st.header("👷 Operatore — Gestione Anomalie e Pianificazione Attività")

    # --- Filtra anomalie già risolte ---
    if 'resolved_anomalies' not in st.session_state:
        st.session_state['resolved_anomalies'] = []

    # --- Calcola anomalie ---
    # Genera le anomalie per ogni metrica
    df_anomalie = pd.concat([
        check_thresholds(df, m)
        for m in ['temperature', 'humidity_air', 'humidity_soil', 'luminosity']
    ])

    # --- Filtra solo le anomalie che non sono ancora risolte ---
    df_anomalie_filtrate = df_anomalie[~df_anomalie['sensor_id'].isin(st.session_state['resolved_anomalies'])]

    # Se non ci sono anomalie, mostra un messaggio
    if df_anomalie_filtrate.empty:
        st.write("Non ci sono anomalie da trattare.")
        return

    # Funzione per generare attività in base alle anomalie
    def generate_activity(row):
        activities = []
        if row['temperature'] > 30:
            activities.append(f"Verifica sensori temperatura in zona {row['zone']}")
        if row['humidity_air'] < 30:
            activities.append(f"Irrigazione zona {row['zone']}")
        if row['humidity_soil'] < 20:
            activities.append(f"Controllo umidità suolo zona {row['zone']}")
        if row['luminosity'] > 100000:
            activities.append(f"Verifica luminosità zona {row['zone']}")
        return activities

    # Pianificazione in base alle anomalie
    activities_list = []
    for index, row in df_anomalie_filtrate.iterrows():
        try:
            # Converte la colonna 'timestamp' in un oggetto datetime, se è una stringa
            anomaly_date = pd.to_datetime(row['timestamp']) if 'timestamp' in row else datetime.today()
        except Exception as e:
            st.error(f"Error while converting timestamp for row {index}: {e}")
            continue  # Skip to next row in case of error
        # Genera le attività per questa riga
        activities = generate_activity(row)
        # Pianifica sempre 2 giorni dopo la data dell'anomalia
        scheduled_date = anomaly_date + timedelta(days=2)  # Pianifica esattamente 2 giorni dopo l'anomalia
        #scheduled_date = scheduled_date.date()
        #st.write(f"Scheduled Date for activity: {scheduled_date}")
        for activity in activities:
            activities_list.append({
                'data_anomalia': anomaly_date,  # Data dell'anomalia come prima colonna
                'data': scheduled_date,  # Pianifica sempre 2 giorni dopo la data dell'anomalia
                'attività': activity,
                'priorità': 'Alta',  # Puoi aggiungere logiche per assegnare priorità
                'zona': row['zone'],
                'sensor_id': row.get('sensor_id', 'Unknown'),  # Usa get per evitare KeyError
                'status': 'Da fare'  # Stato dell'attività
            })
    # Crea il DataFrame delle attività
    activities_df = pd.DataFrame(activities_list)
    # Rimuovi duplicati (per lo stesso sensore e stessa data + millisecondi)
    activities_df = activities_df.drop_duplicates(subset=['sensor_id', 'data', 'attività'], keep='first')
    # Ordina la tabella per data (facoltativo)
    activities_df = activities_df[['data_anomalia', 'data', 'attività', 'priorità', 'zona', 'sensor_id', 'status']]  # Mostra data_anomalia come prima colonna
    activities_df = activities_df.sort_values(by='data')  # Ordina le attività per data pianificata
    # Visualizza la data dell'anomalia e pianifica sempre 2 giorni dopo
    activities_df['data_anomalia'] = activities_df['data_anomalia'].dt.strftime('%Y-%m-%d %H:%M:%S')  # Formatta la data per visualizzarla con ore, minuti e secondi
    activities_df['data'] = activities_df['data'].dt.strftime('%Y-%m-%d %H:%M:%S')  # Formatta la data pianificata con ore, minuti e secondi
 
    # --- Pianificazione Attività ---
    st.subheader("📅 Pianificazione Attività")
    activities_df = activities_df.reset_index(drop=True)

    # --- Selezione attività da completare ---
    selected_activity = st.selectbox("Seleziona attività da risolvere", activities_df.index)

    if selected_activity is not None:
        selected_row = activities_df.iloc[selected_activity]

        # Mostra i dettagli dell'attività
        st.write(f"**Attività da Risolvere**:")
        st.write(f"**Data:** {selected_row['data']}")
        st.write(f"**Attività:** {selected_row['attività']}")
        st.write(f"**Zona:** {selected_row['zona']}")
        st.write(f"**Sensore:** {selected_row['sensor_id']}")
        st.write(f"**Priorità:** {selected_row['priorità']}")

        # Conferma completamento attività
        confirm_button = st.button(f"✅ Conferma completamento attività: {selected_row['attività']}")

        if confirm_button:
            # Aggiungi l'ID sensore all'elenco delle anomalie risolte
            st.session_state['resolved_anomalies'].append(selected_row['sensor_id'])
            # Modifica lo stato dell'attività
            activities_df.loc[selected_activity, 'status'] = 'Completata'
            activities_df = activities_df[activities_df['status'] != 'Completata']
            st.success(f"Attività in zona {selected_row['zona']} completata.")
            time.sleep(1)
            # Ricarica automaticamente la pagina per aggiornare la tabella
            st.rerun()

    # Mostra la tabella delle attività
    st.dataframe(activities_df)

    # --- Aggiungi / Modifica Attività ---
    st.subheader("📅 Aggiungi / Modifica Attività")
    if 'attività' in activities_df.columns:
        activity_to_modify = st.selectbox("Seleziona attività da modificare", activities_df['attività'].unique())

        if activity_to_modify:
            activity_row = activities_df[activities_df['attività'] == activity_to_modify].iloc[0]
            new_date = st.date_input("Nuova data", value=activity_row['data'])
            new_priority = st.selectbox("Nuova priorità", ['Alta', 'Media', 'Bassa'], index=['Alta', 'Media', 'Bassa'].index(activity_row['priorità']))

            # Salva la modifica
            if st.button("Aggiorna attività"):
                activities_df.loc[activities_df['attività'] == activity_to_modify, 'data'] = new_date
                activities_df.loc[activities_df['attività'] == activity_to_modify, 'priorità'] = new_priority
                st.session_state['activities_df'] = activities_df
                st.success(f"Attività '{activity_to_modify}' aggiornata con successo!")
    else:
        st.error("Errore: La colonna 'attività' non è presente nei dati delle attività.")

    # --- Indicatore Chiave di Prestazione delle anomalie ---
    st.subheader("📊 Indicatore Chiave di Prestazione Anomalie")
    st.metric("Anomalie rilevate", len(df_anomalie))
    st.metric("Anomalie in zone critiche", len(df_anomalie[df_anomalie['temperature'] > 30]))

    st.markdown("---")

    # --- Sezione Misura Manuale ---
    st.subheader("📋 Inserisci Misura Manuale")

    # Form per inserire misurazioni manuali
    with st.form("manual_form"):
        timestamp = st.date_input("Data", value=datetime.today())
        zona = st.text_input("Zona", value="")
        sensore_id = st.text_input("ID Sensore (opzionale)", value="")
        temperatura = st.number_input("Temperatura (°C)", value=20.0)
        umidita_aria = st.number_input("Umidità Aria (%)", value=50.0)
        umidita_suolo = st.number_input("Umidità Suolo (%)", value=30.0)
        luminosita = st.number_input("Luminosità (lux)", value=1000)

        submitted = st.form_submit_button("Salva Misura")

    if submitted:
        misura = {
            "timestamp": timestamp.isoformat(),
            "zone": zona,
            "sensor_id": sensore_id,
            "temperature": temperatura,
            "humidity_air": umidita_aria,
            "humidity_soil": umidita_suolo,
            "luminosity": luminosita,
            "manual" : True,
            "signature": ""
        }
        response = requests.post(f"{backend_url}/add_manual_measure", json=misura)
        if response.status_code == 200:
            st.success("✅ Misura manuale salvata.")
        else:
            st.error(f"❌ Errore: {response.text}")

    # --- Generazione Report ---
    st.subheader("📑 Genera Report")

    # Input per personalizzare il nome del report
    report_name = st.text_input("Inserisci nome del report", "VitiMonitor_Report")

    # Funzione per generare il report CSV
    def generate_csv_report(df_anomalie, activities_df, completed_activities, report_name):
        with pd.ExcelWriter(f'{report_name}.xlsx') as writer:
            df_anomalie.to_excel(writer, sheet_name='Anomalie', index=False)
            activities_df.to_excel(writer, sheet_name='Attività', index=False)
            
            # Aggiungi la tabella delle attività completate
            completed_df = pd.DataFrame(completed_activities)
            completed_df.to_excel(writer, sheet_name='Attività Completate', index=False)

        st.download_button(
            label="Scarica Report Excel",
            data=open(f'{report_name}.xlsx', 'rb'),
            file_name=f'{report_name}.xlsx',
            mime='application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'
        )

    # Funzione per generare il report PDF con attività completate
    def generate_pdf_report(df_anomalie, activities_df, completed_activities, report_name):
        pdf = FPDF()
        pdf.set_auto_page_break(auto=True, margin=15)
        pdf.add_page()

        # Aggiungi titolo
        pdf.set_font("Arial", 'B', 16)
        pdf.cell(200, 10, txt="VitiMonitor - Report", ln=True, align='C')
        
        # Anomalie
        pdf.ln(10)
        pdf.set_font("Arial", 'B', 12)
        pdf.cell(200, 10, txt="Anomalie Rilevate", ln=True)
        pdf.set_font("Arial", '', 10)
        for index, row in df_anomalie.iterrows():
            pdf.cell(200, 10, txt=f"ID Sensore: {row['sensor_id']} | Zona: {row['zone']} | Metrica: {row['temperature']}", ln=True)
        
        # Attività
        pdf.ln(10)
        pdf.set_font("Arial", 'B', 12)
        pdf.cell(200, 10, txt="Attività Pianificate", ln=True)
        pdf.set_font("Arial", '', 10)
        for index, row in activities_df.iterrows():
            pdf.cell(200, 10, txt=f"Data: {row['data']} | Attività: {row['attività']} | Priorità: {row['priorità']}", ln=True)

        # Attività Completate
        pdf.ln(10)
        pdf.set_font("Arial", 'B', 12)
        pdf.cell(200, 10, txt="Attività Completate", ln=True)
        pdf.set_font("Arial", '', 10)
        for activity in completed_activities:
            pdf.cell(200, 10, txt=f"Data: {activity['data']} | Attività: {activity['attività']} | Zona: {activity['zona']} | Priorità: {activity['priorità']}", ln=True)

        # Salva il PDF
        pdf.output(f"{report_name}.pdf")

        st.download_button(
            label="Scarica Report PDF",
            data=open(f'{report_name}.pdf', 'rb'),
            file_name=f'{report_name}.pdf',
            mime='application/pdf'
        )

    # Genera report Excel e PDF
    if st.button("Genera Report Excel"):
        generate_csv_report(df_anomalie, activities_df, st.session_state['completed_activities'], report_name)

    if st.button("Genera Report PDF"):
        generate_pdf_report(df_anomalie, activities_df, st.session_state['completed_activities'], report_name)

