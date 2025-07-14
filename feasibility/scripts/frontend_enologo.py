import streamlit as st
import pandas as pd
import plotly.express as px

def render(df):
    st.header("🍷 Enologo — Analisi Avanzata")

    # --- Target ottimali per zona ---
    st.subheader("🎯 Valori Ottimali per Zona")

    st.markdown("""
    In questa sezione puoi impostare i valori ottimali per ogni zona.
    """)
    default_targets = pd.DataFrame({
        'zone': df['zone'].unique(),
        'temperature_opt': [22] * len(df['zone'].unique()),
        'humidity_air_opt': [50] * len(df['zone'].unique()),
        'humidity_soil_opt': [30] * len(df['zone'].unique()),
    })

    target_df = st.data_editor(
        default_targets,
        num_rows="dynamic",
        key="targets_editor"
    )

    st.markdown("""
    ℹ️ **Indice di Qualità**: misura aggregata della vicinanza delle condizioni (temperatura, umidità aria, umidità suolo) ai valori ottimali per la vite. Varia tra 0 (condizioni pessime) e 1 (condizioni ideali).
    """)

    # Unisci i target ai dati
    df = df.merge(target_df, on='zone', how='left')

    # Calcolo quality_index per riga
    df['q_temp'] = 1 - abs(df['temperature'] - df['temperature_opt']) / df['temperature_opt']
    df['q_hum_air'] = 1 - abs(df['humidity_air'] - df['humidity_air_opt']) / df['humidity_air_opt']
    df['q_hum_soil'] = 1 - abs(df['humidity_soil'] - df['humidity_soil_opt']) / df['humidity_soil_opt']

    df['q_temp'] = df['q_temp'].clip(lower=0)
    df['q_hum_air'] = df['q_hum_air'].clip(lower=0)
    df['q_hum_soil'] = df['q_hum_soil'].clip(lower=0)

    df['quality_index'] = (
        0.5 * df['q_temp'] +
        0.3 * df['q_hum_air'] +
        0.2 * df['q_hum_soil']
    )

    # --- Distribuzione qualità per zona ---
    df_stats = df[['zone', 'quality_index']]

    st.subheader("📊 Distribuzione Indice di Qualità per Zona")

    fig_zone = px.box(
        df_stats,
        x='quality_index',
        y='zone',
        points='all',
        orientation='h',
        color='zone',
        title='Distribuzione Indice di Qualità per Zona',
        color_discrete_sequence=px.colors.qualitative.Pastel
    )

    fig_zone.update_traces(
        boxmean=True,
        jitter=0.3,
        marker=dict(opacity=0.4, size=4)
    )

    fig_zone.update_layout(
        xaxis=dict(title='Indice di qualità', range=[1,0]),
        yaxis_title='Zona',
        height=500,
        showlegend=False
    )

    st.plotly_chart(fig_zone, use_container_width=True)

    # Soglia selezionabile + metric semplice
    soglia_qualita = st.slider("Soglia qualità minima", 0.7, 1.0, 0.85)
    df_buone = df[df['quality_index'] >= soglia_qualita]

    total_points = len(df)
    above_min_points = len(df_buone)

    if total_points > 0:
        perc_above = (above_min_points / total_points) * 100
    else:
        perc_above = 0

    st.metric(
        label="🔷 Punti sopra la soglia minima",
        value=f"{above_min_points} / {total_points} ({perc_above:.1f}%)"
    )

    # Distribuzione intra-zona
    st.subheader("📊 Distribuzione intra-zona")
    metric = st.selectbox(
        "Scegli metrica per distribuzione",
        ['temperature', 'humidity_air', 'humidity_soil', 'luminosity']
    )

    fig_dist = px.violin(
        df,
        x='zone',
        y=metric,
        color='zone',
        box=True,                # mostra boxplot interno al violin
        points='all',            # mostra punti individuali
        title=f"Distribuzione {metric} per zona",
        color_discrete_sequence=px.colors.qualitative.Pastel
    )

    fig_dist.update_traces(meanline_visible=True)
    fig_dist.update_layout(
        xaxis_title='Zona',
        yaxis_title=metric,
        height=500,
        showlegend=False
    )

    st.plotly_chart(fig_dist, use_container_width=True)

    # Trend temporali per metrica
    st.subheader("📈 Trend temporali")
    zone_sel = st.multiselect("Zone da analizzare", options=df['zone'].unique(), default=list(df['zone'].unique()))
    metric_trend = st.selectbox("Metrica per trend", ['temperature', 'humidity_air', 'humidity_soil', 'luminosity'])

    df_trend = df[df['zone'].isin(zone_sel)]
    fig_trend = px.line(df_trend, x='timestamp', y=metric_trend, color='zone', markers=True,
                        title=f"Trend temporale di {metric_trend}")
    st.plotly_chart(fig_trend, use_container_width=True)

    # Heatmap zone a rischio
    st.subheader("🔥 Zone a rischio")

    risk_metric = st.selectbox(
        "Metrica per rischio",
        ['temperature', 'humidity_air', 'humidity_soil']
    )

    if risk_metric == 'temperature':
        optimal = (15, 30)
    elif risk_metric == 'humidity_air':
        optimal = (40, 60)
    else:
        optimal = (20, 35)

    df_risk_low = df[df[risk_metric] < optimal[0]].copy()
    df_risk_high = df[df[risk_metric] > optimal[1]].copy()
    df_risk = pd.concat([df_risk_low, df_risk_high])

    if not df_risk.empty:
        st.error(f"⚠️ {len(df_risk)} punti fuori intervallo ottimale per **{risk_metric}**")

        col1, col2 = st.columns(2)
        col1.metric(f"🔻 Sotto {optimal[0]}", f"{len(df_risk_low)}")
        col2.metric(f"🔺 Sopra {optimal[1]}", f"{len(df_risk_high)}")

        # Zone più colpite
        zone_risk = df_risk.groupby("zone").size().reset_index(name="count").sort_values("count", ascending=False)
        st.markdown("### 📍 Zone più critiche")
        st.dataframe(zone_risk)

        # Tabella completa
        st.markdown("### 📋 Dettaglio punti a rischio")
        st.dataframe(
            df_risk[
                ['sensor_id', 'zone', 'timestamp', risk_metric]
            ].sort_values(by='timestamp', ascending=False),
            use_container_width=True
        )
    else:
        st.success(f"✅ Tutti i valori di **{risk_metric}** sono entro l'intervallo ottimale")


    st.subheader("💧☀️ Stress Idrico")

    st.markdown("""
    Valori **bassi di umidità suolo** combinati con **alta luminosità** possono indicare condizioni di stress idrico
    per la pianta. Qui vengono evidenziate le misurazioni sospette e le zone maggiormente a rischio.
    """)

    # Parametri configurabili
    col1, col2 = st.columns(2)
    soil_threshold = col1.slider("Soglia umidità suolo minima (%)", 10, 40, 20)
    lum_threshold = col2.slider("Soglia luminosità massima (lux)", 50000, 150000, 100000)

    # Filtra punti a rischio
    df_stress = df[(df['humidity_soil'] <= soil_threshold) & (df['luminosity'] >= lum_threshold)]

    if not df_stress.empty:
        st.error(f"⚠️ {len(df_stress)} misurazioni indicano possibile **stress idrico**")

        # Zone più colpite
        zone_stress = df_stress.groupby("zone").size().reset_index(name="count").sort_values("count", ascending=False)
        st.markdown("### 📍 Zone più a rischio")
        st.dataframe(zone_stress)

        # Scatterplot
        fig_stress = px.scatter(
            df, x='luminosity', y='humidity_soil', color='zone',
            title="Relazione tra luminosità e umidità suolo",
            labels={"luminosity": "Luminosità (lux)", "humidity_soil": "Umidità Suolo (%)"}
        )
        fig_stress.add_shape(
            type="rect",
            x0=lum_threshold, x1=df['luminosity'].max(),
            y0=0, y1=soil_threshold,
            line=dict(color="red", dash="dot"),
            fillcolor="rgba(255,0,0,0.1)",
            layer="below"
        )
        st.plotly_chart(fig_stress, use_container_width=True)

        st.caption(
            f"Sono evidenziati i punti con umidità suolo ≤ {soil_threshold}% e luminosità ≥ {lum_threshold} lux come aree potenzialmente a rischio."
        )

        # Tabella dettagliata
        st.markdown("### 📋 Dettaglio misurazioni a rischio")
        st.dataframe(
            df_stress[['sensor_id', 'zone', 'timestamp', 'humidity_soil', 'luminosity']].sort_values(by='timestamp', ascending=False),
            use_container_width=True
        )
    else:
        st.success("✅ Nessuna condizione di stress idrico rilevata secondo le soglie impostate.")

