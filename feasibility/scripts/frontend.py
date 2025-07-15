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
#st.sidebar.title("⚙️ Configurazione")
backend_url = "http://localhost:8000"
#backend_url = st.sidebar.text_input(
#    "🌐 URL Backend API",
#    value="http://localhost:8000",
#    key="backend_url"
#)

# --- DARK MODE ---
if 'dark_mode' not in st.session_state:
    st.session_state.dark_mode = True

dark_mode = st.sidebar.toggle("🌙 Modalità scura", value=st.session_state.dark_mode)
st.session_state.dark_mode = dark_mode

# --- AGGIORNAMENTO DATI ---
st.sidebar.markdown("---")
st.sidebar.title("🔄 Aggiornamento dati")
#st.sidebar.subheader("🔄 Aggiornamento dati")

if st.sidebar.button("📥 Aggiorna manualmente", key="manual_refresh"):
    st.rerun()

# --- CARICA DATI ---
data = fetch_sensor_data(backend_url)
df = data_to_dataframe(data)

if df.empty:
    st.warning("Nessun dato ricevuto dal backend.")
    st.stop()

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

