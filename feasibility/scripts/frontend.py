import streamlit as st
import pandas as pd
import requests
from datetime import datetime, time
import plotly.express as px
from streamlit_autorefresh import st_autorefresh
import time as t

# --- CONFIG APP ---
st.set_page_config(
    page_title="VitiMonitor Dashboard",
    page_icon="📊",
    layout="wide",
    initial_sidebar_state="expanded"
)

# --- COSTANTI ---
POLLING_OPTIONS = [5, 10, 30, 60]
METRICHE = ['temperature', 'humidity', 'luminosity']
SOGLIE = {
    'temperature': (0, 40),
    'humidity': (10, 90),
    'luminosity': (0, 100000)
}

# --- FUNZIONI ---
def fetch_sensor_data(backend_url):
    try:
        response = requests.get(f"{backend_url}/data", timeout=5)
        response.raise_for_status()
        return response.json().get("data", [])
    except requests.RequestException as e:
        st.error(f"❌ Errore nel fetch dei dati: {e}")
        return []

def data_to_dataframe(data):
    if not data:
        return pd.DataFrame()
    df = pd.DataFrame(data)
    df['lat'] = df['gps'].apply(lambda g: g.get('lat') if g else None)
    df['lon'] = df['gps'].apply(lambda g: g.get('lon') if g else None)
    if 'timestamp' in df.columns:
        df['timestamp'] = pd.to_datetime(df['timestamp'])
    return df

def export_csv(df):
    return df.to_csv(index=False).encode("utf-8")

def check_thresholds(df, metric):
    min_val, max_val = SOGLIE[metric]
    return df[(df[metric] < min_val) | (df[metric] > max_val)]

# --- SIDEBAR ---
st.sidebar.title("⚙️ Configurazione")

if 'dark_mode' not in st.session_state:
    st.session_state.dark_mode = True

dark_mode = st.sidebar.toggle("🌙 Modalità scura", value=st.session_state.dark_mode)
st.session_state.dark_mode = dark_mode

backend_url = "http://localhost:8000"

st.sidebar.markdown("---")
st.sidebar.subheader("🎛️ Filtri dati")

today = datetime.today().date()
date_range = st.sidebar.date_input("Intervallo date", value=(today, today))
orario_inizio, orario_fine = st.sidebar.columns(2)
start_time = orario_inizio.time_input("Orario inizio", value=time(0,0))
end_time = orario_fine.time_input("Orario fine", value=time(23,59))

st.title("📊 VitiMonitor Dashboard")
st.caption("Analisi comparativa dei sensori")

data = fetch_sensor_data(backend_url)
df = data_to_dataframe(data)

if df.empty:
    st.warning("Nessun dato ricevuto dal backend.")
    st.stop()

sensori_disponibili = sorted(df['sensor_id'].unique())
selected_sensors = st.sidebar.multiselect("Sensori da visualizzare", options=sensori_disponibili, default=sensori_disponibili)
st.sidebar.markdown("---")
selected_metrics = st.sidebar.multiselect("Metriche da visualizzare", METRICHE, default=METRICHE)
lat_range = st.sidebar.slider("Latitudine", -90.0, 90.0, (-90.0, 90.0), step=0.1)
lon_range = st.sidebar.slider("Longitudine", -180.0, 180.0, (-180.0, 180.0), step=0.1)

st.sidebar.markdown("---")
st.sidebar.subheader("🔄 Polling dati")

enable_polling = st.sidebar.checkbox("Abilita polling automatico", value=True)
polling_interval = st.sidebar.selectbox("Frequenza (s)", POLLING_OPTIONS, index=0)

if st.sidebar.button("📥 Aggiorna manualmente"):
    st.rerun()

st.sidebar.markdown("---")

backend_url_side = st.sidebar.text_input("🌐 URL Backend API", value=backend_url)

# --- MAIN ---

start_dt = datetime.combine(date_range[0], start_time)
end_dt = datetime.combine(date_range[1], end_time)

df_filtered = df[
    (df['timestamp'] >= start_dt) &
    (df['timestamp'] <= end_dt) &
    df['lat'].between(*lat_range) &
    df['lon'].between(*lon_range) &
    df['sensor_id'].isin(selected_sensors)
]

if not df_filtered.empty:
    st.success(f"Ultimo aggiornamento: **{df_filtered['timestamp'].max()}**")
else:
    st.info("Nessun dato disponibile con i filtri selezionati.")

# --- TABS ---
tab_labels = [
    "📊 Confronto grafico",
    "🗺️ Mappa GPS",
    "🧾 Tabella dati",
    "🔬 Fusion & Culture Planner"
]
tabs = st.tabs(tab_labels)

if "active_tab" not in st.session_state:
    st.session_state.active_tab = tab_labels[0]

for idx, tab in enumerate(tabs):
    with tab:
        st.session_state.active_tab = tab_labels[idx]

if enable_polling and st.session_state.active_tab != "🔬 Fusion & Culture Planner":
    st_autorefresh(interval=polling_interval * 1000, key="polling_enabled")

