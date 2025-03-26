#include <stdio.h>

#include "esp_chip_info.h"
#include "esp_flash.h"

void app_main(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    printf("Modelo de chip: %d\n", chip_info.model);
    printf("Revisión: %d\n", chip_info.revision);
    printf("Cores: %d\n", chip_info.cores);
    printf("Características: %s%s%s\n", (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "Wi-Fi " : "", (chip_info.features & CHIP_FEATURE_BLE) ? "BLE " : "", (chip_info.features & CHIP_FEATURE_BT) ? "Bluetooth " : "");

    uint32_t flash_size;
    if (esp_flash_get_size(NULL, &flash_size) == ESP_OK) {
        printf("Tamaño de Flash: %luMB\n", flash_size / (1024 * 1024));
    } else {
        printf("Error obteniendo tamaño de Flash\n");
    }
}