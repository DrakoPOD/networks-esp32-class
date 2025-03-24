#include <stdio.h>

#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include "nvs_flash.h"

#define LED_PIN GPIO_NUM_2
#define MQTT_BROKER_URI "mqtt://test.mosquitto.org"
#define WIFI_SSID "TuSSID"
#define WIFI_PASS "TuPassword"

static const char *TAG_MQTT = "MQTT";
static const char *TAG_WIFI = "WIFI";

void app_main() {}