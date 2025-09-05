#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SparkFun_SCD30_Arduino_Library.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <vector>
#include <TimeLib.h>
#include <LittleFS.h>
#include <FS.h>


// ==========================
// Configuración WiFi AP
// ==========================
const char* ssid = "ESP32_SCD30";
const char* password = "12345678";


// Servidores
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);


// Pines
#define MOTOR_PIN 18
#define ENCODER_CLK 34
#define ENCODER_DT 35
#define ENCODER_SW 32


// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


// Encoder
volatile int encoderValue = 0;
int lastEncoded = 0;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;


// PWM
int pwmValue = 0;


// SCD30
SCD30 airSensor;
float lastCO2 = 0, lastTemp = 0, lastHum = 0;
bool sensorDataValid = false;
bool scd30Available = false;
unsigned long lastSensorRead = 0;
const unsigned long sensorReadInterval = 2000; // Leer cada 2 segundos


// Modo
enum DisplayMode {
  PWM_MODE,
  SENSOR_MODE
};
DisplayMode currentMode = PWM_MODE;


// Control de tiempo para WebSocket
unsigned long lastWebSocketUpdate = 0;
const unsigned long wsUpdateInterval = 1000;


// ==========================
// Data Logging para Excel
// ==========================
struct DataRecord {
  unsigned long timestamp;
  float co2;
  float temperature;
  float humidity;
  int pwm;
};


std::vector<DataRecord> dataLog;
const size_t MAX_DATA_RECORDS = 500;
unsigned long lastDataLog = 0;
const unsigned long dataLogInterval = 5000;
unsigned long systemStartTime = 0;


// ==========================
// Funciones para LittleFS
// ==========================

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".json")) return "application/json";
  return "text/plain";
}


void handleFileRead(String path) {
  if (path.endsWith("/")) {
    path += "index.html";
  }
 
  String contentType = getContentType(path);
 
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    Serial.println("File served: " + path);
  } else {
    server.send(404, "text/plain", "File not found");
    Serial.println("File not found: " + path);
  }
}


// ==========================
// Funciones de logging
// ==========================

String getFormattedTime(unsigned long timestamp) {
  unsigned long seconds = timestamp / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
 
  seconds = seconds % 60;
  minutes = minutes % 60;
  hours = hours % 24;
 
  char buffer[20];
  sprintf(buffer, "%02lu:%02lu:%02lu:%02lu", days, hours, minutes, seconds);
  return String(buffer);
}


void logData() {
  if (!sensorDataValid) return;
 
  DataRecord record;
  record.timestamp = millis() - systemStartTime;
  record.co2 = lastCO2;
  record.temperature = lastTemp;
  record.humidity = lastHum;
  record.pwm = pwmValue;
 
  dataLog.push_back(record);
 
  if (dataLog.size() > MAX_DATA_RECORDS) {
    dataLog.erase(dataLog.begin());
  }
 
  Serial.printf("Data logged: CO2=%.0f, Temp=%.1f, Hum=%.1f, PWM=%d\n",
                record.co2, record.temperature, record.humidity, record.pwm);
}


String generateCSV() {
  String csv = "Timestamp,CO2 (ppm),Temperature (C),Humidity (%),PWM Value\n";
 
  for (const auto& record : dataLog) {
    csv += getFormattedTime(record.timestamp) + ",";
    csv += String(record.co2, 0) + ",";
    csv += String(record.temperature, 1) + ",";
    csv += String(record.humidity, 1) + ",";
    csv += String(record.pwm) + "\n";
  }
 
  return csv;
}


// ==========================
// Handlers del servidor web
// ==========================

void handleDownloadCSV() {
  String csv = generateCSV();
 
  server.sendHeader("Content-Type", "text/csv");
  server.sendHeader("Content-Disposition", "attachment; filename=\"scd30_data.csv\"");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/csv", csv);
 
  Serial.println("CSV file downloaded");
}


void handleStats() {
  StaticJsonDocument<512> doc;
  doc["totalRecords"] = dataLog.size();
  doc["maxRecords"] = MAX_DATA_RECORDS;
  doc["logInterval"] = dataLogInterval / 1000;
  doc["uptime"] = millis() / 1000;
 
  if (!dataLog.empty()) {
    float avgCO2 = 0, avgTemp = 0, avgHum = 0, avgPWM = 0;
    float minCO2 = dataLog[0].co2, maxCO2 = dataLog[0].co2;
    float minTemp = dataLog[0].temperature, maxTemp = dataLog[0].temperature;
   
    for (const auto& record : dataLog) {
      avgCO2 += record.co2;
      avgTemp += record.temperature;
      avgHum += record.humidity;
      avgPWM += record.pwm;
     
      if (record.co2 < minCO2) minCO2 = record.co2;
      if (record.co2 > maxCO2) maxCO2 = record.co2;
      if (record.temperature < minTemp) minTemp = record.temperature;
      if (record.temperature > maxTemp) maxTemp = record.temperature;
    }
   
    size_t count = dataLog.size();
    doc["avgCO2"] = avgCO2 / count;
    doc["avgTemp"] = avgTemp / count;
    doc["avgHum"] = avgHum / count;
    doc["avgPWM"] = avgPWM / count;
    doc["minCO2"] = minCO2;
    doc["maxCO2"] = maxCO2;
    doc["minTemp"] = minTemp;
    doc["maxTemp"] = maxTemp;
  }
 
  String jsonString;
  serializeJson(doc, jsonString);
  server.send(200, "application/json", jsonString);
}


