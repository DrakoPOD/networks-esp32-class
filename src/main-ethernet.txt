#include <EthernetENC.h>
#include <SPI.h>

// Definir los pines SPI en el ESP32-S3
#define ENC28J60_CS 5  // Chip Select (CS) del ENC28J60

// Dirección MAC (puedes cambiarla si tienes varias placas)
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// Objeto Ethernet
EthernetClient client;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    // Iniciar SPI con los pines correctos en el ESP32-S3
    SPI.begin(12, 13, 11, ENC28J60_CS);

    Serial.println("Iniciando ENC28J60...");

    // Inicializar Ethernet con DHCP
    if (Ethernet.begin(mac) == 0) {
        Serial.println("Fallo al obtener dirección IP por DHCP.");

        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
            while (true) {
                delay(1);  // do nothing, no point running without Ethernet hardware
            }
        }
    } else {
        Serial.print("Dirección IP asignada: ");
        Serial.println(Ethernet.localIP());
    }
}

void loop() {
    // Verificar estado de conexión
    Serial.print("IP: ");
    Serial.println(Ethernet.localIP());
    delay(5000);
}