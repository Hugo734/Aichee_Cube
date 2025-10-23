// Libraries used
#include <DNSServer.h>
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
#include "AETHER_html.h"
#include <esp_task_wdt.h>

// ========================== 
// WiFi Access Point Configuration
// ==========================
const char* ssid = "Aethernet";
const char* password = "12345678";
DNSServer dnsServer;
const byte DNS_PORT = 53;

// Server instances
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Hardware Pin Definitions
#define MOTOR_PIN 18
#define ENCODER_CLK 34
#define ENCODER_DT 35
#define ENCODER_SW 32

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Splash screen bitmap (preserved from original)
const unsigned char splash_bitmap[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x09, 0x82, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0c, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x62, 0x38, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x93, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x2c, 0xc3, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x8c, 0x89, 0x31, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x24, 0xc6, 0x24, 0x70, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x64, 0x50, 0xcd, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x93, 0x33, 0x11, 0x8e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x42, 0x48, 0x8c, 0x66, 0x63, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x0c, 0x9a, 0x91, 0x98, 0x88, 0xe0, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x66, 0x66, 0x23, 0x32, 0x30, 0x20, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0xcb, 0x11, 0x08, 0xcc, 0x46, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x69, 0x33, 0x33, 0x11, 0x99, 0x9c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0xcc, 0xcc, 0x66, 0x62, 0x46, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0x42, 0x49, 0x11, 0x89, 0x0c, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0x49, 0x32, 0x66, 0x33, 0x32, 0x24, 0x08, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x26, 0x88, 0xcc, 0xc8, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x20, 0x70, 0x09, 0x99, 0x11, 0x20, 0x3c, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x5c, 0x44, 0x66, 0x66, 0x41, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0x6f, 0x03, 0x24, 0x89, 0x01, 0xf4, 0x08, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x88, 0x99, 0xb0, 0x0f, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x20, 0x5c, 0xe0, 0x12, 0x48, 0x1e, 0x56, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0x70, 0x0c, 0xc0, 0x75, 0xd4, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0x49, 0x5e, 0x23, 0x01, 0xed, 0x74, 0x20, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7b, 0x9b, 0x80, 0x09, 0xa7, 0x5c, 0x08, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x20, 0x66, 0xe6, 0xc0, 0x0f, 0xbd, 0xd6, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x09, 0x39, 0x39, 0x70, 0x9e, 0xed, 0x74, 0x20, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x4f, 0x6d, 0x9c, 0x7a, 0xad, 0x5c, 0x08, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0xce, 0xec, 0x6b, 0xbd, 0xd6, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x6d, 0xb3, 0x30, 0x6e, 0xe7, 0x74, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x4d, 0x3d, 0x9c, 0x6a, 0xbd, 0x5c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0xcc, 0xe4, 0x3b, 0xad, 0xd6, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0xe7, 0x6c, 0x6e, 0xed, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x59, 0xb9, 0x98, 0x6a, 0xbd, 0x5c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x5d, 0x9a, 0xec, 0x6b, 0xa7, 0x56, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x64, 0x6e, 0xed, 0xd4, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0xed, 0x9c, 0x6a, 0xbd, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0x5c, 0x9b, 0xb0, 0x6b, 0xad, 0x5c, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x08, 0x1b, 0x72, 0xdc, 0x6e, 0xed, 0xd8, 0x40, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x08, 0x0b, 0x5c, 0xcc, 0x6a, 0xb9, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x02, 0x42, 0xcf, 0x30, 0x6b, 0xad, 0x81, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe3, 0x74, 0x6e, 0xef, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x3c, 0xdc, 0x6a, 0x98, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x1b, 0x90, 0x6b, 0xb0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x06, 0x7c, 0x6f, 0xe1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0xcc, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xb0, 0x7e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x34, 0x68, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x0c, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Rotary Encoder Variables
volatile int encoderValue = 0;
int lastEncoder = 0;
unsigned long debounceDelay = 60;

// Single and Double Click Detection
const unsigned long DEBOUNCE_MS = 60;
const unsigned long DBLCLICK_MS = 350;
static bool btn_lastStable = HIGH;
static bool btn_lastRead = HIGH;
static unsigned long btn_tEdge = 0;
static bool btn_pressed = false;
static int btn_clicks = 0;
static unsigned long btn_tRelease = 0;
static bool btn_waitingForDouble = false;

// PWM Control Variables
int pwmValue = 0;
int autoModePWMTarget = 255;

// SCD30 Sensor Variables
SCD30 airSensor;
float lastCO2 = 0, lastTemp = 0, lastHum = 0;
bool sensorDataValid = false;

// OLED Screen Management
enum OledScreen {
  SCREEN_SPLASH,
  SCREEN_QR,
  SCREEN_MAIN,
  SCREEN_MODE_MANUAL,
  SCREEN_MODE_AUTO
};

OledScreen oledScreen = SCREEN_SPLASH;
int mainMenuSelection = 0;
int currentGaugeScreen = 0;

// Automatic Mode Warmup
bool warmingUp = false;
unsigned long warmupStart = 0;
const unsigned long warmupDuration = 120000; // 2 minutes

// Task Scheduling - OPTIMIZED
unsigned long lastWebSocketUpdate = 0;
const unsigned long wsUpdateInterval = 2000;    // ⚠️ Changed: 2s instead of 1s
unsigned long lastOLEDUpdate = 0;
const unsigned long oledUpdateInterval = 500;
unsigned long lastSensorRead = 0;
const unsigned long sensorReadInterval = 1000;
unsigned long lastServerHandle = 0;
const unsigned long serverHandleInterval = 10;
unsigned long lastMemoryCheck = 0;
const unsigned long memoryCheckInterval = 30000; // Check memory every 30s

// Data Logging - OPTIMIZED FOR 1+ HOUR
struct DataRecord {
  unsigned long timestamp;
  float co2;
  float temperature;
  float humidity;
  int pwm;
};

std::vector<DataRecord> dataLog;
const size_t MAX_DATA_RECORDS = 2500;  // ⚠️ Changed: 2500 records = ~1.4 hours at 2s interval
unsigned long lastDataLog = 0;
const unsigned long dataLogInterval = 2000;
unsigned long systemStartTime = 0;

bool loggingEnabled = true;
bool dataClearedFlag = false;

// Memory monitoring
void checkMemory() {
  size_t freeHeap = ESP.getFreeHeap();
  size_t minFreeHeap = ESP.getMinFreeHeap();
  
  Serial.printf("Free Heap: %u bytes | Min Free: %u bytes | Records: %u\n", 
                freeHeap, minFreeHeap, dataLog.size());
  
  // Critical memory check
  if (freeHeap < 20000) {  // Less than 20KB free
    Serial.println("⚠️ LOW MEMORY WARNING! Clearing oldest 500 records...");
    if (dataLog.size() > 500) {
      dataLog.erase(dataLog.begin(), dataLog.begin() + 500);
    }
  }
}

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
  if (!sensorDataValid || !loggingEnabled) return;
  
  DataRecord record;
  record.timestamp = millis() - systemStartTime;
  record.co2 = lastCO2;
  record.temperature = lastTemp;
  record.humidity = lastHum;
  record.pwm = pwmValue;
  
  dataLog.push_back(record);
  
  // Remove oldest if exceeds max
  if (dataLog.size() > MAX_DATA_RECORDS) {
    dataLog.erase(dataLog.begin());
  }
  
  // Periodic debug (every 100 records)
  if (dataLog.size() % 100 == 0) {
    Serial.printf("Data logged #%d: CO2=%.0f, Temp=%.1f, Hum=%.1f, PWM=%d, Time=%s\n", 
                  dataLog.size(), record.co2, record.temperature, record.humidity, 
                  record.pwm, getFormattedTime(record.timestamp).c_str());
  }
}