void handleClearData() {
  dataLog.clear();
  server.send(200, "text/plain", "Data cleared successfully");
  Serial.println("Data log cleared");
}


void handleRoot() {
  handleFileRead("/index.html");
}


// ==========================
// Funciones del SCD30
// ==========================

void initSCD30() {
  Serial.println("Initializing SCD30...");
  
  // Intentar inicializar varias veces
  for (int attempt = 0; attempt < 3; attempt++) {
    if (airSensor.begin()) {
      scd30Available = true;
      Serial.println("SCD30 detected!");
      
      // Configurar el sensor
      airSensor.setMeasurementInterval(2); // Medición cada 2 segundos
      airSensor.setAltitudeCompensation(0); // Ajustar según tu altitud
      airSensor.setAmbientPressure(1013); // Presión en mbar
      
      // Verificar si hay calibración automática activa
      if (airSensor.getAutoSelfCalibration()) {
        Serial.println("Auto-calibration is ON");
      } else {
        Serial.println("Auto-calibration is OFF");
        // Opcionalmente activar auto-calibración
        // airSensor.setAutoSelfCalibration(true);
      }
      
      // Forzar una medición inicial
      delay(2000); // Esperar primera medición
      
      return;
    }
    
    Serial.printf("SCD30 init attempt %d failed\n", attempt + 1);
    delay(1000);
  }
  
  Serial.println("SCD30 not detected after 3 attempts!");
  scd30Available = false;
}

void readSCD30() {
  if (!scd30Available) return;
  
  // Intentar leer datos del sensor
  if (airSensor.dataAvailable()) {
    float co2 = airSensor.getCO2();
    float temp = airSensor.getTemperature();
    float hum = airSensor.getHumidity();
    
    // Validar que los datos sean razonables
    if (co2 > 0 && co2 < 40000 && temp > -40 && temp < 125 && hum >= 0 && hum <= 100) {
      lastCO2 = co2;
      lastTemp = temp;
      lastHum = hum;
      sensorDataValid = true;
      
      Serial.printf("SCD30: CO2=%.0f ppm, Temp=%.1f°C, Hum=%.1f%%\n", co2, temp, hum);
    } else {
      Serial.println("SCD30 data out of range!");
    }
  } else {
    // Si no hay datos disponibles, verificar que el sensor sigue respondiendo
    static unsigned long lastCheckTime = 0;
    if (millis() - lastCheckTime > 10000) { // Verificar cada 10 segundos
      lastCheckTime = millis();
      
      uint16_t interval = airSensor.getMeasurementInterval();
      if (interval == 0 || interval == 65535) {
        Serial.println("SCD30 not responding, reinitializing...");
        initSCD30();
      }
    }
  }
}


// ==========================
// Funciones principales
// ==========================

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
 
  // Mostrar estado WiFi
  display.print("WiFi AP: ");
  display.println(WiFi.softAPgetStationNum());
  display.print("IP: ");
  display.println(WiFi.softAPIP());
 
  if (currentMode == PWM_MODE) {
    display.print("PWM: ");
    display.println(pwmValue);
    display.print("Logs: ");
    display.println(dataLog.size());
    display.print("SCD30: ");
    display.println(scd30Available ? "OK" : "ERROR");
  } else if (sensorDataValid) {
    display.print("CO2: ");
    display.print(lastCO2, 0);
    display.println(" ppm");
    display.print("T: ");
    display.print(lastTemp, 1);
    display.println(" C");
    display.print("H: ");
    display.print(lastHum, 1);
    display.println(" %");
    display.print("Logs: ");
    display.println(dataLog.size());
  } else {
    display.println("Waiting for data...");
    display.print("SCD30: ");
    display.println(scd30Available ? "Init..." : "ERROR");
  }
 
  display.display();
}


void IRAM_ATTR handleEncoder() {
  int clkState = digitalRead(ENCODER_CLK);
  int dtState = digitalRead(ENCODER_DT);
 
  if (clkState != lastEncoded) {
    if (dtState != clkState)
      encoderValue++;
    else
      encoderValue--;
   
    encoderValue = constrain(encoderValue, 0, 255);
    lastEncoded = clkState;
  }
}


void handleButton() {
  bool reading = digitalRead(ENCODER_SW);
 
  if (reading == LOW && lastButtonState == HIGH &&
      (millis() - lastDebounceTime) > debounceDelay) {
    currentMode = (currentMode == PWM_MODE) ? SENSOR_MODE : PWM_MODE;
    lastDebounceTime = millis();
  }
 
  lastButtonState = reading;
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        sendWebSocketData();
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\n", num, length);
      break;
    default:
      break;
  }
}


