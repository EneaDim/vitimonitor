import random
from datetime import datetime, timedelta
import streamlit as st
import pandas as pd
import requests
from fpdf import FPDF
from common import check_thresholds

def render(df, backend_url):
    st.header("👷 Operatore — Gestione Anomalie e Pianificazione Attività")

    # --- Calcola anomalie ---
    df_anomalie = pd.concat([
        check_thresholds(df, m)
        for m in ['temperature', 'humidity_air', 'humidity_soil', 'luminosity']
    ])

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
    for index, row in df_anomalie.iterrows():
        activities = generate_activity(row)
        for activity in activities:
            activities_list.append({
                'data': datetime.today() + timedelta(days=random.randint(0, 2)),  # Pianifica tra 0 e 2 giorni
                'attività': activity,
                'priorità': 'Alta',  # Puoi aggiungere logiche per assegnare priorità
                'zona': row['zone'],
                'sensor_id': row.get('sensor_id', 'Unknown'),  # Use get to avoid KeyError
                'status': 'Da fare'  # Stato dell'attività
            })

    # Crea il DataFrame delle attività
    activities_df = pd.DataFrame(activities_list)

    # Rimuovi eventuali duplicati (stesso sensore e stessa data)
    activities_df = activities_df.drop_duplicates(subset=['sensor_id', 'data'])

    # --- Pianificazione Attività ---
    st.subheader("📅 Pianificazione Attività")

    # Visualizzazione della tabella delle attività
    activities_df = activities_df.reset_index(drop=True)

    # Selezione attività da modificare/mark as completed
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

        # Chiedi conferma prima di completare l'attività
        confirm_button = st.button(f"✅ Conferma completamento attività: {selected_row['attività']}")

        if confirm_button:
            # Mostra un messaggio per chiedere la conferma
            st.write(f"Sei sicuro di voler completare l'attività: {selected_row['attività']}?")

            # Usa un altro pulsante per confermare il completamento dell'attività
            complete_button = st.button("✅ Completa attività")

            if complete_button:
                # Modifica lo stato dell'attività
                activities_df.loc[selected_activity, 'status'] = 'Completata'  # Modifica lo stato dell'attività

                # Rimuovi l'attività completata dalla lista
                activities_df = activities_df[activities_df['status'] != 'Completata']
                st.success(f"Attività in zona {selected_row['zona']} completata.")

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
        note = st.text_area("Note", value="")

        submitted = st.form_submit_button("Salva Misura")

    if submitted:
        misura = {
            "timestamp": timestamp.isoformat(),
            "zona": zona,
            "sensor_id": sensore_id,
            "temperature": temperatura,
            "humidity_air": umidita_aria,
            "humidity_soil": umidita_suolo,
            "luminosity": luminosita,
            "gps": {"lat": None, "lon": None},
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

    # Funzione per generare report Excel
    def generate_excel_report(df_anomalie, activities_df, report_name):
        with pd.ExcelWriter(f'{report_name}.xlsx') as writer:
            df_anomalie.to_excel(writer, sheet_name='Anomalie', index=False)
            activities_df.to_excel(writer, sheet_name='Attività', index=False)

        st.download_button(
            label="Scarica Report Excel",
            data=open(f'{report_name}.xlsx', 'rb'),
            file_name=f'{report_name}.xlsx',
            mime='application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'
        )

    # Funzione per generare report PDF
    def generate_pdf_report(df_anomalie, activities_df, report_name):
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
        generate_excel_report(df_anomalie, activities_df, report_name)

    if st.button("Genera Report PDF"):
        generate_pdf_report(df_anomalie, activities_df, report_name)