// Optimized CSV generation with chunking
String generateCSV() {
  String csv;
  csv.reserve(64 + dataLog.size() * 40); // Pre-allocate memory
  csv += F("Elapsed (DD:HH:MM:SS),CO2 (ppm),Temperature (C),Humidity (%),PWM\n");

  for (const auto& record : dataLog) {
    csv += getFormattedTime(record.timestamp);
    csv += ',';
    csv += String(record.co2, 0);
    csv += ',';
    csv += String(record.temperature, 1);
    csv += ',';
    csv += String(record.humidity, 1);
    csv += ',';
    csv += String(record.pwm);
    csv += '\n';
    
    yield(); // Prevent watchdog timeout during large CSV generation
  }

  return csv;
}

void handleDownloadCSV() {
  if (dataLog.empty()) {
    server.send(404, "text/plain", "No data available");
    return;
  }

  String csv = generateCSV();
  server.sendHeader("Content-Type", "text/csv");
  server.sendHeader("Content-Disposition", "attachment; filename=\"Aether_data.csv\"");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/csv", csv);
  
  Serial.println("CSV file downloaded");
}

void handleClearData() {
  dataLog.clear();
  dataLog.shrink_to_fit(); // Release memory
  systemStartTime = millis();
  lastDataLog = millis();
  dataClearedFlag = true;
  loggingEnabled = false;
  
  server.send(200, "text/plain", "Data cleared successfully. Time reset.");
  Serial.println("Data log cleared and time reset");
}