# --- Confronto grafico ---
with tabs[0]:
    st.subheader("📊 Confronto fra sensori (grafici interattivi)")
    for metric in selected_metrics:
        df_metric = df_filtered[['timestamp', 'sensor_id', metric, 'zone']].dropna()
        fig = px.line(df_metric, x="timestamp", y=metric, color="sensor_id", title=f"{metric.capitalize()} per sensore", markers=True)
        st.plotly_chart(fig, use_container_width=True)

        alerts = check_thresholds(df_metric, metric)
        if not alerts.empty:
            st.error(f"⚠️ {len(alerts)} valori fuori soglia per {metric}:")
            alerts_display = alerts[['sensor_id', 'zone', 'timestamp', metric]].sort_values('timestamp').copy()
            alerts_display.columns = ["Sensore", "Zona", "Timestamp", metric]
            alerts_display["Vai"] = [
                f"[🔍 Vai](?sensor_id={row['Sensore']}&timestamp={row['Timestamp']})"
                for _, row in alerts_display.iterrows()
            ]
            st.dataframe(alerts_display, height=300, use_container_width=True)

# --- Mappa GPS ---
with tabs[1]:
    st.subheader("🗺️ Posizioni GPS")
    import pydeck as pdk
    df_gps = df_filtered[['lat', 'lon']].dropna()
    if not df_gps.empty:
        midpoint = (df_gps['lat'].mean(), df_gps['lon'].mean())
        layer = pdk.Layer(
            "ScatterplotLayer",
            data=df_gps,
            get_position='[lon, lat]',
            get_color='[200, 30, 0, 160]',
            get_radius=50,  # puoi ridurre qui, es. 10-50
            pickable=True,
        )
        view_state = pdk.ViewState(
            latitude=midpoint[0],
            longitude=midpoint[1],
            zoom=12,
            pitch=0,
        )
        st.pydeck_chart(pdk.Deck(layers=[layer], initial_view_state=view_state))
    else:
        st.info("Nessun dato GPS disponibile")

# --- Tabella dati ---
with tabs[2]:
    st.subheader("🧾 Dati tabellari")
    if not df_filtered.empty:
        df_display = df_filtered[['timestamp', 'sensor_id', 'lat', 'lon', 'signature'] + selected_metrics].copy()
        df_display['signature'] = df_display['signature'].apply(lambda s: s[:8] + '...' if s else '')
        st.dataframe(df_display, use_container_width=True)

        csv = export_csv(df_display)
        st.download_button("📤 Esporta dati CSV", data=csv, file_name=f"sensor_data_{datetime.now().date()}.csv", mime="text/csv")
    else:
        st.info("Nessun dato disponibile per la tabella")

# --- Fusion Analysis ---
with tabs[3]:
    st.subheader("🔬 Fusion Analysis & Culture Operations")

    fonte_dati = st.radio("📥 Seleziona la fonte dei dati", ["Carica file CSV/Excel", "Usa dati raccolti dal database"])
    threshold = st.slider("📊 Soglia di fusion score", 0.0, 1.0, 0.5)
    factors = st.multiselect("🎛️ Fattori da includere", METRICHE, default=METRICHE)

    df_fusion = None
    if fonte_dati == "Carica file CSV/Excel":
        uploaded_file = st.file_uploader("📁 Carica dataset di fusion", type=["csv", "xlsx"])
        if uploaded_file:
            df_fusion = pd.read_csv(uploaded_file) if uploaded_file.name.endswith(".csv") else pd.read_excel(uploaded_file)
            st.dataframe(df_fusion.head(), use_container_width=True)
    else:
        df_fusion = df_filtered.copy()
        st.dataframe(df_fusion.head(), use_container_width=True)


    recommendations = pd.DataFrame()  # inizializza vuoto
    if df_fusion is not None and factors:
        st.markdown("### ⚖️ Assegna pesi ai fattori")
        pesi = {factor: st.slider(f"Peso per {factor}", 0.0, 1.0, 1.0/len(factors), 0.05) for factor in factors}
        if st.button("🧪 Esegui analisi"):
            with st.spinner("Calcolo fusion score…"):
                colonne_valide = []
                for factor in factors:
                    if factor in df_fusion.columns:
                        min_val, max_val = SOGLIE[factor]
                        df_fusion[f"norm_{factor}"] = df_fusion[factor].clip(min_val, max_val)
                        df_fusion[f"norm_{factor}"] = (df_fusion[f"norm_{factor}"] - min_val) / (max_val - min_val)
                        colonne_valide.append(factor)
                somma_pesi = sum(pesi[f] for f in colonne_valide)
                pesi_norm = {f: pesi[f]/somma_pesi for f in colonne_valide}
                df_fusion["fusion_score"] = sum(
                    df_fusion[f"norm_{f}"] * pesi_norm[f] for f in colonne_valide
                )
                recommendations = df_fusion[df_fusion["fusion_score"] > threshold]
                st.markdown(f"### 🔷 Raccomandazioni: {len(recommendations)} colture sopra soglia")
            if not recommendations.empty:
                st.dataframe(recommendations, use_container_width=True)
            else:
                st.info("Nessuna coltura sopra la soglia.")
            st.text_area("📝 Annotazioni sulle operazioni")
# --- Footer ---
st.markdown("---")
st.caption(f"VitiMonitor © 2025 — Modalità: 🌙 Scura" if dark_mode else "VitiMonitor © 2025 — Modalità: ☀️ Chiara")

