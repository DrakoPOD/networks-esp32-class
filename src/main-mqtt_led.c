#include <stdio.h>

#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include "nvs_flash.h"

#define LED_PIN GPIO_NUM_2
#define MQTT_BROKER_URI "mqtt://rpiee.local:1883"
#define WIFI_SSID "IoT_EE"
#define WIFI_PASS "12345678"

#define TOPIC "esp32/led"

static const char *TAG_MQTT = "MQTT";
static const char *TAG_WIFI = "WIFI";

static void print_user_property(mqtt5_user_property_handle_t user_property) {
    if (user_property) {
        uint8_t count = esp_mqtt5_client_get_user_property_count(user_property);
        if (count) {
            esp_mqtt5_user_property_item_t *item = malloc(count * sizeof(esp_mqtt5_user_property_item_t));
            if (esp_mqtt5_client_get_user_property(user_property, item, &count) == ESP_OK) {
                for (int i = 0; i < count; i++) {
                    ESP_LOGI(TAG_MQTT, "Propiedad: %s = %s", item[i].key, item[i].value);
                    free((char *)item[i].key);
                    free((char *)item[i].value);
                }
            }
            free(item);
        }
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG_MQTT, "Conectado al broker MQTT");
            esp_mqtt_client_subscribe(client, TOPIC, 0);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG_MQTT, "Mensaje recibido en %.*s: %.*s", event->topic_len, event->topic, event->data_len, event->data);

            // Leer propiedades de usuario
            ESP_LOGI(TAG_MQTT, "Mensaje recibido en el topic: %.*s", event->topic_len, event->topic);
            print_user_property(event->property->user_property);

            // Encender o apagar LED
            if (strncmp(event->data, "ON", event->data_len) == 0) {
                gpio_set_level(LED_PIN, 1);
                ESP_LOGI(TAG_MQTT, "LED ENCENDIDO");
            } else if (strncmp(event->data, "OFF", event->data_len) == 0) {
                gpio_set_level(LED_PIN, 0);
                ESP_LOGI(TAG_MQTT, "LED APAGADO");
            }
            break;

        default:
            break;
    }
}
static void reconnect_wifi(void *arg) {
    ESP_LOGI(TAG_WIFI, "Intentando reconectar... %s", WIFI_SSID);
    esp_wifi_connect();
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG_WIFI, "Conectado a Wi-Fi");
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG_WIFI, "Conectado con IP: " IPSTR, IP2STR(&event->ip_info.ip));
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
        ESP_LOGW(TAG_WIFI, "Desconectado del Wi-Fi. Razón: %d", event->reason);
        // Diagnóstico del error
        switch (event->reason) {
            case WIFI_REASON_AUTH_EXPIRE:
                ESP_LOGE(TAG_WIFI, "Error: La autenticación expiró");
                break;
            case WIFI_REASON_AUTH_FAIL:
                ESP_LOGE(TAG_WIFI, "Error: Fallo de autenticación (credenciales incorrectas)");
                break;
            case WIFI_REASON_NO_AP_FOUND:
                ESP_LOGE(TAG_WIFI, "Error: No se encontró el punto de acceso (SSID incorrecto o fuera de rango)");
                break;
            case WIFI_REASON_HANDSHAKE_TIMEOUT:
                ESP_LOGE(TAG_WIFI, "Error: Timeout en el handshake (posible problema de compatibilidad con el router)");
                break;
            case WIFI_REASON_BEACON_TIMEOUT:
                ESP_LOGE(TAG_WIFI, "Error: No se reciben beacons del router (señal muy débil)");
                break;
            case WIFI_REASON_ASSOC_LEAVE:
                ESP_LOGE(TAG_WIFI, "Error: Desconectado manualmente por otro dispositivo");
                break;
            default:
                ESP_LOGW(TAG_WIFI, "Error desconocido: %d", event->reason);
                break;
        }

        // Intentar reconectar
        esp_timer_handle_t reconnect_timer;
        esp_timer_create_args_t timer_args = {.callback = &reconnect_wifi, .arg = NULL, .name = "reconnect_timer"};
        esp_timer_create(&timer_args, &reconnect_timer);
        esp_timer_start_once(reconnect_timer, 5000000);  // 5 segundos
    }
}

void wifi_init() {
    nvs_flash_init();
    esp_netif_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta =
            {
                .ssid = WIFI_SSID,
                .password = WIFI_PASS,
            },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

void app_main() {
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    esp_log_level_set("wifi", ESP_LOG_DEBUG);

    wifi_init();  // 🔹 Primero conectamos a Wi-Fi

    // 🔹 Configuramos MQTT
    esp_mqtt_client_config_t mqtt_cfg = {.broker.address.uri = MQTT_BROKER_URI, .session.protocol_ver = MQTT_PROTOCOL_V_5};

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}