void handleToggleLogging() {
  loggingEnabled = !loggingEnabled;
  
  if (loggingEnabled && dataClearedFlag) {
    systemStartTime = millis();
    lastDataLog = millis();
    dataClearedFlag = false;
  }
  
  String message = loggingEnabled ? "Logging started" : "Logging stopped";
  server.send(200, "text/plain", message);
  Serial.println(message);
}

void handleStartLogging() {
  if (!loggingEnabled || dataClearedFlag) {
    loggingEnabled = true;
    if (dataClearedFlag) {
      systemStartTime = millis();
      lastDataLog = millis();
      dataClearedFlag = false;
    }
    server.send(200, "text/plain", "Logging started");
    Serial.println("Logging started via refresh");
  } else {
    server.send(200, "text/plain", "Logging already active");
  }
}

void drawGauge(int centerX, int centerY, int radius, float value, float minVal, float maxVal, 
               const char* label, const char* units) {
  
  float range = maxVal - minVal;
  float normalizedValue = constrain((value - minVal) / range, 0.0, 1.0);
  
  float startAngle = 225.0 * PI / 180.0;
  float endAngle = -45.0 * PI / 180.0;
  float angleRange = endAngle - startAngle;
  float currentAngle = startAngle + (angleRange * normalizedValue);
  
  // Draw gauge arc
  for (int angle = 0; angle <= 270; angle += 15) {
    float rad = (225 - angle) * PI / 180.0;
    int x1 = centerX + cos(rad) * radius;
    int y1 = centerY - sin(rad) * radius;
    int x2 = centerX + cos(rad) * (radius - 3);
    int y2 = centerY - sin(rad) * (radius - 3);
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }
  
  // Draw major tick marks
  for (int i = 0; i <= 4; i++) {
    float angle = startAngle + (angleRange * i / 4);
    int x1 = centerX + cos(angle) * radius;
    int y1 = centerY - sin(angle) * radius;
    int x2 = centerX + cos(angle) * (radius - 5);
    int y2 = centerY - sin(angle) * (radius - 5);
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }
  
  // Draw needle
  int needleX = centerX + cos(currentAngle) * (radius - 7);
  int needleY = centerY - sin(currentAngle) * (radius - 7);
  display.drawLine(centerX, centerY, needleX, needleY, SSD1306_WHITE);
  
  // Draw center pivot
  display.fillCircle(centerX, centerY, 2, SSD1306_WHITE);
  
  // Display value
  display.setTextSize(1);
  char valueStr[16];
  if (maxVal > 100) {
    snprintf(valueStr, sizeof(valueStr), "%.0f%s", value, units);
  } else {
    snprintf(valueStr, sizeof(valueStr), "%.1f%s", value, units);
  }
  
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(valueStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(centerX - w/2, centerY + radius + 2);
  display.print(valueStr);
  
  // Display label
  display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(centerX - w/2, centerY - radius - 8);
  display.print(label);
}

void drawDualGauges(float value1, float min1, float max1, const char* label1, const char* units1,
                    float value2, float min2, float max2, const char* label2, const char* units2,
                    const char* screen) {
  display.clearDisplay();
  
  drawGauge(40, 32, 20, value1, min1, max1, label1, units1);
  drawGauge(91, 32, 20, value2, min2, max2, label2, units2);
  
  display.setTextSize(1);
  display.setCursor(108, 0);
  display.print(screen);
  
  display.display();
}

void updateOLED() {
  switch (oledScreen) {
    case SCREEN_SPLASH:
      display.drawBitmap(0, 0, splash_bitmap, 128, 64, WHITE);
      display.display();
      break;
      
    case SCREEN_MAIN:
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("Connection Data:");
      display.print("IP: ");   display.println(WiFi.softAPIP());
      display.print("SSID: "); display.println(ssid);
      display.print("PSWD: "); display.println(password);
      display.println();
      display.println("Select mode:");
      display.print(mainMenuSelection == 0 ? "> " : "  ");
      display.println("Manual");
      display.print(mainMenuSelection == 1 ? "> " : "  ");
      display.println("Automatic");
      display.display();
      break;

    case SCREEN_MODE_MANUAL:
      if (currentGaugeScreen == 0) {
        drawDualGauges(
          lastCO2, 0, 2000, "CO2", "ppm",
          pwmValue, 0, 255, "PWM", "",
          "1/2"
        );
      } else {
        drawDualGauges(
          lastHum, 0, 100, "Humid", "%",
          lastTemp, 0, 50, "Temp", "C",
          "2/2"
        );
      }
      break;

    case SCREEN_MODE_AUTO:
      if (warmingUp) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        unsigned long elapsed = millis() - warmupStart;
        unsigned long remaining = (elapsed < warmupDuration) ? 
                                   (warmupDuration - elapsed) : 0;
        display.println("Auto Mode - Warmup");
        display.print("Time left: ");
        display.print(remaining / 1000);
        display.println(" s");
        display.println();
        display.print("Target PWM: ");
        display.println(autoModePWMTarget);
        display.print("Current PWM: ");
        display.println(pwmValue);
        display.println();
        display.println("Adjust PWM with encoder");
        display.println("Double-click to exit");
        display.display();
      } else {
        if (currentGaugeScreen == 0) {
          drawDualGauges(
            lastCO2, 0, 2000, "CO2", "ppm",
            pwmValue, 0, 255, "PWM", "",
            "1/2"
          );
        } else {
          drawDualGauges(
            lastHum, 0, 100, "Humid", "%",
            lastTemp, 0, 50, "Temp", "C",
            "2/2"
          );
        }
      }
      break;
  }
}

