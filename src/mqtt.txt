#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

// Configuración de red WiFi
const char* ssid = "TU_SSID";
const char* password = "TU_CONTRASEÑA";

// Configuración del broker MQTT
const char* mqtt_server = "broker.hivemq.com";  // Puedes cambiarlo
const int mqtt_port = 1883;
const char* mqtt_topic = "test/esp32";

WiFiClient espClient;
PubSubClient client(espClient);

// Función para manejar los mensajes recibidos
void callback(char* topic, byte* message, unsigned int length) {
    Serial.print("Mensaje recibido en tópico: ");
    Serial.println(topic);

    String payload;
    for (int i = 0; i < length; i++) {
        payload += (char)message[i];
    }
    Serial.println("Mensaje recibido: " + payload);

    // Parsear el JSON recibido
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
        // Verificar si "tipo" es una cadena de texto
        if (doc["tipo"].is<const char*>()) {
            const char* tipo = doc["tipo"];
            Serial.print("Tipo: ");
            Serial.println(tipo);
        } else {
            Serial.println("Error: 'tipo' no es una cadena de texto válida");
        }

        // Verificar si "valor" es un número entero
        if (doc["valor"].is<int>()) {
            int valor = doc["valor"];
            Serial.print("Valor: ");
            Serial.println(valor);
        } else {
            Serial.println("Error: 'valor' no es un número entero válido");
        }
    } else {
        Serial.println("Error al parsear JSON");
    }
}

// Conectar a la red WiFi
void setup_wifi() {
    Serial.print("Conectando a ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConectado a WiFi");
}

// Reconectar al broker MQTT en caso de desconexión
void reconnect() {
    while (!client.connected()) {
        Serial.print("Intentando conexión MQTT...");
        if (client.connect("ESP32Client")) {
            Serial.println("Conectado al broker MQTT");
            client.subscribe(mqtt_topic);  // Suscribirse al tópico
        } else {
            Serial.print("Error, rc=");
            Serial.print(client.state());
            Serial.println(" Intentando de nuevo en 5 segundos...");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // Publicar un JSON cada 5 segundos
    static unsigned long lastMsg = 0;
    if (millis() - lastMsg > 5000) {
        lastMsg = millis();

        // Crear JSON
        StaticJsonDocument<200> doc;
        doc["tipo"] = "sensor";
        doc["valor"] = random(0, 100);  // Simulación de un dato de sensor

        char buffer[256];
        serializeJson(doc, buffer);

        Serial.print("Publicando JSON: ");
        Serial.println(buffer);
        client.publish(mqtt_topic, buffer);
    }
}
