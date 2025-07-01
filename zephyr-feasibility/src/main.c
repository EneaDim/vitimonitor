/*
 * Zephyr + QEMU + Zbus + MQTT - Mock Sensor Project (con commenti dettagliati)
 * FILES:
 * - prj.conf: configurazione delle feature Zephyr
 * - CMakeLists.txt: definizione progetto
 * - riscv.overlay / cortexm.overlay: override dei device
 */

#include <zephyr/kernel.h>              // Include il kernel Zephyr (thread, sleep, ecc.)
#include <zephyr/device.h>              // Accesso a dispositivi hardware
#include <zephyr/devicetree.h>          // Parsing del device tree
#include <zephyr/drivers/sensor.h>      // API generica per sensori
#include <zephyr/logging/log.h>         // Logging di sistema
#include <zephyr/net/mqtt.h>            // API MQTT
#include <zephyr/net/socket.h>          // Sockets per rete
#include <zephyr/net/net_config.h>      // Configurazione rete
#include <zephyr/net/net_if.h>          // Interfacce di rete
#include <zephyr/zbus/zbus.h>           // Inclusione del message bus Zbus

LOG_MODULE_REGISTER(main);              // Abilita il logging per questo modulo (nome: main)

/* ========== DEFINIZIONE DEL CANALE ZBUS ========== */

// Definizione della struttura per i dati dei sensori
typedef struct {
    int temp;                           // Temperatura
    int humidity;                       // Umidità
} sensor_data_t;

// Creazione del canale Zbus che trasporterà struct sensor_data_t
ZBUS_CHAN_DEFINE(sensor_data_chan,      // Nome del canale
                 sensor_data_t,         // Tipo di dato trasportato
                 NULL, NULL,            // Callback opzionali (non usate qui)
                 ZBUS_OBSERVERS_EMPTY,  // Nessun observer statico
                 ZBUS_MSG_INIT(.temp = 0, .humidity = 0)); // Valori iniziali

/* ========== THREAD DI PRODUZIONE DATI (MOCK SENSOR) ========== */

// Funzione che rappresenta un sensore fittizio
void mock_sensor_thread(void)
{
    while (1) {
        // Genera valori casuali simulati
        sensor_data_t data = {
            .temp = 22 + (sys_rand32_get() % 5),       // Temp tra 22 e 26
            .humidity = 40 + (sys_rand32_get() % 10)   // Umidità tra 40 e 49
        };

        // Pubblica i dati sul canale Zbus
        zbus_chan_pub(&sensor_data_chan, &data, K_MSEC(100));

        // Attende 2 secondi prima di generare nuovi dati
        k_sleep(K_SECONDS(2));
    }
}

// Creazione di un thread che esegue mock_sensor_thread
K_THREAD_DEFINE(mock_sensor_tid,        // Nome interno del thread
                1024,                   // Dimensione dello stack
                mock_sensor_thread,     // Funzione da eseguire
                NULL, NULL, NULL,       // Parametri (non usati)
                5,                      // Priorità del thread
                0,                      // Opzioni
                0);                     // Avvio immediato

/* ========== THREAD DI PUBBLICAZIONE MQTT ========== */

// Definizione costanti per la connessione MQTT
#define MQTT_CLIENTID "zephyr_sensor"   // Nome client identificativo
#define MQTT_BROKER_ADDR "192.0.2.1"    // IP del broker (sostituire con reale)
#define MQTT_PORT 1883                  // Porta default MQTT
#define MQTT_TOPIC "sensor/data"        // Topic a cui pubblicare

// Client MQTT e indirizzo del broker
static struct mqtt_client client;
static struct sockaddr_storage broker;

// Callback chiamata quando ci sono eventi MQTT (es. connesso/disconnesso)
static void mqtt_event_handler(struct mqtt_client *const c, const struct mqtt_evt *evt) {
    ARG_UNUSED(c);                      // Ignora parametro non usato
    switch (evt->type) {
        case MQTT_EVT_CONNACK:
            LOG_INF("MQTT connected"); // Log: connesso
            break;
        case MQTT_EVT_DISCONNECT:
            LOG_WRN("MQTT disconnected"); // Log: disconnesso
            break;
        default:
            break;                      // Altri eventi non gestiti
    }
}

// Funzione per iniziare la connessione MQTT
static int mqtt_connect(void)
{
    struct sockaddr_in *broker4 = (struct sockaddr_in *)&broker;  // Interpreta broker come IPv4
    broker4->sin_family = AF_INET;                                 // Famiglia indirizzo
    broker4->sin_port = htons(MQTT_PORT);                          // Porta del broker
    inet_pton(AF_INET, MQTT_BROKER_ADDR, &broker4->sin_addr);     // Converte stringa IP in binario

    mqtt_client_init(&client);                                     // Inizializza client MQTT

    client.broker = &broker;                                       // Imposta indirizzo del broker
    client.evt_cb = mqtt_event_handler;                            // Imposta callback eventi
    client.client_id.utf8 = (uint8_t *)MQTT_CLIENTID;              // ID client come stringa UTF-8
    client.client_id.size = strlen(MQTT_CLIENTID);                 // Lunghezza dell'ID
    client.protocol_version = MQTT_VERSION_3_1_1;                  // Versione protocollo MQTT
    client.transport.type = MQTT_TRANSPORT_NON_SECURE;             // Connessione non sicura (no TLS)

    return mqtt_connect(&client);                                  // Avvia la connessione
}

// Thread che ascolta su Zbus e invia i dati via MQTT
void mqtt_publisher_thread(void)
{
    sensor_data_t received;                // Buffer per ricevere dati dal canale

    if (mqtt_connect() != 0) {             // Prova a connettersi al broker MQTT
        LOG_ERR("MQTT connection failed");
        return;                            // Termina se fallisce
    }

    while (1) {
        zbus_chan_sub_wait(&sensor_data_chan, &received, K_FOREVER); // Attende nuovi dati dal canale

        // Crea payload JSON da inviare
        char payload[64];
        snprintf(payload, sizeof(payload), "{\"temp\":%d,\"humidity\":%d}",
                 received.temp, received.humidity);

        // Parametri della pubblicazione MQTT
        struct mqtt_publish_param param = {
            .message.topic.qos = MQTT_QOS_0_AT_MOST_ONCE,              // Nessuna conferma
            .message.topic.topic.utf8 = (uint8_t *)MQTT_TOPIC,         // Topic
            .message.topic.topic.size = strlen(MQTT_TOPIC),            // Lunghezza topic
            .message.payload.data = payload,                           // Dati JSON da inviare
            .message.payload.len = strlen(payload),                    // Lunghezza dati
            .message_id = sys_rand32_get(),                            // ID casuale del messaggio
            .dup_flag = 0,
            .retain_flag = 0
        };

        mqtt_publish(&client, &param);            // Invia i dati al broker
        k_sleep(K_SECONDS(2));                    // Aspetta prima di inviare il prossimo
    }
}

// Creazione thread per pubblicazione MQTT
K_THREAD_DEFINE(mqtt_pub_tid,             // Nome interno del thread
                2048,                    // Stack più grande per rete
                mqtt_publisher_thread,   // Funzione da eseguire
                NULL, NULL, NULL,        // Parametri
                5,                       // Priorità
                0,                       // Opzioni
                0);

