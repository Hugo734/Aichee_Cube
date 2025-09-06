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
//tiempo en excel
#include <TimeLib.h> 

// ========================== 
// Configuración WiFi AP
// ==========================
const char* ssid = "ESP32_SCD30";
const char* password = "12345678";  // Mínimo 8 caracteres

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

// Modo
enum DisplayMode {
  PWM_MODE,
  SENSOR_MODE
};
DisplayMode currentMode = PWM_MODE;

// Control de tiempo para WebSocket
unsigned long lastWebSocketUpdate = 0;
const unsigned long wsUpdateInterval = 1000; // Actualizar cada 1 segundo

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
const size_t MAX_DATA_RECORDS = 500; // Máximo de registros en memoria
unsigned long lastDataLog = 0;
const unsigned long dataLogInterval = 5000; // Log cada 5 segundos
unsigned long systemStartTime = 0;

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
  if (!sensorDataValid) return;
  
  DataRecord record;
  record.timestamp = millis() - systemStartTime;
  record.co2 = lastCO2;
  record.temperature = lastTemp;
  record.humidity = lastHum;
  record.pwm = pwmValue;
  
  dataLog.push_back(record);
  
  // Mantener el tamaño del buffer bajo control
  if (dataLog.size() > MAX_DATA_RECORDS) {
    dataLog.erase(dataLog.begin());
  }
  
  Serial.printf("Data logged: CO2=%.0f, Temp=%.1f, Hum=%.1f, PWM=%d\n", 
                record.co2, record.temperature, record.humidity, record.pwm);
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
  doc["uptime"] = millis() / 1000;
  
  if (!dataLog.empty()) {
    // Calcular promedios
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
  server.send(200, "text/plain", "Data cleared successfully");
  Serial.println("Data log cleared");
}

// HTML mejorado con botón de descarga
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
  </style>
</head>
<body>
  <div class="card">
    <h1>SCD30 Data Logger</h1>
    <p>Status: <span id="status" class="disconnected">Disconnected</span></p>
     
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
    </div>
    
    <div class="btn-group">
      <button class="btn" onclick="downloadCSV()">📥 Download Excel (CSV)</button>
      <button class="btn btn-info" onclick="refreshStats()">🔄 Refresh Stats</button>
      <button class="btn btn-danger" onclick="clearData()">🗑️ Clear Data</button>
    </div>
     
    <div class="info">
      Data is logged every 5 seconds. Maximum 500 records stored.
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
        
        // Ejes
        ctx.strokeStyle = '#ccc';
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(chartPadding, chartPadding);
        ctx.lineTo(chartPadding, height - chartPadding);
        ctx.lineTo(width - chartPadding, height - chartPadding);
        ctx.stroke();
        
        // Etiquetas Y cada 100 ppm
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
        
        // Datos
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
          const startIdx = Math.max(0, co2Data.length - 5);
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
       
      function connect() {
        console.log('Connecting to WebSocket:', wsUrl);
        try {
          ws = new WebSocket(wsUrl);
          ws.addEventListener('open', () => {
            console.log('WebSocket connected');
            statusEl.textContent = 'Connected';
            statusEl.className = 'connected';
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
            } catch (e) {
              console.warn('Error parsing message:', e, evt.data);
            }
          });
          
          ws.addEventListener('close', () => {
            console.log('WebSocket disconnected');
            statusEl.textContent = 'Disconnected';
            statusEl.className = 'disconnected';
            reconnectTimer = setTimeout(connect, 2000);
          });
          
          ws.addEventListener('error', (err) => {
            console.error('WebSocket error:', err);
            ws.close();
          });
        } catch (e) {
          console.error('Failed to create WebSocket:', e);
          reconnectTimer = setTimeout(connect, 2000);
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
        if (confirm('Are you sure you want to clear all logged data?')) {
          fetch('/clear', { method: 'POST' })
            .then(r => r.text())
            .then(result => { alert(result); refreshStats(); })
            .catch(error => console.error('Error:', error));
        }
      };
      
      window.refreshStats = function() {
        fetch('/stats')
          .then(r => r.json())
          .then(data => {
            recordCountEl.textContent = data.totalRecords + '/' + data.maxRecords;
            avgCO2El.textContent = data.avgCO2 ? data.avgCO2.toFixed(0) + ' ppm' : '—';
            avgTempEl.textContent = data.avgTemp ? data.avgTemp.toFixed(1) + '°C' : '—';
          })
          .catch(error => console.error('Error fetching stats:', error));
      };
       
      connect();
      window.addEventListener('resize', drawChart);
      setInterval(refreshStats, 10000);
    })();
  </script>
