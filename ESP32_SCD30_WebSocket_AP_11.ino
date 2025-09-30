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

// Bitmap para la pantalla de inicio (128x64 pixels)
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

// Encoder
volatile int encoderValue = 0;
int lastEncoded = 0;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;

// Variables para cambio de pantalla con presión prolongada
unsigned long buttonPressStartTime = 0;
bool buttonLongPressDetected = false;
const unsigned long LONG_PRESS_TIME = 2000; // 2 segundos

// PWM
int pwmValue = 0;

// SCD30
SCD30 airSensor;
float lastCO2 = 0, lastTemp = 0, lastHum = 0;
bool sensorDataValid = false;

// ===== SISTEMA DE PANTALLAS =====
enum OledScreen {
  SCREEN_SPLASH,      
  SCREEN_MAIN,
  SCREEN_MODE_MANUAL,
  SCREEN_MODE_AUTO
};

OledScreen oledScreen = SCREEN_SPLASH;
int mainMenuSelection = 0;
int currentGaugeScreen = 0; // 0 = CO2/PWM, 1 = Humidity/Temperature

bool warmingUp = false;
unsigned long warmupStart = 0;
const unsigned long warmupDuration = 120000; // 2 minutos

// Control de tiempo para tareas
unsigned long lastWebSocketUpdate = 0;
const unsigned long wsUpdateInterval = 1000;
unsigned long lastOLEDUpdate = 0;
const unsigned long oledUpdateInterval = 500;
unsigned long lastSensorRead = 0;
const unsigned long sensorReadInterval = 1000;
unsigned long lastServerHandle = 0;
const unsigned long serverHandleInterval = 10;

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
const size_t MAX_DATA_RECORDS = 1000;
unsigned long lastDataLog = 0;
const unsigned long dataLogInterval = 2000;
unsigned long systemStartTime = 0;

// Variable para controlar el estado del logging
bool loggingEnabled = true;
bool dataClearedFlag = false;

