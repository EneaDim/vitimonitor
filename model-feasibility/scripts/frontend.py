import streamlit as st
import requests
import pandas as pd
from datetime import datetime, date
from streamlit_autorefresh import st_autorefresh

# --- CONFIGURAZIONE INIZIALE ---

# Frequenze di polling disponibili (in secondi)
POLLING_OPTIONS = [0, 5, 10, 30, 60]  # 0 = polling disabilitato

# --- FUNZIONI UTILI ---

def fetch_sensor_data(backend_url):
    """
    Richiama il backend all'endpoint /data e ritorna i dati in forma di lista di dizionari.
    """
    try:
        response = requests.get(f"{backend_url}/data")
        response.raise_for_status()
        json_data = response.json()
        return json_data.get("data", [])
    except requests.RequestException as e:
        st.error(f"Errore nel fetch dei dati: {e}")
        return []

def data_to_dataframe(data):
    """
    Converte la lista di dizionari in un DataFrame pandas, utile per tabelle e grafici.
    Gestisce anche la conversione timestamp in datetime.
    """
    if not data:
        return pd.DataFrame()
    df = pd.DataFrame(data)
    # Espandi gps in colonne lat, lon
    df['lat'] = df['gps'].apply(lambda g: g.get('lat') if g else None)
    df['lon'] = df['gps'].apply(lambda g: g.get('lon') if g else None)
    # Converti timestamp in datetime (se presente)
    if 'timestamp' in df.columns:
        df['timestamp'] = pd.to_datetime(df['timestamp'])
    return df

def filter_dataframe(df, date_range, selected_types, lat_range, lon_range):
    """
    Filtra il dataframe per data, tipo dato e coordinate GPS.
    """
    if df.empty:
        return df
    
    # Filtro data (timestamp)
    if date_range:
        start_date, end_date = date_range
        df = df[(df['timestamp'] >= pd.Timestamp(start_date)) & (df['timestamp'] <= pd.Timestamp(end_date) + pd.Timedelta(days=1))]
    
    # Filtro tipo dato (temperature, humidity, luminosity)
    # Il filtro riguarda cosa visualizzare, quindi si userà nel frontend (grafici e tabella)
    # Qui non si filtra il df, lo lasciamo intero per mostrare tutto in tabella.
    # Però potresti voler filtrare qui se vuoi.

    # Filtro GPS
    if lat_range:
        lat_min, lat_max = lat_range
        df = df[(df['lat'] >= lat_min) & (df['lat'] <= lat_max)]
    if lon_range:
        lon_min, lon_max = lon_range
        df = df[(df['lon'] >= lon_min) & (df['lon'] <= lon_max)]
    
    return df

def display_data_table(df, selected_types):
    """
    Mostra una tabella con i dati sensoriali, nasconde la firma completa.
    Mostra solo i tipi selezionati.
    """
    if df.empty:
        st.info("Nessun dato disponibile al momento.")
        return

    cols = ['timestamp', 'lat', 'lon', 'signature'] + selected_types
    # signature accorciata
    df_display = df[cols].copy()
    df_display['signature'] = df_display['signature'].apply(lambda s: s[:8] + '...' if s else '')

    st.dataframe(df_display)

def display_charts(df, selected_types):
    """
    Visualizza grafici delle metriche selezionate nel tempo.
    """
    if df.empty:
        return

    st.subheader("Andamento dei sensori nel tempo")

    for metric in selected_types:
        if metric in df.columns:
            st.markdown(f"### {metric.capitalize()}")
            st.line_chart(df.set_index('timestamp')[metric])

# --- STREAMLIT UI ---

st.title("Dashboard Sensori Vitivinicoli")

# Input backend URL
backend_url = st.sidebar.text_input("URL Backend API", value="http://localhost:8000")

# Sidebar per controlli utente
st.sidebar.header("Configurazione Polling")

# Imposta valori di default sessione per polling se non esistono
if 'polling_interval' not in st.session_state:
    st.session_state.polling_interval = 5  # default 5s
if 'polling_enabled' not in st.session_state:
    st.session_state.polling_enabled = True

# Toggle abilitazione polling
polling_enabled = st.sidebar.checkbox("Abilita polling automatico", value=st.session_state.polling_enabled)
st.session_state.polling_enabled = polling_enabled

# Se polling abilitato, seleziona frequenza
polling_interval = 0
if polling_enabled:
    polling_interval = st.sidebar.selectbox(
        "Frequenza polling (secondi)",
        POLLING_OPTIONS[1:],  # escludi 0
        index=POLLING_OPTIONS[1:].index(st.session_state.polling_interval) if st.session_state.polling_interval in POLLING_OPTIONS else 0
    )
else:
    polling_interval = 0  # polling off

st.session_state.polling_interval = polling_interval

# Pulsante manuale di refresh
if st.sidebar.button("Aggiorna dati manualmente"):
    st.experimental_rerun()

# --- Filtri dati ---

st.sidebar.header("Filtri dati")

# Filtro data range (default ultimi 7 giorni)
min_date = datetime(2000, 1, 1)
max_date = datetime.now()
date_range = st.sidebar.date_input("Intervallo date", value=(max_date.date() - pd.Timedelta(days=7), max_date.date()), min_value=min_date.date(), max_value=max_date.date())

# Seleziona tipo dato da mostrare
tipo_dati_possibili = ['temperature', 'humidity', 'luminosity']
selected_types = st.sidebar.multiselect("Tipi di dato da visualizzare", options=tipo_dati_possibili, default=tipo_dati_possibili)

# Filtro GPS: range latitudine e longitudine
# Per semplicità prendiamo range ampi di default
lat_min, lat_max = st.sidebar.slider("Intervallo latitudine", -90.0, 90.0, (-90.0, 90.0), step=0.1)
lon_min, lon_max = st.sidebar.slider("Intervallo longitudine", -180.0, 180.0, (-180.0, 180.0), step=0.1)

# --- Fetch dati e UI ---

# Polling automatico con st_autorefresh
if polling_enabled and polling_interval > 0:
    st_autorefresh(interval=polling_interval * 1000, key="polling")

data = fetch_sensor_data(backend_url)
df = data_to_dataframe(data)

# Applica filtri
df_filtered = filter_dataframe(df, date_range, selected_types, (lat_min, lat_max), (lon_min, lon_max))

# Mostra info ultimo timestamp
if not df_filtered.empty:
    last_time = df_filtered['timestamp'].max()
    st.write(f"Dati aggiornati all'ultimo timestamp: **{last_time}**")
else:
    st.write("Nessun dato disponibile.")

# Tabella
display_data_table(df_filtered, selected_types)

# Grafici
display_charts(df_filtered, selected_types)

# Mappa GPS
if not df_filtered.empty and df_filtered[['lat', 'lon']].dropna().shape[0] > 0:
    st.subheader("Posizione GPS dei dati raccolti")
    st.map(df_filtered[['lat', 'lon']].dropna())

