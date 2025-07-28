# 📊 Modelli Agronomici e Predittivi

Vitimonitor integra strumenti agronomici e predittivi per aiutarti a capire **quando intervenire** e **cosa aspettarti dalla stagione**, con modelli concreti validati sul campo.
Ogni modello si basa su dati raccolti dal vigneto e restituisce informazioni comprensibili, utili per la tua gestione quotidiana.

<img src="../images/modelli.png" width="90%" alt="Modelli agronomici predittivi" style="display: block; margin: auto;">

---

## 1. 🌡️ Growing Degree Days (GDD)

**Scopo:** Previsione delle fasi fenologiche della vite (gemmazione, invaiatura, maturazione, raccolta)

**Descrizione:**

- Si basa sull’accumulo termico giornaliero rispetto a una soglia minima (es. 10 °C)
- Utilizzato per tracciare lo sviluppo fenologico delle viti
- Ogni microzona può avere un calcolo personalizzato
- Visualizzato come grafico stagionale
- Soglie regolabili in base al vitigno

**Dati richiesti:**
- Temperatura massima giornaliera
- Temperatura minima giornaliera
- Soglia base per la vite (es. 10 °C)

**Algoritmo consigliato:**
- Calcolo cumulativo GDD giornaliero
- Sliding window per soglie fenologiche (es. 100–150 GDD → invaiatura)

**Output:**
- GDD giornalieri e cumulati
- Mappa fenologica per microzona
- Previsioni delle fasi critiche

---

## 2. 💧 Evapotraspirazione Potenziale (ET₀)

**Scopo:** Calcolo del bilancio idrico per gestire irrigazione e prevenire stress idrico

**Metodo:**

- Si utilizza il metodo standard FAO (Penman‑Monteith)

**Dati richiesti:**
- Temperatura dell’aria (max/min/media)
- Umidità relativa
- Radiazione solare (PAR o globale)
- Velocità del vento *(opzionale)*
- Pressione atmosferica *(opzionale)*

**Algoritmo consigliato:**
- FAO Penman-Monteith ET₀ (librerie Python: `pyet`, `refet`, o `cropET`)
- Integrazione con pluviometro e portata irrigua

**Output:**
- ET₀ giornaliera e cumulata
- Bilancio idrico su base giornaliera
- Alert di carenza o eccesso idrico

---

## 3. 🍃 Modelli Fitopatologici

**Scopo:** Previsione del rischio di malattie fungine sulla vite

### 3.1 Peronospora

**Descrizione:**
- Basato su modelli consolidati in Italia (Goidanich, Baldacci, 3-10-10)
- Considera temperature, umidità relativa e ore di bagnatura fogliare
- Genera segnalazioni in caso di condizioni favorevoli all’infezione

**Dati richiesti:**
- Temperatura media
- Umidità relativa
- Ore di bagnatura fogliare
- Precipitazioni (mm)

**Algoritmo:**
- Modello Goidanich/Baldacci o 3–10–10 (regole empiriche)
- Logica fuzzy o soglie mobili per previsione rischio

**Output:**
- Livello di rischio (basso/medio/alto)
- Giorni con condizioni favorevoli
- Allerta preventiva per trattamenti

### 3.2 Oidio

**Descrizione:**
- Usa il modello Gubler-Thomas (GT)
- Fattori di rischio: temperature comprese tra 21–30 °C, umidità alta, clima caldo e secco
- Fornisce livelli di rischio settimanale con soglie adattabili

**Dati richiesti:**
- Temperatura media giornaliera
- Umidità relativa
- Indice di secchezza (assenza pioggia)

**Algoritmo:**
- Gubler-Thomas (GT)
- Modello di accumulo del rischio (score settimanale)

**Output:**
- Livello di rischio cumulativo
- Finestra ottimale per intervento

### 3.3 Botrite

**Descrizione:**
- Più probabile in condizioni di umidità elevata (>90%) e dopo la pioggia
- Particolarmente critica in fase di invaiatura
- Il microclima chiuso e danni meccanici aumentano il rischio

**Dati richiesti:**
- Umidità relativa > 90%
- Eventi di pioggia recente
- Stato fenologico (invaiatura/post-pioggia)

**Algoritmo:**
- Modello empirico con condizioni soglia
- Supporto AI (classificatore binario con Random Forest)

**Output:**
- Allerta di rischio infezione
- Suggerimento di trattamento preventivo


---

## 4. 📷 Modelli AI (Fasi Avanzate)

**Scopo:** Supporto visivo per la maturazione, la diagnosi e il monitoraggio

### 4.1 Riconoscimento visivo da campo

**Descrizione:**
- Classificazione dello stadio fenologico tramite immagini
- Rilevamento precoce di malattie visibili
- Conteggio automatico dei grappoli o acini da immagini (drone o operatore)

**Dati richiesti:**
- Immagini RGB o multispettrali di grappoli, foglie
- Stato fenologico manuale per training

**Algoritmo:**
- CNN (ResNet, EfficientNet) + Transfer Learning
- Segmentazione e classificazione

**Output:**
- Classificazione dello stadio fenologico
- Rilevamento precoce di malattie

### 4.2 Previsione della vendemmia ottimale

**Descrizione:**
- Incrocia dati climatici, osservazioni sul campo e analisi visive
- Restituisce una finestra temporale ottimale per la raccolta su base microzonale

