#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "TuSSID";
const char* password = "TuContraseña";

AsyncWebServer server(80);

int contador = 0;

// Función para insertar variables en la plantilla HTML
String processor(const String& var) {
    if (var == "CONTADOR") {
        return String(contador);
    }
    return String();
}

// Página HTML con marcador %CONTADOR%
const char html_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head><title>ESP32 Web Server</title></head>
<body>
    <h1>ESP32 Servidor Web</h1>
    <p>Valor del contador: %CONTADOR%</p>
    <button onclick="window.location.href='/incrementar'">Incrementar</button>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConectado!");

    // Servir la página HTML y reemplazar variables
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", html_page, processor);
    });

    // Ruta para incrementar el contador
    server.on("/incrementar", HTTP_GET, [](AsyncWebServerRequest *request){
        contador++;
        request->redirect("/");
    });

    server.begin();
}

void loop() {}