void IRAM_ATTR handleEncoder() {
  int clkState = digitalRead(ENCODER_CLK);
  int dtState  = digitalRead(ENCODER_DT);

  if (clkState != lastEncoder) {
    if (dtState != clkState) {
      encoderValue++;
    } else {
      encoderValue--;
    }
    
    // Constrain encoder value
    if (oledScreen == SCREEN_MODE_MANUAL) {
      encoderValue = constrain(encoderValue, 0, 255);
    } else if (oledScreen == SCREEN_MODE_AUTO && warmingUp) {
      encoderValue = constrain(encoderValue, 0, 255);
    } else if (oledScreen == SCREEN_MAIN) {
      encoderValue = constrain(encoderValue, 0, 1);
    }
    
    lastEncoder = clkState;
  }
}

void handleRoot() {
  server.send_P(200, "text/html", AETHER_html);
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
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
    default:
      break;
  }
}

void sendWebSocketData() {
  // Optimized JSON document size
  StaticJsonDocument<128> doc;
  char out[128];

  doc["t"]    = millis();
  doc["co2"]  = (int)lastCO2;        // Send as int to save bytes
  doc["temp"] = (int)(lastTemp * 10) / 10.0; // 1 decimal
  doc["hum"]  = (int)(lastHum * 10) / 10.0;  // 1 decimal
  doc["pwm"]  = pwmValue;

  if (oledScreen == SCREEN_MODE_MANUAL) {
    doc["mode"] = "MANUAL";
  } else if (oledScreen == SCREEN_MODE_AUTO) {
    doc["mode"] = "AUTO";
  } else {
    doc["mode"] = "MAIN";
  }

  size_t n = serializeJson(doc, out, sizeof(out));
  if (n) webSocket.broadcastTXT(out, n);
}

