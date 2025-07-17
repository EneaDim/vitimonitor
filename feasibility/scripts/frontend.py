import pandas as pd
from datetime import datetime, time
import streamlit as st
from common import METRICHE, fetch_sensor_data, data_to_dataframe
import frontend_manager as fm
import frontend_enologo as fe
import frontend_operatore as fo
import frontend_log as fl

# --- CONFIG APP ---
st.set_page_config(
    page_title="VitiMonitor Dashboard",
    page_icon="🍇",
    layout="wide",
    initial_sidebar_state="expanded"
)

st.title("🍇 VitiMonitor Dashboard")
st.caption("Monitoraggio in tempo reale e analisi dei dati dei vigneti")

# --- BACKEND URL ---
backend_url = "http://localhost:8000"

# Memorizza la scelta dell'utente
#dark_mode = st.sidebar.toggle("🌙 Modalità scura", value=st.session_state.dark_mode)
#st.session_state.dark_mode = dark_mode
st.session_state.dark_mode = True

# --- AGGIORNAMENTO DATI ---
st.sidebar.title("🔄 Aggiornamento dati")
#st.sidebar.subheader("🔄 Aggiornamento dati")

if st.sidebar.button("📥 Aggiorna manualmente", key="manual_refresh"):
    st.rerun()

from datetime import datetime, time
import streamlit as st
import requests

# Filtro intervallo dati in frontend.py (sidebar)
st.sidebar.markdown("---")
st.sidebar.title("📈 Intervallo dati")
# Ensure date_range is a tuple of two elements, even if the user selects one date
date_range = st.sidebar.date_input(
    "Intervallo date",
    value=(datetime.today().date(), datetime.today().date()),  # default today
    key="log_date_range"
)

# Check if the date_range is a tuple of length 1 or 2
if isinstance(date_range, tuple) and len(date_range) == 1:
    date_range = (date_range[0], date_range[0])  # If only one date is selected, set it as both start and end date

#date_range = st.sidebar.date_input(
#    "Intervallo date",
#    value=(datetime.today().date(), datetime.today().date()),  # default today
#    key="log_date_range"
#)

start_time = st.sidebar.time_input(
    "Orario inizio",
    value=time(0, 0),
    key="log_start_time"
)

end_time = st.sidebar.time_input(
    "Orario fine",
    value=time(23, 59),
    key="log_end_time"
)

# Converti le date e orari in stringhe per inviarli tramite la richiesta
start_date = date_range[0].strftime('%Y-%m-%d')
end_date = date_range[1].strftime('%Y-%m-%d')
start_time_str = start_time.strftime('%H:%M:%S')
end_time_str = end_time.strftime('%H:%M:%S')

# Costruisci la URL dell'endpoint e invia la richiesta GET
url = f"http://localhost:8000/data?start_date={start_date}&end_date={end_date}&start_time={start_time_str}&end_time={end_time_str}"

# Fai una richiesta GET al backend
response = requests.get(url)

# Gestisci la risposta
if response.status_code == 200:
    data = response.json()['data']
    df = pd.DataFrame(data)
    if df.empty:
        st.warning("Nessun dato ricevuto dal backend.")
        st.stop()
else:
    st.error(f"Errore nel recupero dei dati: {response.status_code}")
    st.stop()

df['timestamp'] = pd.to_datetime(df['timestamp'], utc=True)  # Converte in datetime UTC
df['timestamp'] = df['timestamp'].dt.tz_convert('Europe/Zurich')  # Converti in CEST

# --- TABS FRONTEND ---
tabs = st.tabs(["👨‍💼 Manager", "🍷 Enologo", "👷 Operatore", "📝 Log"])

with tabs[0]:
    fm.render(df)

with tabs[1]:
    fe.render(df)

with tabs[2]:
    fo.render(df, backend_url)

with tabs[3]:
    fl.render(df, backend_url)

