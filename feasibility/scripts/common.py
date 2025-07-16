import streamlit as st
import pandas as pd
import requests

# Costanti aggiornate
METRICHE = ['temperature', 'humidity_air', 'humidity_soil', 'luminosity']

SOGLIE = {
    'temperature': (0, 40),
    'humidity_air': (30, 70),
    'humidity_soil': (15, 35),
    'luminosity': (0, 100000)
}

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
    if 'lat' in df.columns and 'lon' in df.columns:
        df['lat'] = df['lat']
        df['lon'] = df['lon']
    if 'timestamp' in df.columns:
        df['timestamp'] = pd.to_datetime(df['timestamp'])
    return df

def check_thresholds(df, metric):
    min_val, max_val = SOGLIE[metric]
    return df[(df[metric] < min_val) | (df[metric] > max_val)]