void setupWiFiAP() {
  Serial.println("Setting up WiFi Access Point...");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), 
                    IPAddress(192, 168, 4, 1), 
                    IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  dnsServer.start(DNS_PORT, "*", IP);
  
  // HTTP routes
  server.on("/", handleRoot);
  server.on("/download", handleDownloadCSV);
  server.on("/clear", HTTP_POST, handleClearData);
  server.on("/startlog", HTTP_POST, handleStartLogging);
  server.on("/toggle", HTTP_POST, handleToggleLogging);
  server.on("/generate_204", handleRoot);  // Android captive portal
  server.on("/gen_204", handleRoot);
  server.on("/hotspot-detect.html", handleRoot);  // iOS
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  webSocket.enableHeartbeat(20000, 5000, 3); // More relaxed heartbeat
  Serial.println("WebSocket server started on port 81");
}

void setup() {
  Serial.begin(115200);
  
  // Configure watchdog timer (30 seconds) - ESP32 Arduino Core 3.x
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 30000,
    .idle_core_mask = 0,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);
  
  systemStartTime = millis();

  // Encoder setup
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  lastEncoder = digitalRead(ENCODER_CLK);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoder, CHANGE);
  
  // OLED setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (true) { esp_task_wdt_reset(); delay(1000); }
  }
  display.clearDisplay();
  display.display();
  
  // PWM setup
  ledcAttach(MOTOR_PIN, 5000, 8);
  ledcWrite(MOTOR_PIN, 0);
  
  // SCD30 setup
  if (!airSensor.begin()) {
    Serial.println("SCD30 not detected!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("SCD30 not found!");
    display.display();
  } else {
    Serial.println("SCD30 ready.");
    airSensor.setAutoSelfCalibration(false);
    airSensor.setMeasurementInterval(3);
  }
  
  // Reserve memory
  dataLog.reserve(MAX_DATA_RECORDS);
  
  setupWiFiAP();
  
  oledScreen = SCREEN_SPLASH;
  updateOLED();
  
  Serial.println("Setup complete. Starting main loop...");
  Serial.printf("Free Heap: %u bytes\n", ESP.getFreeHeap());
}

void onSingleClick() {
  switch (oledScreen) {
    case SCREEN_SPLASH:
      oledScreen = SCREEN_MAIN;
      encoderValue = 0;
      mainMenuSelection = 0;
      break;
    case SCREEN_MAIN:
      oledScreen = (mainMenuSelection == 0) ? SCREEN_MODE_MANUAL : SCREEN_MODE_AUTO;
      if (oledScreen == SCREEN_MODE_AUTO) {
        warmingUp = true;
        warmupStart = millis();
        pwmValue = 0;
        autoModePWMTarget = 255;
        encoderValue = autoModePWMTarget;
        ledcWrite(MOTOR_PIN, pwmValue);
        currentGaugeScreen = 0;
      } else {
        encoderValue = pwmValue;
        currentGaugeScreen = 0;
      }
      break;
    case SCREEN_MODE_MANUAL:
    case SCREEN_MODE_AUTO:
      currentGaugeScreen = (currentGaugeScreen + 1) % 2;
      break;
  }
}

