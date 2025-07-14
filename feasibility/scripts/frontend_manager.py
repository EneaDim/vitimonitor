import streamlit as st
import pandas as pd
import plotly.express as px
from common import check_thresholds

def render(df):
    st.header("👨‍💼 Manager")
    df['date'] = df['timestamp'].dt.date
    last_week = pd.Timestamp.today() - pd.Timedelta(days=7)
    df_week = df[df['timestamp'] >= last_week]

    # 🔷 Parametri economici
    st.subheader("💰 Parametri Economici")
    col_econ1, col_econ2, col_econ3 = st.columns(3)
    with col_econ1:
        prezzo_kg = st.number_input("Prezzo uva (€/kg)", value=1.5, step=0.1, format="%.2f")
    with col_econ2:
        costo_critico_per_zona = st.number_input("Penalità per zona a rischio (€)", value=500, step=50)
    with col_econ3:
        produzione_base_kg = st.number_input("Produzione ideale (kg)", value=20000, step=1000)

    # 🔷 Calcoli economici
    produzione_stimata = produzione_base_kg * (1 - df_week['temperature'].std() / 10)
    ricavi = produzione_stimata * prezzo_kg

    df_zone = df_week.groupby('zone').agg({
        'temperature': ['mean', 'std'],
        'humidity_air': ['mean', 'std'],
        'humidity_soil': ['mean', 'std']
    }).reset_index()
    df_zone.columns = [
        'zone', 'temp_mean', 'temp_std', 'hum_air_mean', 'hum_air_std', 'hum_soil_mean', 'hum_soil_std'
    ]
    df_zone['rischio'] = (df_zone['temp_std'] + df_zone['hum_air_std'] + df_zone['hum_soil_std']) / 3
    zone_critiche = df_zone[df_zone['rischio'] > 2.0]
    costo_critico_totale = len(zone_critiche) * costo_critico_per_zona
    roi_stimato = ricavi - costo_critico_totale

    st.markdown("---")

    # 🔷 Indicatore Chiave di Prestazione economici
    st.subheader("📊 Indicatore Chiave di Prestazione settimanali — Economia")
    kpi_econ_cols = st.columns(2)
    kpi_econ_cols[0].metric("🍇 Produzione stimata", f"{round(produzione_stimata):,} kg")
    kpi_econ_cols[1].metric("💰 ROI stimato", f"{roi_stimato:,.0f} €")

    st.markdown("---")

    # 🔷 Indicatore Chiave di Prestazione misurazioni
    st.subheader("📊 Indicatore Chiave di Prestazione settimanali — Misurazioni")
    kpi_mis_cols = st.columns(5)
    kpi_mis_cols[0].metric("🌡️ Temp media (7gg)", f"{df_week['temperature'].mean():.1f}°C")
    kpi_mis_cols[1].metric("💧 Umidità aria media (7gg)", f"{df_week['humidity_air'].mean():.1f}%")
    kpi_mis_cols[2].metric("🌱 Umidità suolo media (7gg)", f"{df_week['humidity_soil'].mean():.1f}%")
    kpi_mis_cols[3].metric("📈 Sensori attivi", f"{df['sensor_id'].nunique()}")
    kpi_mis_cols[4].metric("🚨 Anomalie", len(pd.concat(
        [check_thresholds(df, m) for m in ['temperature', 'humidity_air', 'humidity_soil', 'luminosity']]
    )))

    st.markdown("---")

    # 🔷 Distribuzione
    st.subheader("📊 Distribuzione misurazioni")

    for metric, label in [
        ('temperature', 'Temperatura (°C)'),
        ('humidity_air', 'Umidità Aria (%)'),
        ('humidity_soil', 'Umidità Suolo (%)')
    ]:
        st.markdown(f"### {label}")

        fig = px.violin(
            df_week,
            y='zone',
            x=metric,
            color='zone',
            box=True,
            points='all',
            orientation='h',
            title=f"Distribuzione di {label} per Zona",
            color_discrete_sequence=px.colors.qualitative.Pastel
        )
        fig.update_traces(meanline_visible=True)
        fig.update_layout(
            xaxis_title=label,
            yaxis_title='Zona',
            height=500,
            showlegend=False
        )
        st.plotly_chart(fig, use_container_width=True)

    st.markdown("---")

