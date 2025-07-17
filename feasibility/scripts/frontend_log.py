import streamlit as st
import pandas as pd
import plotly.express as px
import pydeck as pdk
from datetime import datetime, time
from streamlit_autorefresh import st_autorefresh
from common import METRICHE, SOGLIE, check_thresholds
import random

def render(df, backend_url):
    st.title("📝 Log — VitiMonitor")
    st.caption("Analisi storica e in tempo reale dei dati sensori")

    if 'log_dark_mode' not in st.session_state:
        st.session_state.log_dark_mode = True

    dark_mode = st.session_state.log_dark_mode

    # Definizione dei sensori disponibili e delle metriche
    sensori_disponibili = sorted(df['sensor_id'].unique())
    selected_sensors = sensori_disponibili  # Selezionati tutti i sensori disponibili

    selected_metrics = METRICHE  # Selezionate tutte le metriche disponibili

    # --- FILTRI SIDEBAR ---
    st.sidebar.markdown("---")
    df_filtered = df
    #df_filtered = df[
    #    (df['timestamp'] >= start_dt) &
    #    (df['timestamp'] <= end_dt) &
    #    df['lat'].between(*lat_range) &
    #    df['lon'].between(*lon_range) &
    #    df['sensor_id'].isin(selected_sensors)
    #]

    if not df_filtered.empty:
        st.success(f"Ultimo aggiornamento: **{df_filtered['timestamp'].max()}**")
    else:
        st.info("Nessun dato disponibile con i filtri selezionati.")

    # --- TABS ---
    tabs = st.tabs([
        "📊 Grafici storici",
        "🔬 Analisi dati",
        "🗺️ Mappa GPS",
        "🧾 Tabella dati"
    ])

    # --- Grafici storici ---
    with tabs[0]:
        st.subheader("📊 Grafici temporali")
        for metric in selected_metrics:
            df_metric = df_filtered[['timestamp', 'sensor_id', metric, 'zone']].dropna()
            fig = px.line(
                df_metric, x="timestamp", y=metric, color="sensor_id",
                title=f"{metric.replace('_', ' ').capitalize()} per sensore", markers=True
            )
            st.plotly_chart(fig, use_container_width=True)

    # --- Analisi Dati ---
    with tabs[1]:
        st.subheader("🔬 Analisi Dati")

        threshold = st.slider("📊 Soglia dati", 0.0, 1.0, 0.5)
        factors = st.multiselect("🎛️ Fattori", METRICHE, default=METRICHE)

        df_analysis = df_filtered.copy()

        recommendations = pd.DataFrame()
        if df_analysis is not None and factors:
            st.markdown("### ⚖️ Assegna pesi ai fattori")
            pesi = {factor: st.slider(f"Peso per {factor}", 0.0, 1.0, 1.0/len(factors), 0.05) for factor in factors}
            if st.button("🧪 Esegui analisi dati"):
                with st.spinner("Calcolo…"):
                    colonne_valide = []
                    for factor in factors:
                        if factor in df_analysis.columns:
                            min_val, max_val = SOGLIE[factor]
                            df_analysis[f"norm_{factor}"] = df_analysis[factor].clip(min_val, max_val)
                            df_analysis[f"norm_{factor}"] = (df_analysis[f"norm_{factor}"] - min_val) / (max_val - min_val)
                            colonne_valide.append(factor)
                    somma_pesi = sum(pesi[f] for f in colonne_valide)
                    pesi_norm = {f: pesi[f]/somma_pesi for f in colonne_valide}
                    df_analysis["composite_score"] = sum(
                        df_analysis[f"norm_{f}"] * pesi_norm[f] for f in colonne_valide
                    )
                    recommendations = df_analysis[df_analysis["composite_score"] > threshold]
                if not recommendations.empty:
                    st.markdown(f"### 🔷 Raccomandazioni: {len(recommendations)} punti sopra soglia")
                    st.dataframe(recommendations, use_container_width=True)
                else:
                    st.info("Nessun punto sopra la soglia.")
                st.text_area("📝 Annotazioni")
    # --- Mappa GPS ---
    with tabs[2]:
        st.subheader("🗺️ Mappa GPS")
    
        # Genera dati GPS casuali per una lista di sensor_id
        sensor_ids = [1, 2, 3, 4, 5]  # Esempio di lista di sensor_id
        gps_data = [
            {'sensor_id': sensor_id, 'lat': 45.7 + random.uniform(-0.01, 0.01), 'lon': 9.0 + random.uniform(-0.01, 0.01)}
            for sensor_id in sensor_ids
        ]
        
        # Crea un DataFrame da questi dati
        df_gps_generated = pd.DataFrame(gps_data)
    
        if not df_gps_generated.empty:
            midpoint = (df_gps_generated['lat'].mean(), df_gps_generated['lon'].mean())
            layer = pdk.Layer(
                "ScatterplotLayer",
                data=df_gps_generated,
                get_position='[lon, lat]',
                get_color='[200, 30, 0, 160]',
                get_radius=50,
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
    with tabs[3]:
        st.subheader("🧾 Tabella dati")
        if not df_filtered.empty:
            df_display = df_filtered[['timestamp', 'sensor_id', 'signature'] + selected_metrics].copy()
            df_display['signature'] = df_display['signature'].apply(lambda s: s[:8] + '...' if s else '')
            st.dataframe(df_display, use_container_width=True)

            csv = df_display.to_csv(index=False).encode("utf-8")
            st.download_button("📤 Esporta CSV", data=csv, file_name=f"log_{datetime.now().date()}.csv", mime="text/csv")
        else:
            st.info("Nessun dato disponibile per la tabella")

    # --- Footer ---
    st.markdown("---")
    st.caption(f"VitiMonitor © 2025 — Modalità: 🌙 Scura" if dark_mode else "VitiMonitor © 2025 — Modalità: ☀️ Chiara")