</body>
</html>
)rawliteral";


// ==========================
// Funciones
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
  } else if (sensorDataValid) {
    display.print("CO2: ");
    display.println(lastCO2, 0);
    display.print("T: ");
    display.print(lastTemp, 1);
    display.println(" C");
    display.print("H: ");
    display.print(lastHum, 1);
    display.println(" %");
    display.print("Logs: ");
    display.println(dataLog.size());
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

// Función para manejar la página principal
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// Función para manejar página no encontrada
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
        
        // Enviar datos iniciales al cliente conectado
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
  
  doc["co2"] = lastCO2;
  doc["temp"] = lastTemp;
  doc["hum"] = lastHum;
  doc["pwm"] = pwmValue;
  doc["mode"] = (currentMode == PWM_MODE) ? "PWM" : "SENSOR";
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Enviar a todos los clientes conectados
  webSocket.broadcastTXT(jsonString);
}

// Configurar WiFi en modo AP
void setupWiFiAP() {
  Serial.println("Setting up WiFi Access Point...");
  
  // Configurar el ESP32 como Access Point
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  // Configurar rutas del servidor web
  server.on("/", handleRoot);
  server.on("/download", handleDownloadCSV);
  server.on("/stats", handleStats);
  server.on("/clear", HTTP_POST, handleClearData);
  server.onNotFound(handleNotFound);
  
  // Iniciar servidor web
  server.begin();
  Serial.println("HTTP server started");
  
  // Iniciar WebSocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started on port 81");
}

// ==========================
// Setup
// ==========================
void setup() {
  Serial.begin(115200);
  
  // Guardar tiempo de inicio
  systemStartTime = millis();
  
  // Configurar pines del encoder
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoder, CHANGE);
  
  // Inicializar OLED
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
  
  // Configurar PWM para el motor
  ledcAttach(MOTOR_PIN, 20000, 8);
  ledcWrite(MOTOR_PIN, 0);
  
  // Inicializar sensor SCD30
  if (!airSensor.begin()) {
    Serial.println("SCD30 not detected!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("SCD30 not found!");
    display.display();
  } else {
    Serial.println("SCD30 ready.");
  }
  
  // Reservar espacio para el data log
  dataLog.reserve(MAX_DATA_RECORDS);
  
  // Configurar WiFi AP y servidores
  setupWiFiAP();
  
  // Actualizar display con información inicial
  updateOLED();
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
  
  // Leer datos del sensor si están disponibles
  if (airSensor.dataAvailable()) {
    lastCO2 = airSensor.getCO2();
    lastTemp = airSensor.getTemperature();
    lastHum = airSensor.getHumidity();
    sensorDataValid = true;
  }
  
  // Log de datos cada X segundos
  unsigned long currentMillis = millis();
  if (currentMillis - lastDataLog >= dataLogInterval && sensorDataValid) {
    lastDataLog = currentMillis;
    logData();
  }
  
  // Actualizar display OLED
  updateOLED();
  
  // Enviar datos por WebSocket periódicamente
  if (currentMillis - lastWebSocketUpdate >= wsUpdateInterval) {
    lastWebSocketUpdate = currentMillis;
    if (webSocket.connectedClients() > 0) {
      sendWebSocketData();
    }
  }
  
  delay(100);
}