void onDoubleClick() {
  switch (oledScreen) {
    case SCREEN_MODE_MANUAL:
    case SCREEN_MODE_AUTO:
      oledScreen = SCREEN_MAIN;
      warmingUp = false;
      currentGaugeScreen = 0;
      encoderValue = mainMenuSelection;
      break;
    default:
      break;
  }
}

void handleModeButton() {
  bool raw = digitalRead(ENCODER_SW);

  if (raw != btn_lastRead) { 
    btn_lastRead = raw; 
    btn_tEdge = millis(); 
  }
  if (millis() - btn_tEdge < DEBOUNCE_MS) return;

  if (raw != btn_lastStable) {
    btn_lastStable = raw;

    if (btn_lastStable == LOW) {
      btn_pressed = true;
    } else {
      if (btn_pressed) {
        btn_pressed = false;
        unsigned long now = millis();

        if (!btn_waitingForDouble) {
          btn_clicks = 1;
          btn_tRelease = now;
          btn_waitingForDouble = true;
        } else {
          if (now - btn_tRelease <= DBLCLICK_MS) {
            btn_clicks = 0;
            btn_waitingForDouble = false;
            onDoubleClick();
          } else {
            onSingleClick();
            btn_clicks = 1;
            btn_tRelease = now;
            btn_waitingForDouble = true;
          }
        }
      }
    }
  }

  if (btn_waitingForDouble && (millis() - btn_tRelease > DBLCLICK_MS)) {
    if (btn_clicks == 1) onSingleClick();
    btn_clicks = 0;
    btn_waitingForDouble = false;
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Reset watchdog
  esp_task_wdt_reset();

  // DNS server
  dnsServer.processNextRequest();

  // Button handling
  handleModeButton();

  // HTTP and WebSocket (skip during splash)
  if (oledScreen != SCREEN_SPLASH) {
    if (currentMillis - lastServerHandle >= serverHandleInterval) {
      lastServerHandle = currentMillis;
      server.handleClient();
      webSocket.loop();
    }
  }
  
  // Read sensor
  if (currentMillis - lastSensorRead >= sensorReadInterval) {
    lastSensorRead = currentMillis;
    if (airSensor.dataAvailable()) {
      lastCO2 = airSensor.getCO2();
      lastTemp = airSensor.getTemperature();
      lastHum = airSensor.getHumidity();
      sensorDataValid = true;
    }
  }
  
  // Log data
  if (oledScreen != SCREEN_SPLASH && loggingEnabled && 
      currentMillis - lastDataLog >= dataLogInterval && sensorDataValid) {
    lastDataLog = currentMillis;
    logData();
  }
  
  // Memory check
  if (currentMillis - lastMemoryCheck >= memoryCheckInterval) {
    lastMemoryCheck = currentMillis;
    checkMemory();
  }
  
  // Update menu selection
  if (oledScreen == SCREEN_MAIN) {
    mainMenuSelection = constrain(encoderValue, 0, 1);
  }
  
  // Update PWM based on mode
  if (oledScreen == SCREEN_MODE_MANUAL) {
    pwmValue = constrain(encoderValue, 0, 255);
    ledcWrite(MOTOR_PIN, pwmValue);
  } else if (oledScreen == SCREEN_MODE_AUTO) {
    if (warmingUp) {
      autoModePWMTarget = constrain(encoderValue, 0, 255);
      if (currentMillis - warmupStart >= warmupDuration) {
        warmingUp = false;
        pwmValue = autoModePWMTarget;
        ledcWrite(MOTOR_PIN, pwmValue);
      }
    }
  }

  // OLED update
  if (currentMillis - lastOLEDUpdate >= oledUpdateInterval) {
    lastOLEDUpdate = currentMillis;
    updateOLED();
  }

  // WebSocket broadcast
  if (oledScreen != SCREEN_SPLASH && 
      currentMillis - lastWebSocketUpdate >= wsUpdateInterval) {
    lastWebSocketUpdate = currentMillis;
    if (webSocket.connectedClients() > 0) {
      sendWebSocketData();
    }
  }

  yield();
}