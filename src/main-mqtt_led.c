#include <ctype.h>
#include <stdio.h>

#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "mqtt_client.h"
#include "nvs_flash.h"

#define LED_PIN GPIO_NUM_2
#define MQTT_BROKER_URI "mqtt://rpiee.local:1883"
#define WIFI_SSID "IoT_EE"
#define WIFI_PASS "12345678"

#define TOPIC_R "esp32/led/R"
#define TOPIC_G "esp32/led/G"
#define TOPIC_B "esp32/led/B"

#define LED_STRIP_USE_DMA 0

#if LED_STRIP_USE_DMA
// Numbers of the LED in the strip
#define LED_STRIP_LED_COUNT 256
#define LED_STRIP_MEMORY_BLOCK_WORDS 1024  // this determines the DMA block size
#else
// Numbers of the LED in the strip
#define LED_STRIP_LED_COUNT 1
#define LED_STRIP_MEMORY_BLOCK_WORDS 0  // let the driver choose a proper memory block size automatically
#endif                                  // LED_STRIP_USE_DMA

// GPIO assignment
#define LED_STRIP_GPIO_PIN 48

// 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define LED_STRIP_RMT_RES_HZ (10 * 1000 * 1000)

static const char *TAG_MQTT = "MQTT";
static const char *TAG_WIFI = "WIFI";
static const char *TAG_LED = "LED";

led_strip_handle_t led_strip;
led_strip_handle_t configure_led(void) {
    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config =
        {.strip_gpio_num = LED_STRIP_GPIO_PIN,  // The GPIO that connected to the LED strip's data line
         .max_leds = LED_STRIP_LED_COUNT,       // The number of LEDs in the strip
         .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
         .led_model = LED_MODEL_WS2812,  // LED strip model
         .flags = {
             .invert_out = false,  // don't invert the output signal
         }};

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config =
        {.clk_src = RMT_CLK_SRC_DEFAULT,         // different clock source can lead to different power consumption
         .resolution_hz = LED_STRIP_RMT_RES_HZ,  // RMT counter clock frequency
         .flags = {
             .with_dma = LED_STRIP_USE_DMA,  // Using DMA can improve performance when driving more LEDs
         }};

    // LED Strip object handle
    led_strip_handle_t led_strip;
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG_LED, "Created LED strip object with RMT backend");
    return led_strip;
}

// Función para verificar si el mensaje es un número
int is_number(const char *str, int len) {
    if (str == NULL || len == 0) return 0;  // Si es nulo o vacío, no es número

    int i = 0;
    if (str[0] == '-') i++;  // Permitir signo negativo

    for (; i < len; i++) {
        if (!isdigit((unsigned char)str[i])) return 0;
    }
    return 1;
}

int clamp(int value, int min, int max) { return (value < min) ? min : (value > max) ? max : value; }

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

uint8_t r = 0;
uint8_t g = 0;
uint8_t b = 0;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG_MQTT, "Conectado al broker MQTT");
            esp_mqtt_client_subscribe(client, TOPIC_R, 1);
            esp_mqtt_client_subscribe(client, TOPIC_G, 1);
            esp_mqtt_client_subscribe(client, TOPIC_B, 1);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG_MQTT, "Mensaje recibido en %.*s: %.*s", event->topic_len, event->topic, event->data_len, event->data);

            // Leer propiedades de usuario
            ESP_LOGI(TAG_MQTT, "Mensaje recibido en el topic: %.*s", event->topic_len, event->topic);
            print_user_property(event->property->user_property);

            if (is_number(event->data, event->data_len)) {
                uint8_t value = strtol(event->data, NULL, 10);

                value = clamp(value, 0, 255);

                if (strncmp(event->topic, TOPIC_R, event->topic_len) == 0) {
                    r = value;
                } else if (strncmp(event->topic, TOPIC_G, event->topic_len) == 0) {
                    g = value;
                } else if (strncmp(event->topic, TOPIC_B, event->topic_len) == 0) {
                    b = value;
                }

                led_strip_set_pixel(led_strip, 0, r, g, b);
                led_strip_refresh(led_strip);

                ESP_LOGI(TAG_LED, "Led color have change");
            } else {
                ESP_LOGE(TAG_MQTT, "El mensaje no es un número");
            }
            // Encender o apagar LED
            // if (strncmp(event->data, "ON", event->data_len) == 0) {
            //     gpio_set_level(LED_PIN, 1);
            //     ESP_LOGI(TAG_MQTT, "LED ENCENDIDO");
            // } else if (strncmp(event->data, "OFF", event->data_len) == 0) {
            //     gpio_set_level(LED_PIN, 0);
            //     ESP_LOGI(TAG_MQTT, "LED APAGADO");
            // }
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
/// asdsad

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
    led_strip = configure_led();
    led_strip_set_pixel(led_strip, 0, 0, 0, 0);
    led_strip_refresh(led_strip);

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