// HTML actualizado para la interfaz web
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>SCD30 Data Logger</title>
  <style>
    body{font-family:system-ui,Arial,sans-serif;margin:20px;line-height:1.6;background:#f5f5f5}
    h1{font-size:1.6rem;margin-bottom:0.25rem;color:#333}
    h2{font-size:1.3rem;margin-bottom:10px;color:#555}
    .row{display:flex;gap:12px;align-items:center;margin:8px 0}
    .label{min-width:110px;color:#444}
    .val{font-weight:700;font-size:1.25rem}
    #status{font-weight:700;padding:4px 8px;border-radius:4px}
    #status.connected{background:#d4edda;color:#155724}
    #status.disconnected{background:#f8d7da;color:#721c24}
    .card{padding:16px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);max-width:600px;margin-bottom:20px;background:white}
    canvas{border:1px solid #ddd;background:white;width:100%;max-width:560px;height:250px;display:block;margin-top:10px}
    .info{margin-top:8px;color:#666;font-size:0.9rem}
    hr{border:none;border-top:1px solid #e0e0e0;margin:15px 0}
    .btn{padding:10px 20px;background:#4CAF50;color:white;border:none;border-radius:4px;cursor:pointer;font-size:14px;margin:5px;transition:background 0.3s}
    .btn:hover{background:#45a049}
    .btn-danger{background:#f44336}
    .btn-danger:hover{background:#da190b}
    .btn-info{background:#2196F3}
    .btn-info:hover{background:#0b7dda}
    .btn-group{display:flex;gap:10px;flex-wrap:wrap;margin-top:15px}
    .stats{background:#f9f9f9;padding:10px;border-radius:4px;margin-top:10px}
    .stat-item{display:inline-block;margin-right:20px;margin-bottom:5px}
    .stat-label{font-size:0.85rem;color:#666}
    .stat-value{font-weight:bold;color:#333}
    .mode-indicator{display:inline-block;padding:2px 8px;border-radius:3px;font-size:0.85rem;margin-left:10px}
    .mode-manual{background:#ffeb3b;color:#333}
    .mode-auto{background:#4caf50;color:white}
    .logging-status{display:inline-block;padding:2px 8px;border-radius:3px;font-size:0.85rem;margin-left:10px}
    .logging-active{background:#4caf50;color:white}
    .logging-paused{background:#ff9800;color:white}
  </style>
</head>
<body>
  <div class="card">
    <h1>SCD30 Data Logger <span id="modeIndicator" class="mode-indicator"></span></h1>
    <p>Status: <span id="status" class="disconnected">Disconnected</span>
       <span id="loggingStatus" class="logging-status"></span></p>
     
    <div class="row">
      <div class="label">CO₂ (ppm):</div>
      <div id="co2" class="val">—</div>
    </div>
     
    <div class="row">
      <div class="label">Temperature (°C):</div>
      <div id="temp" class="val">—</div>
    </div>
     
    <div class="row">
      <div class="label">Humidity (%):</div>
      <div id="hum" class="val">—</div>
    </div>
     
    <hr/>
     
    <div class="row">
      <div class="label">PWM value:</div>
      <div id="pwm" class="val">—</div>
    </div>
    
    <div class="stats" id="statsPanel">
      <div class="stat-item">
        <span class="stat-label">Records:</span>
        <span class="stat-value" id="recordCount">0</span>
      </div>
      <div class="stat-item">
        <span class="stat-label">Avg CO₂:</span>
        <span class="stat-value" id="avgCO2">—</span>
      </div>
      <div class="stat-item">
        <span class="stat-label">Avg Temp:</span>
        <span class="stat-value" id="avgTemp">—</span>
      </div>
      <div class="stat-item">
        <span class="stat-label">Session Time:</span>
        <span class="stat-value" id="sessionTime">00:00:00</span>
      </div>
    </div>
    
    <div class="btn-group">
      <button class="btn" onclick="downloadCSV()">📥 Download Excel (CSV)</button> 
      <button class="btn btn-info" onclick="startLogging()">▶️ Start Logging</button>
      <button class="btn btn-danger" onclick="clearData()">🗑️ Clear & Reset</button>
    </div>
     
    <div class="info">
      Data is logged every 2 seconds. Maximum 1000 records stored.
      Clear & Reset will erase all data and reset the timer.
    </div>
  </div>
  
  <div class="card">
    <h2>CO₂ History (Last 60 seconds)</h2>
    <canvas id="co2Chart"></canvas>
    <div class="info">
      Range: 0-900 ppm | Updates every second
    </div>
  </div>
   
  <script>
    (function(){
      const statusEl = document.getElementById('status');
      const co2El = document.getElementById('co2');
      const tempEl = document.getElementById('temp');
      const humEl = document.getElementById('hum');
      const pwmEl = document.getElementById('pwm');
      const modeEl = document.getElementById('modeIndicator');
      const loggingStatusEl = document.getElementById('loggingStatus');
      const sessionTimeEl = document.getElementById('sessionTime');
      
      const recordCountEl = document.getElementById('recordCount');
      const avgCO2El = document.getElementById('avgCO2');
      const avgTempEl = document.getElementById('avgTemp');
      
      const canvas = document.getElementById('co2Chart');
      const ctx = canvas.getContext('2d');
      const maxDataPoints = 60;
      const co2Data = [];
      
      const chartPadding = 40;
      const minCO2 = 0;
      const maxCO2 = 900;
      
      let sessionStartTime = Date.now();
      
      function updateSessionTime(uptime) {
        if (uptime !== undefined) {
          const hours = Math.floor(uptime / 3600);
          const minutes = Math.floor((uptime % 3600) / 60);
          const seconds = uptime % 60;
          sessionTimeEl.textContent = 
            String(hours).padStart(2, '0') + ':' +
            String(minutes).padStart(2, '0') + ':' +
            String(seconds).padStart(2, '0');
        }
      }
      
      function resizeCanvas() {
        const rect = canvas.getBoundingClientRect();
        canvas.width = rect.width;
        canvas.height = rect.height;
      }
      
      function drawChart() {
        resizeCanvas();
        const width = canvas.width;
        const height = canvas.height;
        ctx.clearRect(0, 0, width, height);
        
        ctx.strokeStyle = '#ccc';
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(chartPadding, chartPadding);
        ctx.lineTo(chartPadding, height - chartPadding);
        ctx.lineTo(width - chartPadding, height - chartPadding);
        ctx.stroke();
        
        ctx.fillStyle = '#666';
        ctx.font = '11px sans-serif';
        ctx.textAlign = 'right';
        ctx.textBaseline = 'middle';
        
        const step = 100;
        for (let value = minCO2; value <= maxCO2; value += step) {
          const normalized = (value - minCO2) / (maxCO2 - minCO2);
          const y = height - chartPadding - (height - 2 * chartPadding) * normalized;
          
          ctx.fillText(value.toFixed(0), chartPadding - 5, y);
          
          if (value !== minCO2 && value !== maxCO2) {
            ctx.strokeStyle = '#f0f0f0';
            ctx.beginPath();
            ctx.moveTo(chartPadding, y);
            ctx.lineTo(width - chartPadding, y);
            ctx.stroke();
          }
        }
        
        if (co2Data.length > 1) {
          ctx.strokeStyle = '#4CAF50';
          ctx.lineWidth = 2;
          ctx.beginPath();
          
          const chartWidth = width - 2 * chartPadding;
          const chartHeight = height - 2 * chartPadding;
          
          for (let i = 0; i < co2Data.length; i++) {
            const x = chartPadding + (chartWidth * i / (maxDataPoints - 1));
            const normalizedValue = (co2Data[i] - minCO2) / (maxCO2 - minCO2);
            const y = chartPadding + chartHeight * (1 - normalizedValue);
            
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
          }
          ctx.stroke();
          
          ctx.fillStyle = '#4CAF50';
          const startIdx = Math.max(0, co2Data.length - 2);
          for (let i = startIdx; i < co2Data.length; i++) {
            const x = chartPadding + (chartWidth * i / (maxDataPoints - 1));
            const normalizedValue = (co2Data[i] - minCO2) / (maxCO2 - minCO2);
            const y = chartPadding + chartHeight * (1 - normalizedValue);
            
            ctx.beginPath();
            ctx.arc(x, y, 3, 0, 2 * Math.PI);
            ctx.fill();
          }
        }
        
        if (co2Data.length === 0) {
          ctx.fillStyle = '#999';
          ctx.font = '14px sans-serif';
          ctx.textAlign = 'center';
          ctx.textBaseline = 'middle';
          ctx.fillText('Waiting for data...', width / 2, height / 2);
        }
      }
      
      drawChart();
      
      const wsUrl = 'ws://' + location.hostname + ':81/';
      let ws;
      let reconnectTimer;
      let reconnectAttempts = 0;
      const maxReconnectAttempts = 10;
      const reconnectDelay = 2000;
       
      function connect() {
        if (reconnectAttempts >= maxReconnectAttempts) {
          console.log('Max reconnection attempts reached');
          statusEl.textContent = 'Connection failed';
          statusEl.className = 'disconnected';
          return;
        }
        
        console.log('Connecting to WebSocket:', wsUrl, 'Attempt:', reconnectAttempts + 1);
        try {
          ws = new WebSocket(wsUrl);
          
          ws.addEventListener('open', () => {
            console.log('WebSocket connected');
            statusEl.textContent = 'Connected';
            statusEl.className = 'connected';
            reconnectAttempts = 0;
            if (reconnectTimer) clearTimeout(reconnectTimer);
            refreshStats();
          });
          
          ws.addEventListener('message', (evt) => {
            try {
              const d = JSON.parse(evt.data);
              if (d.co2 !== undefined) {
                co2El.textContent = d.co2.toFixed(0);
                co2Data.push(parseFloat(d.co2));
                if (co2Data.length > maxDataPoints) co2Data.shift();
                drawChart();
              }
              if (d.temp !== undefined) tempEl.textContent = d.temp.toFixed(1);
              if (d.hum !== undefined) humEl.textContent = d.hum.toFixed(1);
              if (d.pwm !== undefined) pwmEl.textContent = d.pwm;
              if (d.mode !== undefined) {
                modeEl.textContent = d.mode;
                modeEl.className = 'mode-indicator mode-' + d.mode.toLowerCase();
              }
            } catch (e) {
              console.warn('Error parsing message:', e, evt.data);
            }
          });
          
          ws.addEventListener('close', () => {
            console.log('WebSocket disconnected');
            statusEl.textContent = 'Disconnected';
            statusEl.className = 'disconnected';
            reconnectAttempts++;
            reconnectTimer = setTimeout(connect, reconnectDelay);
          });
          
          ws.addEventListener('error', (err) => {
            console.error('WebSocket error:', err);
            ws.close();
          });
        } catch (e) {
          console.error('Failed to create WebSocket:', e);
          reconnectAttempts++;
          reconnectTimer = setTimeout(connect, reconnectDelay);
        }
      }
      
      window.downloadCSV = function() {
        const link = document.createElement('a');
        link.href = '/download';
        link.download = 'scd30_data.csv';
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
      };
      
      window.clearData = function() {
        if (confirm('This will clear all data and reset the timer. Are you sure?')) {
          fetch('/clear', { method: 'POST' })
            .then(r => r.text())
            .then(result => { 
              alert(result); 
              sessionStartTime = Date.now();
              refreshStats();
            })
            .catch(error => console.error('Error:', error));
        }
      };
      
      window.startLogging = function() {
        fetch('/startlog', { method: 'POST' })
          .then(r => r.text())
          .then(result => { 
            console.log(result);
            refreshStats();
          })
          .catch(error => console.error('Error:', error));
      };
      
      window.refreshStats = function() {
        fetch('/stats')
          .then(r => r.json())
          .then(data => {
            recordCountEl.textContent = data.totalRecords + '/' + data.maxRecords;
            avgCO2El.textContent = data.avgCO2 ? data.avgCO2.toFixed(0) + ' ppm' : '—';
            avgTempEl.textContent = data.avgTemp ? data.avgTemp.toFixed(1) + '°C' : '—';
            updateSessionTime(data.uptime);
            
            if (data.loggingEnabled) {
              loggingStatusEl.textContent = 'Recording';
              loggingStatusEl.className = 'logging-status logging-active';
            } else {
              loggingStatusEl.textContent = 'Paused';
              loggingStatusEl.className = 'logging-status logging-paused';
            }
          })
          .catch(error => console.error('Error fetching stats:', error));
      };
       
      connect();
      window.addEventListener('resize', drawChart);
      setInterval(refreshStats, 2000);
    })();
  </script>
</body>
</html>
)rawliteral";

// Función para obtener tiempo formateado
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

// Función para agregar registro de datos
void logData() {
  if (!sensorDataValid || !loggingEnabled) return;
  
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
  
  Serial.printf("Data logged #%d: CO2=%.0f, Temp=%.1f, Hum=%.1f, PWM=%d, Time=%s\n", 
                dataLog.size(), record.co2, record.temperature, record.humidity, 
                record.pwm, getFormattedTime(record.timestamp).c_str());
}

// Función para generar CSV
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

// Handler para descargar CSV
void handleDownloadCSV() {
  String csv = generateCSV();
  
  server.sendHeader("Content-Type", "text/csv");
  server.sendHeader("Content-Disposition", "attachment; filename=\"scd30_data.csv\"");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/csv", csv);
  
  Serial.println("CSV file downloaded");
}

// Handler para obtener estadísticas
void handleStats() {
  StaticJsonDocument<512> doc;
  doc["totalRecords"] = dataLog.size();
  doc["maxRecords"] = MAX_DATA_RECORDS;
  doc["logInterval"] = dataLogInterval / 1000;
  doc["uptime"] = (millis() - systemStartTime) / 1000;
  doc["loggingEnabled"] = loggingEnabled;
  
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

// Handler para limpiar datos
void handleClearData() {
  dataLog.clear();
  systemStartTime = millis();
  lastDataLog = millis();
  dataClearedFlag = true;
  loggingEnabled = false;
  
  server.send(200, "text/plain", "Data cleared successfully. Time reset.");
  Serial.println("Data log cleared and time reset");
}

// Handler para iniciar/detener el logging
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

// Handler para iniciar logging
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

// ==========================
// Funciones de dibujo de medidores (gauges)
// ==========================

// Función para dibujar un medidor circular con unidades
void drawGauge(int centerX, int centerY, int radius, float value, float minVal, float maxVal, 
               const char* label, const char* units, bool drawIndicators = false, 
               const char* mode = "", const char* screen = "", const char* info = "") {
  // NO aplicar offset adicional aquí - usar las coordenadas directas
  
  // Calcular el ángulo basado en el valor
  float range = maxVal - minVal;
  float normalizedValue = constrain((value - minVal) / range, 0.0, 1.0);
  
  // Los medidores van de 225° (izquierda abajo) a -45° (derecha abajo)
  float startAngle = 225.0 * PI / 180.0;
  float endAngle = -45.0 * PI / 180.0;
  float angleRange = endAngle - startAngle;
  float currentAngle = startAngle + (angleRange * normalizedValue);
  
  // Dibujar el arco del medidor
  for (int angle = 0; angle <= 270; angle += 15) {
    float rad = (225 - angle) * PI / 180.0;
    int x1 = centerX + cos(rad) * radius;
    int y1 = centerY - sin(rad) * radius;
    int x2 = centerX + cos(rad) * (radius - 3);
    int y2 = centerY - sin(rad) * (radius - 3);
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }
  
  // Dibujar marcas principales
  for (int i = 0; i <= 4; i++) {
    float angle = startAngle + (angleRange * i / 4);
    int x1 = centerX + cos(angle) * radius;
    int y1 = centerY - sin(angle) * radius;
    int x2 = centerX + cos(angle) * (radius - 5);
    int y2 = centerY - sin(angle) * (radius - 5);
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }
  
  // Dibujar la aguja
  int needleX = centerX + cos(currentAngle) * (radius - 7);
  int needleY = centerY - sin(currentAngle) * (radius - 7);
  display.drawLine(centerX, centerY, needleX, needleY, SSD1306_WHITE);
  
  // Dibujar círculo central
  display.fillCircle(centerX, centerY, 2, SSD1306_WHITE);
  
  // Mostrar el valor con unidades
  display.setTextSize(1);
  char valueStr[16];
  if (maxVal > 100) {
    snprintf(valueStr, sizeof(valueStr), "%.0f%s", value, units);
  } else {
    snprintf(valueStr, sizeof(valueStr), "%.1f%s", value, units);
  }
  
  // Centrar el texto del valor
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(valueStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(centerX - w/2, centerY + radius + 2);
  display.print(valueStr);
  
  // Mostrar el label
  display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(centerX - w/2, centerY - radius - 8);
  display.print(label);
  
  // Dibujar indicadores si es necesario
  if (drawIndicators) {
    display.setTextSize(1);
    // Modo en esquina superior izquierda
    display.setCursor(0, 0);
    display.print(mode);
    
    // Pantalla actual en esquina superior derecha
    display.setCursor(108, 0);
    display.print(screen);
    
    // Info adicional en esquina inferior izquierda
    display.setCursor(0, 56);
    display.print(info);
  }
}

// Función para dibujar 2 medidores en una pantalla con indicadores
void drawDualGauges(float value1, float min1, float max1, const char* label1, const char* units1,
                    float value2, float min2, float max2, const char* label2, const char* units2,
                    const char* mode, const char* screen, const char* info) {
  display.clearDisplay();
  
  // Ajuste de posiciones: mover el gauge izquierdo más a la derecha
  // Gauge izquierdo: ahora en x=40 (antes 27)
  // Gauge derecho: mantener en x=91
  drawGauge(40, 32, 20, value1, min1, max1, label1, units1, false, "", "", "");
  drawGauge(91, 32, 20, value2, min2, max2, label2, units2, false, "", "", "");
  
  // Dibujar indicadores una sola vez
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(mode);
  
  display.setCursor(108, 0);
  display.print(screen);
  
  display.setCursor(0, 56);
  display.print(info);
  
  display.display();
}

// Función actualizada para mostrar la pantalla OLED
void updateOLED() {

  switch (oledScreen) {
    case SCREEN_SPLASH:
      // Mostrar el bitmap que ocupa toda la pantalla (128x64)
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
      {
        char info[20];
        snprintf(info, sizeof(info), "R:%d", dataLog.size());
        
        if (currentGaugeScreen == 0) {
          drawDualGauges(
            lastCO2, 0, 2000, "CO2", "ppm",
            pwmValue, 0, 255, "PWM", "",
            "MAN", "1/2", info
          );
        } else {
          drawDualGauges(
            lastHum, 0, 100, "Humid", "%",
            lastTemp, 0, 50, "Temp", "C",
            "MAN", "2/2", info
          );
        }
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
        display.println("Auto Mode");
        display.println("Warming up sensor...");
        display.print("Time left: ");
        display.print(remaining / 1000);
        display.println(" s");
        display.println();
        display.println("Server active!");
        display.print("Clients: ");
        display.println(webSocket.connectedClients());
        display.display();
      } else {
        char info[20];
        snprintf(info, sizeof(info), "C:%d", webSocket.connectedClients());
        
        if (currentGaugeScreen == 0) {
          drawDualGauges(
            lastCO2, 0, 2000, "CO2", "ppm",
            pwmValue, 0, 255, "PWM", "",
            "AUTO", "1/2", info
          );
        } else {
          drawDualGauges(
            lastHum, 0, 100, "Humid", "%",
            lastTemp, 0, 50, "Temp", "C",
            "AUTO", "2/2", info
          );
        }
      }
      break;
  }
}

void IRAM_ATTR handleEncoder() {
  int clkState = digitalRead(ENCODER_CLK);
  int dtState = digitalRead(ENCODER_DT);
  
  if (clkState != lastEncoded) {
    if (dtState != clkState) {
      encoderValue++;
    } else {
      encoderValue--;
    }
    
    if (oledScreen == SCREEN_MODE_MANUAL) {
      encoderValue = constrain(encoderValue, 0, 255);
    } else if (oledScreen == SCREEN_MAIN) {
      encoderValue = constrain(encoderValue, 0, 1);
    }
    
    lastEncoded = clkState;
  }
}

// Handler para la página principal
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// Handler para página no encontrada
void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// Función WebSocket Event
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

// Función para enviar datos por WebSocket
void sendWebSocketData() {
  StaticJsonDocument<200> doc;

  doc["co2"]  = lastCO2;
  doc["temp"] = lastTemp;
  doc["hum"]  = lastHum;
  doc["pwm"]  = pwmValue;

  if (oledScreen == SCREEN_MODE_MANUAL) {
    doc["mode"] = "MANUAL";
  } else if (oledScreen == SCREEN_MODE_AUTO) {
    doc["mode"] = "AUTO";
  } else {
    doc["mode"] = "MAIN";
  }

  String jsonString;
  serializeJson(doc, jsonString);
  webSocket.broadcastTXT(jsonString);
}

// Configurar WiFi en modo AP
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
  
  // Configurar rutas del servidor web
  server.on("/", handleRoot);
  server.on("/download", handleDownloadCSV);
  server.on("/stats", handleStats);
  server.on("/clear", HTTP_POST, handleClearData);
  server.on("/startlog", HTTP_POST, handleStartLogging);
  server.on("/toggle", HTTP_POST, handleToggleLogging);
  server.onNotFound(handleNotFound);
  
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
  
  systemStartTime = millis();
  
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoder, CHANGE);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();
  
  ledcAttach(MOTOR_PIN, 5000, 8);
  ledcWrite(MOTOR_PIN, 0);
  
  if (!airSensor.begin()) {
    Serial.println("SCD30 not detected!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("SCD30 not found!");
    display.display();
  } else {
    Serial.println("SCD30 ready.");
    airSensor.setMeasurementInterval(2);
  }
  
  dataLog.reserve(MAX_DATA_RECORDS);
  
  setupWiFiAP();
  
  oledScreen = SCREEN_SPLASH;
  updateOLED();
}

// ==========================
// Loop optimizado
// ==========================
void loop() {
  unsigned long currentMillis = millis();
  
  // Solo manejar servidor y websocket si NO estamos en splash screen
  if (oledScreen != SCREEN_SPLASH) {
    if (currentMillis - lastServerHandle >= serverHandleInterval) {
      lastServerHandle = currentMillis;
      server.handleClient();
      webSocket.loop();
    }
  }
  
  if (currentMillis - lastSensorRead >= sensorReadInterval) {
    lastSensorRead = currentMillis;
    if (airSensor.dataAvailable()) {
      lastCO2 = airSensor.getCO2();
      lastTemp = airSensor.getTemperature();
      lastHum = airSensor.getHumidity();
      sensorDataValid = true;
    }
  }
  
  // Data logging
  if (oledScreen != SCREEN_SPLASH && loggingEnabled && 
      currentMillis - lastDataLog >= dataLogInterval && sensorDataValid) {
    lastDataLog = currentMillis;
    logData();
  }
  
  // Manejo del encoder en modo principal
  if (oledScreen == SCREEN_MAIN) {
    mainMenuSelection = constrain(encoderValue, 0, 1);
  }
  
  // Manejo del botón del encoder
  bool currentButtonState = digitalRead(ENCODER_SW);
  
  // Detectar cuando se presiona el botón
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    buttonPressStartTime = currentMillis;
    buttonLongPressDetected = false;
  }
  
  // Detectar presión prolongada (2 segundos) para cambiar entre pantallas de gauges
  if (currentButtonState == LOW && !buttonLongPressDetected &&
      (oledScreen == SCREEN_MODE_MANUAL || (oledScreen == SCREEN_MODE_AUTO && !warmingUp))) {
    if (currentMillis - buttonPressStartTime >= LONG_PRESS_TIME) {
      buttonLongPressDetected = true;
      currentGaugeScreen = (currentGaugeScreen + 1) % 2;
      Serial.printf("Long press detected! Screen changed to: %d\n", currentGaugeScreen);
      updateOLED(); // Actualizar inmediatamente
    }
  }
  
  // Detectar cuando se suelta el botón (click normal)
  if (currentButtonState == HIGH && lastButtonState == LOW) {
    if (!buttonLongPressDetected && (currentMillis - lastDebounceTime) > debounceDelay) {
      // Click normal (no fue una presión larga)
      if (oledScreen == SCREEN_SPLASH) {
        oledScreen = SCREEN_MAIN;
        encoderValue = 0;
        mainMenuSelection = 0;
      } else if (oledScreen == SCREEN_MAIN) {
        oledScreen = (mainMenuSelection == 0) ? SCREEN_MODE_MANUAL : SCREEN_MODE_AUTO;
        if (oledScreen == SCREEN_MODE_AUTO) {
          warmingUp = true;
          warmupStart = currentMillis;
          pwmValue = 0;
          ledcWrite(MOTOR_PIN, pwmValue);
          currentGaugeScreen = 0;
        } else if (oledScreen == SCREEN_MODE_MANUAL) {
          encoderValue = pwmValue;
          currentGaugeScreen = 0;
        }
      } else {
        // Volver al menú principal
        oledScreen = SCREEN_MAIN;
        warmingUp = false;
        encoderValue = mainMenuSelection;
        currentGaugeScreen = 0;
      }
      lastDebounceTime = currentMillis;
    }
    buttonLongPressDetected = false;
  }
  
  lastButtonState = currentButtonState;
  
  // En modo manual, ajustar PWM con el encoder
  if (oledScreen == SCREEN_MODE_MANUAL) {
    pwmValue = constrain(encoderValue, 0, 255);
    ledcWrite(MOTOR_PIN, pwmValue);
  }
  
  // Modo automático
  if (oledScreen == SCREEN_MODE_AUTO) {
    if (warmingUp && (currentMillis - warmupStart >= warmupDuration)) {
      warmingUp = false;
      pwmValue = 255;
      ledcWrite(MOTOR_PIN, pwmValue);
    }
  }
  
  // Actualización del OLED
  if (currentMillis - lastOLEDUpdate >= oledUpdateInterval) {
    lastOLEDUpdate = currentMillis;
    updateOLED();
  }
  
  // Enviar datos por WebSocket
  if (oledScreen != SCREEN_SPLASH && 
      currentMillis - lastWebSocketUpdate >= wsUpdateInterval) {
    lastWebSocketUpdate = currentMillis;
    if (webSocket.connectedClients() > 0) {
      sendWebSocketData();
    }
  }
  
  yield();
}