**Dati richiesti:**
- GDD accumulati
- Valori manuali (°Brix, pH, acidità)
- Immagini da campo (droni o manuali)

**Algoritmo:**
- Regressione multipla o Random Forest
- Ottimizzazione finestra raccolta

**Output:**
- Intervallo temporale per vendemmia ottimale
- Raccomandazioni microzonali

### 4.3 Stima zuccherina non distruttiva

**Descrizione:**
- Utilizzo di sensori iperspettrali combinati con algoritmi di machine learning
- Predizione del contenuto zuccherino (°Brix) senza campionamento diretto

**Dati richiesti:**
- Spettri iperspettrali (400–1000 nm)
- Immagini foglie o acini

**Algoritmo:**
- PLSR, SVR o Random Forest
- Data preprocessing (PCA, normalizzazione)

**Output:**
- Valore °Brix stimato
- Mappa zuccherina in vigna

### 4.4 Riconoscimento varietale

**Descrizione:**
- Riconoscimento automatico del vitigno tramite immagini di foglie e grappoli
- Elevata accuratezza grazie a reti neurali convoluzionali

**Dati richiesti:**
- Immagini foglie, grappoli, semi

**Algoritmo:**
- CNN + classificazione multiclasse

**Output:**
- Identificazione automatica del vitigno
- Verifica varietale nelle parcelle

### 4.5 Fenotipizzazione avanzata

**Descrizione:**
- Identificazione dello stress della pianta (es. tossicità rameica)
- Calcolo di indici vegetativi e fisiologici da immagini multispettrali

**Dati richiesti:**
- Immagini multispettrali (da drone o campo)
- Indici vegetativi: NDVI, GNDVI, CCCI, PRI, ecc.
- Parametri agronomici di riferimento (se disponibili)

**Algoritmo:**
- Estrazione automatica degli indici con librerie GIS o Python (`rasterio`, `OpenCV`, `scikit-image`)
- Clustering non supervisionato (K-means, DBSCAN) per individuare zone critiche
- Classificatori supervisati per riconoscimento stress (SVM, RF, CNN)

**Output:**
- Mappa di stress localizzato
- Classificazione dello stato vegetativo
- Segnalazioni preventive su anomalie fisiologiche

### 4.6 Rilevamento malattie con AI avanzata

**Descrizione:**
- Algoritmi di nuova generazione (Transformer) applicati a dati multisorgente
- Predizione localizzata a livello di pixel o parcella

**Implementazione:**
**Dati richiesti:**
- Dataset etichettato di immagini RGB/multispettrali da campo o drone
- Metadati ambientali: temperatura, umidità, eventi meteo
- Informazioni storiche di infezioni note

**Algoritmo:**
- Vision Transformer (ViT), Swin Transformer o TabPFN per classificazione e segmentazione
- Segmentazione semantica con reti deep learning (es. UNet + attention)
- Fusion sensoriale: integrazione immagini + dati meteo per predizione contestuale

**Output:**
- Mappa predittiva del rischio malattie
- Localizzazione precisa delle infezioni (pixel/parcella)
- Classificazione delle malattie per tipo e intensità

---

## 5. 🧪 Modelli Agrometeorologici & GIS

**Scopo:** Simulazioni dinamiche e mappatura spaziale

**Descrizione:**
- Utilizzo di modelli fisici per simulare lo sviluppo della vite e i suoi bisogni
- Integrazione con dati da GIS e da droni/satelliti per creare mappe microclimatiche
- Permette previsioni di crescita e maturazione su base territoriale

**Dati richiesti:**
- Dati climatici storici
- Dati suolo, topografia
- Immagini satellitari o da drone

**Algoritmo:**
- Modelli dinamici (es. STICS, WOFOST)
- GIS per interpolazione spaziale

**Output:**
- Simulazioni crescita
- Mappe di stress e sviluppo per parcella

---

## 6. 🧠 Modelli Personalizzati e UX

**Parametri personalizzabili:**

- Vitigno
- Epoca di potatura
- Altitudine
- Tipo di suolo
- Presenza o assenza di irrigazione

### Visualizzazione:

| Modello | Interfaccia di visualizzazione|
|-----------------------|---------------------------------------------|
| GDD | Grafico lineare con soglie evidenziate|
| ET₀ / Bilancio idrico | Istogramma + mappa cromatica|
| Malattie| Semaforo rischio (verde/giallo/rosso) |
| AI visivo | Galleria immagini con diagnosi automatica |
| Vendemmia | Timeline con finestre consigliate |

---

## 7. 🧩 Modelli Futuri (Opzionali)

**Descrizione:**
- Calcolo dello stress termico per il grappolo (es. scottature)
- Stima dell’indice di vigore tramite immagini NDVI da drone
- Curve di maturazione zuccherina e fenolica
- Predizione della resa e qualità tramite modelli ML
- AutoML e modelli adattivi in funzione del contesto
- Ottimizzazione dei trattamenti fitosanitari su base predittiva
- Integrazione di AI e remote sensing per una gestione più sostenibile

---

## 8. 💡 Sostenibilità e Intelligenza Artificiale

**Descrizione:**
- Uso dell’AI per ridurre trattamenti chimici e migliorare efficienza
- Droni e UAV abilitano la viticoltura di precisione con visione multispettrale
- Supporto decisionale automatizzato per agronomi e viticoltori

---