void sendWebSocketData() {
  StaticJsonDocument<200> doc;
 
  doc["co2"] = lastCO2;
  doc["temp"] = lastTemp;
  doc["hum"] = lastHum;
  doc["pwm"] = pwmValue;
  doc["mode"] = (currentMode == PWM_MODE) ? "PWM" : "SENSOR";
  doc["sensorStatus"] = scd30Available ? "OK" : "ERROR";
 
  String jsonString;
  serializeJson(doc, jsonString);
 
  webSocket.broadcastTXT(jsonString);
}


void setupWiFiAP() {
  Serial.println("Setting up WiFi Access Point...");
 
  WiFi.softAP(ssid, password);
 
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
 
  server.on("/", handleRoot);
  server.on("/download", handleDownloadCSV);
  server.on("/stats", handleStats);
  server.on("/clear", HTTP_POST, handleClearData);
 
  server.on("/styles.css", []() {
    handleFileRead("/styles.css");
  });
 
  server.on("/script.js", []() {
    handleFileRead("/script.js");
  });
 
  server.onNotFound([]() {
    handleFileRead(server.uri());
  });
 
  server.begin();
  Serial.println("HTTP server started");
 
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started on port 81");
}


// ==========================
// Setup
// ==========================
void setup() {
  Serial.begin(115200);
  delay(1000); // Dar tiempo al sistema para estabilizarse
  
  Serial.println("\n\n=== ESP32 SCD30 Data Logger Starting ===");
 
  // Inicializar Wire (I2C) con pines específicos si es necesario
  // Wire.begin(21, 22); // SDA, SCL - Descomentar si usas pines diferentes
  Wire.begin(); // Usar pines por defecto
  Wire.setClock(100000); // Configurar velocidad I2C a 100kHz para mejor compatibilidad
  
  // Inicializar LittleFS
  if (!LittleFS.begin(true)) { // true = formatear si falla
    Serial.println("LittleFS Mount Failed - Formatting...");
    LittleFS.format();
    if (!LittleFS.begin()) {
      Serial.println("LittleFS Mount Failed after format!");
      return;
    }
  }
  Serial.println("LittleFS mounted successfully");
 
  // Verificar archivos esenciales
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  Serial.println("Files in LittleFS:");
  while (file) {
    Serial.printf("  %s (%d bytes)\n", file.name(), file.size());
    file = root.openNextFile();
  }
 
  if (!LittleFS.exists("/index.html")) {
    Serial.println("WARNING: index.html not found!");
  }
  if (!LittleFS.exists("/styles.css")) {
    Serial.println("WARNING: styles.css not found!");
  }
  if (!LittleFS.exists("/script.js")) {
    Serial.println("WARNING: script.js not found!");
  }
 
  systemStartTime = millis();
 
  // Configurar pines del encoder
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoder, CHANGE);
 
  // Inicializar OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    // No hacer while(true) para permitir que el resto funcione
  } else {
    Serial.println("OLED initialized");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Initializing...");
    display.display();
  }
 
  // Configurar PWM para el motor
  ledcAttach(MOTOR_PIN, 20000, 8);
  ledcWrite(MOTOR_PIN, 0);
  Serial.println("PWM initialized");
 
  // Inicializar sensor SCD30 - CRÍTICO: Hacer esto DESPUÉS del OLED
  delay(1000); // Dar tiempo adicional antes de inicializar SCD30
  initSCD30();
 
  // Reservar espacio para el data log
  dataLog.reserve(MAX_DATA_RECORDS);
 
  // Configurar WiFi AP y servidores
  setupWiFiAP();
 
  // Actualizar display con información inicial
  updateOLED();
  
  Serial.println("=== Setup Complete ===\n");
}


// ==========================
// Loop
// ==========================
void loop() {
  // Manejar servidor web
  server.handleClient();
 
  // Manejar WebSocket
  webSocket.loop();
 
  // Manejar botón del encoder
  handleButton();
 
  // Actualizar PWM si está en modo PWM
  if (currentMode == PWM_MODE) {
    pwmValue = encoderValue;
    ledcWrite(MOTOR_PIN, pwmValue);
  }
 
  // Leer datos del sensor periódicamente
  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorRead >= sensorReadInterval) {
    lastSensorRead = currentMillis;
    readSCD30();
  }
 
  // Log de datos cada X segundos
  if (currentMillis - lastDataLog >= dataLogInterval && sensorDataValid) {
    lastDataLog = currentMillis;
    logData();
  }
 
  // Actualizar display OLED
  static unsigned long lastDisplayUpdate = 0;
  if (currentMillis - lastDisplayUpdate >= 500) { // Actualizar cada 500ms
    lastDisplayUpdate = currentMillis;
    updateOLED();
  }
 
  // Enviar datos por WebSocket periódicamente
  if (currentMillis - lastWebSocketUpdate >= wsUpdateInterval) {
    lastWebSocketUpdate = currentMillis;
    if (webSocket.connectedClients() > 0) {
      sendWebSocketData();
    }
  }
 
  delay(50); // Reducir el delay para mejor respuesta
}
