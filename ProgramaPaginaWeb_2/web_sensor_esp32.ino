#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_PD";
const char* password = "123456789";

// Crear una instancia del servidor web
WebServer server(80);

// Definir los pines de entrada
const int pinAutoInput = 19;      // Pin para la entrada Auto (GPIO19)
const int pinRangeInput = 5;      // Pin para la entrada de rango (uA/mA) (GPIO5)
const int inputPin = 34;          // Pin 34 como entrada de señal analógica

// Variables para almacenar estadísticas
float minValue = 99999.0;
float maxValue = -99999.0;
float sumValues = 0;
int count = 0;

// Función para leer la corriente sin conversión
float readCurrent() {
  int sensorValue = analogRead(inputPin);
  float voltage = (sensorValue / 4095.0) * 3300.0;  // Convertir a milivoltios (0-3300mV)
  return voltage;
}

// Función para la página principal
void handleRoot() {
  // Leer los estados lógicos de las entradas
  bool isAuto = digitalRead(pinAutoInput);
  bool isMilliAmp = digitalRead(pinRangeInput);  // HIGH = mA, LOW = uA
  
  // Definir la unidad de medida dependiendo del estado del pin de rango
  String displayUnit = "";
  if (isAuto) {
    displayUnit = "Auto";
  } else if (isMilliAmp) {
    displayUnit = "mA";
  } else {
    displayUnit = "µA";
  }
  
  float currentValue = readCurrent();  // Leer la corriente
  
  // Actualizar estadísticas
  minValue = min(minValue, currentValue);
  maxValue = max(maxValue, currentValue);
  sumValues += currentValue;
  count++;
  float avgValue = sumValues / count;

  // Crear el contenido HTML
  String html = "<html><head>";
  html += "<meta charset='UTF-8'>";  // Añadir la codificación UTF-8
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'/>";
  html += "<title>Monitor de Corriente ESP32</title>";
  html += "<link href='https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;500;700&display=swap' rel='stylesheet'/>";
  html += "<style>";
  html += "body { font-family: 'Roboto', sans-serif; background-color: #f5f7fa; padding: 20px; color: #2c3e50; }";
  html += ".container { max-width: 800px; margin: auto; }";
  html += "h1 { text-align: center; color: #3498db; margin-bottom: 20px; }";
  html += ".btn { padding: 10px 20px; border-radius: 5px; margin: 5px; cursor: pointer; border: 2px solid #3498db; color: #3498db; background: transparent; font-weight: bold; }";
  html += ".btn-active { background-color: #3498db; color: white; }";
  html += ".card { background: white; border-radius: 10px; padding: 20px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); margin-bottom: 20px; }";
  html += ".value { font-size: 2.5rem; text-align: center; }";
  html += ".unit { font-size: 1rem; color: #7f8c8d; }";
  html += ".stats { font-size: 1.1rem; margin-top: 10px; }";
  html += ".stats div { display: flex; justify-content: space-between; margin: 5px 0; }";
  html += "</style>";

  html += "<script>";
  html += "function updateDisplay() {";
  html += "  document.getElementById('currentValue').innerText = '" + String(currentValue, 2) + " " + displayUnit + "';";
  html += "  document.getElementById('minValue').innerText = '" + String(minValue, 2) + " " + displayUnit + "';";
  html += "  document.getElementById('maxValue').innerText = '" + String(maxValue, 2) + " " + displayUnit + "';";
  html += "  document.getElementById('avgValue').innerText = '" + String(avgValue, 2) + " " + displayUnit + "';";
  html += "}";
  html += "setInterval(updateDisplay, 1000);";  // Actualizar la visualización cada segundo
  html += "</script>";

  html += "</head><body>";
  
  html += "<div class='container'>";
  html += "<h1>Monitor de Corriente ESP32</h1>";

  // Selector de medición (Auto, mA, µA)
  html += "<div class='card'><div style='text-align:center;'>";
  html += "<button class='btn " + String(isAuto ? "btn-active" : "") + "'>Auto</button>";
  html += "<button class='btn " + String(isMilliAmp ? "btn-active" : "") + "'>mA</button>";
  html += "<button class='btn " + String(!isMilliAmp && !isAuto ? "btn-active" : "") + "'>µA</button>";
  html += "</div></div>";

  // Mostrar el valor de la corriente actual
  html += "<div class='card'><h2>Corriente Actual</h2><div class='value' id='currentValue'>--</div></div>";

  // Estadísticas de la corriente
  html += "<div class='card'><h2>Estadísticas</h2><div class='stats'>";
  html += "<div><span>Mínimo:</span><span id='minValue'>--</span></div>";
  html += "<div><span>Máximo:</span><span id='maxValue'>--</span></div>";
  html += "<div><span>Promedio:</span><span id='avgValue'>--</span></div>";
  html += "</div></div>";

  html += "</div>";  // Fin de la container
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  pinMode(inputPin, INPUT);
  pinMode(pinAutoInput, INPUT);
  pinMode(pinRangeInput, INPUT);

  WiFi.softAP(ssid, password);
  Serial.println("Punto de acceso iniciado");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}
