// Libraries used
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
// WiFi Access Point Configuration
// The ESP32 creates its own WiFi network that devices can connect to
// ==========================
const char* ssid = "Aether";      // Network name (SSID)
const char* password = "12345678";      // Network password (min 8 chars required)

// Server instances for HTTP and WebSocket communication
WebServer server(80);                   // HTTP server on port 80
WebSocketsServer webSocket = WebSocketsServer(81);  // WebSocket server on port 81

// Hardware Pin Definitions
#define MOTOR_PIN 18        // PWM output pin for motor/fan control
#define ENCODER_CLK 34      // Rotary encoder clock pin (rotation detection)
#define ENCODER_DT 35       // Rotary encoder data pin (rotation direction)
#define ENCODER_SW 32       // Rotary encoder switch pin (push button)

// OLED Display Configuration
#define SCREEN_WIDTH 128    // OLED display width in pixels
#define SCREEN_HEIGHT 64    // OLED display height in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Splash screen bitmap (128x64 pixels) - displayed on startup
// This array contains the pixel data for the initial logo/graphic
const unsigned char splash_bitmap[] PROGMEM = {
	// [Bitmap data preserved as in original]
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
volatile int encoderValue = 0;          // Current encoder position value
int lastEncoded = 0;                    // Previous encoder state for comparison
bool lastButtonState = HIGH;            // Previous button state for edge detection
unsigned long lastDebounceTime = 0;     // Timestamp for button debounce
unsigned long debounceDelay = 200;      // Debounce delay in milliseconds

// Long Press Detection Variables
// Used to detect when user holds the encoder button for screen switching
unsigned long buttonPressStartTime = 0;
bool buttonLongPressDetected = false;
const unsigned long LONG_PRESS_TIME = 2000; // 2 seconds for long press (gauge switch)
const unsigned long EXIT_AUTO_PRESS_TIME = 6000; // 6 seconds to exit auto mode

// PWM Control Variables
int pwmValue = 0;  // Current PWM value (0-255) for motor speed control
int autoModePWMTarget = 255; // Target PWM value after warmup in auto mode

// SCD30 Sensor Variables
SCD30 airSensor;                        // SCD30 sensor object
float lastCO2 = 0, lastTemp = 0, lastHum = 0;  // Latest sensor readings
bool sensorDataValid = false;           // Flag indicating valid sensor data available

// ===== DISPLAY SCREEN MANAGEMENT SYSTEM =====
// Defines different screens that can be shown on the OLED
enum OledScreen {
  SCREEN_SPLASH,      // Initial splash screen with logo
  SCREEN_MAIN,        // Main menu for mode selection
  SCREEN_MODE_MANUAL, // Manual control mode screen
  SCREEN_MODE_AUTO    // Automatic control mode screen
};

OledScreen oledScreen = SCREEN_SPLASH;  // Current active screen
int mainMenuSelection = 0;              // Selected item in main menu (0=Manual, 1=Auto)
int currentGaugeScreen = 0;             // Current gauge display (0=CO2/PWM, 1=Humidity/Temp)

// Automatic Mode Warmup Variables
// In auto mode, system warms up sensor before starting ventilation
bool warmingUp = false;
unsigned long warmupStart = 0;
const unsigned long warmupDuration = 120000; // 2 minutes warmup period

// Task Scheduling Variables
// Using non-blocking delays for multitasking
unsigned long lastWebSocketUpdate = 0;
const unsigned long wsUpdateInterval = 1000;    // WebSocket update every 1 second
unsigned long lastOLEDUpdate = 0;
const unsigned long oledUpdateInterval = 500;   // OLED refresh every 500ms
unsigned long lastSensorRead = 0;
const unsigned long sensorReadInterval = 1000;  // Sensor read every 1 second
unsigned long lastServerHandle = 0;
const unsigned long serverHandleInterval = 10;  // Handle web requests every 10ms

// ==========================
// Data Logging System for CSV Export
// Stores sensor readings for later download
// ==========================
struct DataRecord {
  unsigned long timestamp;  // Time since recording started (milliseconds)
  float co2;                // CO2 reading in ppm
  float temperature;        // Temperature in Celsius
  float humidity;           // Relative humidity in %
  int pwm;                  // PWM value at time of recording
};

std::vector<DataRecord> dataLog;               // Dynamic array to store data records
const size_t MAX_DATA_RECORDS = 1000;          // Maximum records to prevent memory overflow
unsigned long lastDataLog = 0;                 // Timestamp of last data log
const unsigned long dataLogInterval = 2000;    // Log data every 2 seconds
unsigned long systemStartTime = 0;             // System start timestamp for relative timing

// Logging Control Variables
bool loggingEnabled = true;     // Flag to enable/disable data logging
bool dataClearedFlag = false;   // Flag indicating data was cleared (for restart logic)

// HTML Web Interface
// Embedded HTML/CSS/JavaScript for the web dashboard
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>Aether</title>
  <style>
    /* CSS styling for clean, responsive interface */
    body{font-family:system-ui,Arial,sans-serif;margin:20px;line-height:1.6;background:#f5f5f5}
    h1{font-size:1.6rem;margin-bottom:0.25rem;color:#333}
    h2{font-size:1.3rem;margin-bottom:10px;color:#555}
    
    /* Layout containers */
    .container{max-width:1400px;margin:0 auto}
    .columns-wrapper{display:flex;gap:20px;align-items:flex-start}
    .left-column{flex:1;min-width:300px}
    .right-column{flex:1;min-width:400px}
    
    .row{display:flex;gap:12px;align-items:center;margin:8px 0}
    .label{min-width:110px;color:#444}
    .val{font-weight:700;font-size:1.25rem}
    #status{font-weight:700;padding:4px 8px;border-radius:4px}
    #status.connected{background:#d4edda;color:#155724}
    #status.disconnected{background:#f8d7da;color:#721c24}
    .card{padding:16px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);margin-bottom:20px;background:white}
    canvas{border:1px solid #ddd;background:white;width:100%;height:250px;display:block;margin-top:10px}
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
    .gauges-container{display:flex;gap:20px;justify-content:center;margin-top:20px;flex-wrap:wrap}
    .gauge-wrapper{text-align:center}
    .gauge-canvas{border:1px solid #ddd;background:white;width:140px;height:140px;display:block;margin:0 auto}
    .gauge-label{font-size:0.9rem;color:#555;margin-top:5px}
    .gauge-value{font-size:1.1rem;font-weight:bold;color:#333;margin-top:3px}
    
    /* Responsive design */
    @media (max-width: 992px) {
      .columns-wrapper{flex-direction:column}
      .left-column, .right-column{width:100%;min-width:unset}
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="columns-wrapper">
      <!-- Left Column -->
      <div class="left-column">
        <!-- Main data display card -->
        <div class="card">
          <h1>Aether <span id="modeIndicator" class="mode-indicator"></span></h1>
          <p>Status: <span id="status" class="disconnected">Disconnected</span>
             <span id="loggingStatus" class="logging-status"></span></p>
          
          <!-- Statistics panel -->
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
          
          <!-- Control buttons -->
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

        <!-- Gauges display card -->
        <div class="card">
          <h2>Gauges</h2>
          <div class="gauges-container">
            <div class="gauge-wrapper">
              <canvas id="co2Gauge" class="gauge-canvas"></canvas>
              <div class="gauge-label">CO₂</div>
              <div class="gauge-value" id="co2GaugeValue">— ppm</div>
            </div>
            <div class="gauge-wrapper">
              <canvas id="tempGauge" class="gauge-canvas"></canvas>
              <div class="gauge-label">Temperature</div>
              <div class="gauge-value" id="tempGaugeValue">— °C</div>
            </div>
            <div class="gauge-wrapper">
              <canvas id="humGauge" class="gauge-canvas"></canvas>
              <div class="gauge-label">Humidity</div>
              <div class="gauge-value" id="humGaugeValue">— %</div>
            </div>
            <div class="gauge-wrapper">
              <canvas id="pwmGauge" class="gauge-canvas"></canvas>
              <div class="gauge-label">PWM</div>
              <div class="gauge-value" id="pwmGaugeValue">—</div>
            </div>
          </div>
        </div>
      </div>
      
      <!-- Right Column -->
      <div class="right-column">
        <!-- CO2 history chart -->
        <div class="card">
          <h2>CO₂ History (Last 60 seconds)</h2>
          <canvas id="co2Chart"></canvas>
          <div class="info">
            Range: 0-900 ppm | Updates every second
          </div>
        </div>
      </div>
    </div>
  </div>
   
  <script>
    /* JavaScript for WebSocket communication and real-time updates */
    (function(){
      // DOM element references
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
      
      // Gauge setup
      class Gauge {
        constructor(canvasId, minVal, maxVal, unit, color) {
          this.canvas = document.getElementById(canvasId);
          this.ctx = this.canvas.getContext('2d');
          this.minVal = minVal;
          this.maxVal = maxVal;
          this.unit = unit;
          this.color = color || '#4CAF50';
          this.value = 0;
          
          // Setup canvas size
          this.canvas.width = 140;
          this.canvas.height = 140;
          
          this.centerX = 70;
          this.centerY = 80;
          this.radius = 45;
        }
        
        draw(value) {
          this.value = value;
          const ctx = this.ctx;
          
          // Clear canvas
          ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
          
          // Calculate angle
          const range = this.maxVal - this.minVal;
          const normalizedValue = Math.max(0, Math.min(1, (value - this.minVal) / range));
          
          // Angles: 225° to -45° (270° total sweep)
          const startAngle = 225 * Math.PI / 180;
          const endAngle = -45 * Math.PI / 180;
          const angleRange = endAngle - startAngle;
          const currentAngle = startAngle + (angleRange * normalizedValue);
          
          // Draw background arc
          ctx.strokeStyle = '#e0e0e0';
          ctx.lineWidth = 8;
          ctx.lineCap = 'round';
          ctx.beginPath();
          ctx.arc(this.centerX, this.centerY, this.radius, startAngle, endAngle);
          ctx.stroke();
          
          // Draw value arc
          ctx.strokeStyle = this.color;
          ctx.lineWidth = 8;
          ctx.beginPath();
          ctx.arc(this.centerX, this.centerY, this.radius, startAngle, currentAngle);
          ctx.stroke();
          
          // Draw scale ticks
          ctx.strokeStyle = '#666';
          ctx.lineWidth = 1;
          for (let i = 0; i <= 10; i++) {
            const angle = startAngle + (angleRange * i / 10);
            const x1 = this.centerX + Math.cos(angle) * (this.radius - 10);
            const y1 = this.centerY + Math.sin(angle) * (this.radius - 10);
            const x2 = this.centerX + Math.cos(angle) * (this.radius - 5);
            const y2 = this.centerY + Math.sin(angle) * (this.radius - 5);
            
            ctx.beginPath();
            ctx.moveTo(x1, y1);
            ctx.lineTo(x2, y2);
            ctx.stroke();
          }
          
          // Draw major ticks
          ctx.lineWidth = 2;
          for (let i = 0; i <= 4; i++) {
            const angle = startAngle + (angleRange * i / 4);
            const x1 = this.centerX + Math.cos(angle) * (this.radius - 12);
            const y1 = this.centerY + Math.sin(angle) * (this.radius - 12);
            const x2 = this.centerX + Math.cos(angle) * (this.radius - 5);
            const y2 = this.centerY + Math.sin(angle) * (this.radius - 5);
            
            ctx.beginPath();
            ctx.moveTo(x1, y1);
            ctx.lineTo(x2, y2);
            ctx.stroke();
          }
          
          // Draw needle
          ctx.strokeStyle = '#333';
          ctx.lineWidth = 3;
          ctx.lineCap = 'round';
          const needleX = this.centerX + Math.cos(currentAngle) * (this.radius - 15);
          const needleY = this.centerY + Math.sin(currentAngle) * (this.radius - 15);
          ctx.beginPath();
          ctx.moveTo(this.centerX, this.centerY);
          ctx.lineTo(needleX, needleY);
          ctx.stroke();
          
          // Draw center cap
          ctx.fillStyle = '#333';
          ctx.beginPath();
          ctx.arc(this.centerX, this.centerY, 4, 0, 2 * Math.PI);
          ctx.fill();
          
          // Draw min/max labels
          ctx.fillStyle = '#666';
          ctx.font = '10px sans-serif';
          ctx.textAlign = 'center';
          
          // Min label
          const minX = this.centerX + Math.cos(startAngle) * (this.radius + 15);
          const minY = this.centerY + Math.sin(startAngle) * (this.radius + 15);
          ctx.fillText(this.minVal.toString(), minX, minY);
          
          // Max label
          const maxX = this.centerX + Math.cos(endAngle) * (this.radius + 15);
          const maxY = this.centerY + Math.sin(endAngle) * (this.radius + 15);
          ctx.fillText(this.maxVal.toString(), maxX, maxY);
        }
      }
      
      // Create gauge instances
      const co2Gauge = new Gauge('co2Gauge', 0, 2000, 'ppm', '#FF6B6B');
      const tempGauge = new Gauge('tempGauge', 0, 50, '°C', '#4ECDC4');
      const humGauge = new Gauge('humGauge', 0, 100, '%', '#45B7D1');
      const pwmGauge = new Gauge('pwmGauge', 0, 255, '', '#FFA07A');
      
      // Initialize gauges
      co2Gauge.draw(0);
      tempGauge.draw(0);
      humGauge.draw(0);
      pwmGauge.draw(0);
      
      // Chart setup
      const canvas = document.getElementById('co2Chart');
      const ctx = canvas.getContext('2d');
      const maxDataPoints = 60;
      const co2Data = [];
      
      const chartPadding = 40;
      const minCO2 = 0;
      const maxCO2 = 900;
      
      let sessionStartTime = Date.now();
      
      // Update session timer display
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
      
      // Canvas resize handler
      function resizeCanvas() {
        const rect = canvas.getBoundingClientRect();
        canvas.width = rect.width;
        canvas.height = rect.height;
      }
      
      // Draw CO2 history chart
      function drawChart() {
        resizeCanvas();
        const width = canvas.width;
        const height = canvas.height;
        ctx.clearRect(0, 0, width, height);
        
        // Draw axes
        ctx.strokeStyle = '#ccc';
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(chartPadding, chartPadding);
        ctx.lineTo(chartPadding, height - chartPadding);
        ctx.lineTo(width - chartPadding, height - chartPadding);
        ctx.stroke();
        
        // Draw Y-axis labels
        ctx.fillStyle = '#666';
        ctx.font = '11px sans-serif';
        ctx.textAlign = 'right';
        ctx.textBaseline = 'middle';
        
        const step = 100;
        for (let value = minCO2; value <= maxCO2; value += step) {
          const normalized = (value - minCO2) / (maxCO2 - minCO2);
          const y = height - chartPadding - (height - 2 * chartPadding) * normalized;
          
          ctx.fillText(value.toFixed(0), chartPadding - 5, y);
          
          // Draw grid lines
          if (value !== minCO2 && value !== maxCO2) {
            ctx.strokeStyle = '#f0f0f0';
            ctx.beginPath();
            ctx.moveTo(chartPadding, y);
            ctx.lineTo(width - chartPadding, y);
            ctx.stroke();
          }
        }
        
        // Draw CO2 data line
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
          
          // Draw last points as dots
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
        
        // Show waiting message if no data
        if (co2Data.length === 0) {
          ctx.fillStyle = '#999';
          ctx.font = '14px sans-serif';
          ctx.textAlign = 'center';
          ctx.textBaseline = 'middle';
          ctx.fillText('Waiting for data...', width / 2, height / 2);
        }
      }
      
      drawChart();
      
      // WebSocket connection setup
      const wsUrl = 'ws://' + location.hostname + ':81/';
      let ws;
      let reconnectTimer;
      let reconnectAttempts = 0;
      const maxReconnectAttempts = 10;
      const reconnectDelay = 2000;
       
      // Connect to WebSocket server
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
          
          // WebSocket event handlers
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
              // Update display with received data
              if (d.co2 !== undefined) {
                const co2Value = parseFloat(d.co2);
                co2El.textContent = co2Value.toFixed(0);
                co2Data.push(co2Value);
                if (co2Data.length > maxDataPoints) co2Data.shift();
                drawChart();
                
                // Update CO2 gauge
                co2Gauge.draw(co2Value);
                document.getElementById('co2GaugeValue').textContent = co2Value.toFixed(0) + ' ppm';
              }
              if (d.temp !== undefined) {
                const tempValue = parseFloat(d.temp);
                tempEl.textContent = tempValue.toFixed(1);
                
                // Update temperature gauge
                tempGauge.draw(tempValue);
                document.getElementById('tempGaugeValue').textContent = tempValue.toFixed(1) + ' °C';
              }
              if (d.hum !== undefined) {
                const humValue = parseFloat(d.hum);
                humEl.textContent = humValue.toFixed(1);
                
                // Update humidity gauge
                humGauge.draw(humValue);
                document.getElementById('humGaugeValue').textContent = humValue.toFixed(1) + ' %';
              }
              if (d.pwm !== undefined) {
                const pwmValue = parseInt(d.pwm);
                pwmEl.textContent = pwmValue;
                
                // Update PWM gauge
                pwmGauge.draw(pwmValue);
                document.getElementById('pwmGaugeValue').textContent = pwmValue;
              }
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
      
      // Button click handlers
      window.downloadCSV = function() {
        const link = document.createElement('a');
        link.href = '/download';
        link.download = 'Aether_data.csv';
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
      
      // Refresh statistics from server
      window.refreshStats = function() {
        fetch('/stats')
          .then(r => r.json())
          .then(data => {
            recordCountEl.textContent = data.totalRecords + '/' + data.maxRecords;
            avgCO2El.textContent = data.avgCO2 ? data.avgCO2.toFixed(0) + ' ppm' : '—';
            avgTempEl.textContent = data.avgTemp ? data.avgTemp.toFixed(1) + '°C' : '—';
            updateSessionTime(data.uptime);
            
            // Update logging status indicator
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
       
      // Initialize connection and start periodic updates
      connect();
      window.addEventListener('resize', drawChart);
      setInterval(refreshStats, 2000);
    })();
  </script>
</body>
</html>
)rawliteral";

/**
 * Format timestamp into readable time string
 * @param timestamp - Time in milliseconds
 * @return Formatted string as DD:HH:MM:SS
 */
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

/**
 * Log current sensor data to memory
 * Called periodically based on dataLogInterval
 */
void logData() {
  // Only log if sensor data is valid and logging is enabled
  if (!sensorDataValid || !loggingEnabled) return;
  
  // Create new data record
  DataRecord record;
  record.timestamp = millis() - systemStartTime;
  record.co2 = lastCO2;
  record.temperature = lastTemp;
  record.humidity = lastHum;
  record.pwm = pwmValue;
  
  // Add to log vector
  dataLog.push_back(record);
  
  // Remove oldest record if we exceed max size
  if (dataLog.size() > MAX_DATA_RECORDS) {
    dataLog.erase(dataLog.begin());
  }
  
  // Debug output
  Serial.printf("Data logged #%d: CO2=%.0f, Temp=%.1f, Hum=%.1f, PWM=%d, Time=%s\n", 
                dataLog.size(), record.co2, record.temperature, record.humidity, 
                record.pwm, getFormattedTime(record.timestamp).c_str());
}

/**
 * Generate CSV file content from logged data
 * @return String containing CSV formatted data
 */
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

/**
 * HTTP handler for CSV file download
 * Sends logged data as downloadable CSV file
 */
void handleDownloadCSV() {
  String csv = generateCSV();
  
  // Set appropriate headers for file download
  server.sendHeader("Content-Type", "text/csv");
  server.sendHeader("Content-Disposition", "attachment; filename=\"scd30_data.csv\"");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/csv", csv);
  
  Serial.println("CSV file downloaded");
}

/**
 * HTTP handler for statistics endpoint
 * Returns JSON with current statistics and system status
 */
void handleStats() {
  StaticJsonDocument<512> doc;
  doc["totalRecords"] = dataLog.size();
  doc["maxRecords"] = MAX_DATA_RECORDS;
  doc["logInterval"] = dataLogInterval / 1000;
  doc["uptime"] = (millis() - systemStartTime) / 1000;
  doc["loggingEnabled"] = loggingEnabled;
  
  // Calculate statistics if data exists
  if (!dataLog.empty()) {
    float avgCO2 = 0, avgTemp = 0, avgHum = 0, avgPWM = 0;
    float minCO2 = dataLog[0].co2, maxCO2 = dataLog[0].co2;
    float minTemp = dataLog[0].temperature, maxTemp = dataLog[0].temperature;
    
    // Calculate averages and find min/max values
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

/**
 * HTTP handler to clear all logged data
 * Resets system time and stops logging
 */
void handleClearData() {
  dataLog.clear();
  systemStartTime = millis();
  lastDataLog = millis();
  dataClearedFlag = true;
  loggingEnabled = false;
  
  server.send(200, "text/plain", "Data cleared successfully. Time reset.");
  Serial.println("Data log cleared and time reset");
}

/**
 * HTTP handler to toggle logging on/off
 * Manages the start/stop state of data recording
 */
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

/**
 * HTTP handler to start logging
 * Ensures logging is active and resets timing if needed
 */
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

/**
 * Draw a circular gauge meter on the OLED display
 * @param centerX - X coordinate of gauge center
 * @param centerY - Y coordinate of gauge center
 * @param radius - Radius of the gauge
 * @param value - Current value to display
 * @param minVal - Minimum value of the gauge range
 * @param maxVal - Maximum value of the gauge range
 * @param label - Text label for the gauge
 * @param units - Unit suffix for the value display
 */
void drawGauge(int centerX, int centerY, int radius, float value, float minVal, float maxVal, 
               const char* label, const char* units) {
  
  // Calculate angle based on value
  float range = maxVal - minVal;
  float normalizedValue = constrain((value - minVal) / range, 0.0, 1.0);
  
  // Gauges sweep from 225° (lower left) to -45° (lower right)
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
  
  // Draw needle pointer
  int needleX = centerX + cos(currentAngle) * (radius - 7);
  int needleY = centerY - sin(currentAngle) * (radius - 7);
  display.drawLine(centerX, centerY, needleX, needleY, SSD1306_WHITE);
  
  // Draw center pivot point
  display.fillCircle(centerX, centerY, 2, SSD1306_WHITE);
  
  // Display value with units
  display.setTextSize(1);
  char valueStr[16];
  if (maxVal > 100) {
    snprintf(valueStr, sizeof(valueStr), "%.0f%s", value, units);
  } else {
    snprintf(valueStr, sizeof(valueStr), "%.1f%s", value, units);
  }
  
  // Center the value text
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(valueStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(centerX - w/2, centerY + radius + 2);
  display.print(valueStr);
  
  // Display the label
  display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(centerX - w/2, centerY - radius - 8);
  display.print(label);
}

/**
 * Draw two gauge meters on a single screen
 * @param value1, min1, max1, label1, units1 - Parameters for first gauge
 * @param value2, min2, max2, label2, units2 - Parameters for second gauge
 * @param screen - Screen indicator text (e.g., "1/2" for page 1 of 2)
 */
void drawDualGauges(float value1, float min1, float max1, const char* label1, const char* units1,
                    float value2, float min2, float max2, const char* label2, const char* units2,
                    const char* screen) {
  display.clearDisplay();
  
  // Position the gauges side by side
  drawGauge(40, 32, 20, value1, min1, max1, label1, units1);
  drawGauge(91, 32, 20, value2, min2, max2, label2, units2);
  
  // Show screen indicator in top right corner
  display.setTextSize(1);
  display.setCursor(108, 0);
  display.print(screen);
  
  display.display();
}

/**
 * Update OLED display based on current screen mode
 * Handles all different display states including splash, menu, manual, and auto modes
 */
void updateOLED() {
  switch (oledScreen) {
    case SCREEN_SPLASH:
      // Display the full-screen splash bitmap (128x64)
      display.drawBitmap(0, 0, splash_bitmap, 128, 64, WHITE);
      display.display();
      break;
      
    case SCREEN_MAIN:
      // Display main menu with WiFi connection info and mode selection
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
      // Manual mode: display selected gauge screen
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
      // Auto mode: show warmup or gauge screens
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
        display.println("Hold 3s to exit");
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

/**
 * Interrupt service routine for rotary encoder rotation
 * Handles clockwise and counter-clockwise rotation detection
 */
void IRAM_ATTR handleEncoder() {
  int clkState = digitalRead(ENCODER_CLK);
  int dtState = digitalRead(ENCODER_DT);
  
  if (clkState != lastEncoded) {
    if (dtState != clkState) {
      encoderValue++;
    } else {
      encoderValue--;
    }
    
    // Constrain encoder value based on current screen context
    if (oledScreen == SCREEN_MODE_MANUAL) {
      encoderValue = constrain(encoderValue, 0, 255);
    } else if (oledScreen == SCREEN_MODE_AUTO && warmingUp) {
      // Allow PWM adjustment during warmup
      encoderValue = constrain(encoderValue, 0, 255);
    } else if (oledScreen == SCREEN_MAIN) {
      encoderValue = constrain(encoderValue, 0, 1);
    }
    
    lastEncoded = clkState;
  }
}

// HTTP handler for root URL - serves main HTML dashboard
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// HTTP handler for 404 errors
void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

/**
 * WebSocket event handler
 * Manages client connections, disconnections, and incoming messages
 */
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        sendWebSocketData();  // Send initial data to new client
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

/**
 * Broadcast sensor data to all connected WebSocket clients
 * Sends JSON formatted data including sensor readings and system mode
 */
void sendWebSocketData() {
  StaticJsonDocument<200> doc;

  doc["co2"]  = lastCO2;
  doc["temp"] = lastTemp;
  doc["hum"]  = lastHum;
  doc["pwm"]  = pwmValue;

  // Include current operating mode
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

/**
 * Configure ESP32 as WiFi Access Point
 * Sets up HTTP and WebSocket servers for client connections
 */
void setupWiFiAP() {
  Serial.println("Setting up WiFi Access Point...");
  
  // Configure WiFi in AP mode with fixed IP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), 
                    IPAddress(192, 168, 4, 1), 
                    IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);
  
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  // Configure HTTP server routes
  server.on("/", handleRoot);
  server.on("/download", handleDownloadCSV);
  server.on("/stats", handleStats);
  server.on("/clear", HTTP_POST, handleClearData);
  server.on("/startlog", HTTP_POST, handleStartLogging);
  server.on("/toggle", HTTP_POST, handleToggleLogging);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
  
  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started on port 81");
}

/**
 * System initialization
 * Configures all hardware peripherals, initializes sensors, and starts servers
 */
void setup() {
  Serial.begin(115200);
  
  systemStartTime = millis();
  
  // Configure rotary encoder pins and interrupt
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoder, CHANGE);
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (true);  // Halt if display not found
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("");
  display.display();
  
  // Configure PWM for motor control
  ledcAttach(MOTOR_PIN, 5000, 8);  // 5kHz PWM, 8-bit resolution
  ledcWrite(MOTOR_PIN, 0);         // Start with motor off
  
  // Initialize SCD30 sensor
  if (!airSensor.begin()) {
    Serial.println("SCD30 not detected!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("SCD30 not found!");
    display.display();
  } else {
    Serial.println("SCD30 ready.");
    airSensor.setMeasurementInterval(2);  // Set to 2-second intervals
  }
  
  // Reserve memory for data logging
  dataLog.reserve(MAX_DATA_RECORDS);
  
  // Start WiFi access point and web servers
  setupWiFiAP();
  
  // Show splash screen initially
  oledScreen = SCREEN_SPLASH;
  updateOLED();
}

/**
 * Main program loop
 * Implements non-blocking task scheduling for concurrent operations
 */
void loop() {
  unsigned long currentMillis = millis();
  
  // Handle HTTP and WebSocket clients (skip during splash screen)
  if (oledScreen != SCREEN_SPLASH) {
    if (currentMillis - lastServerHandle >= serverHandleInterval) {
      lastServerHandle = currentMillis;
      server.handleClient();
      webSocket.loop();
    }
  }
  
  // Read sensor data periodically
  if (currentMillis - lastSensorRead >= sensorReadInterval) {
    lastSensorRead = currentMillis;
    if (airSensor.dataAvailable()) {
      lastCO2 = airSensor.getCO2();
      lastTemp = airSensor.getTemperature();
      lastHum = airSensor.getHumidity();
      sensorDataValid = true;
    }
  }
  
  // Log data if enabled (not during splash screen)
  if (oledScreen != SCREEN_SPLASH && loggingEnabled && 
      currentMillis - lastDataLog >= dataLogInterval && sensorDataValid) {
    lastDataLog = currentMillis;
    logData();
  }
  
  // Update menu selection based on encoder in main menu
  if (oledScreen == SCREEN_MAIN) {
    mainMenuSelection = constrain(encoderValue, 0, 1);
  }
  
  // Handle encoder button press
  bool currentButtonState = digitalRead(ENCODER_SW);
  
  // Detect button press (falling edge)
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    buttonPressStartTime = currentMillis;
    buttonLongPressDetected = false;
  }
  
  // Detect long press for different purposes depending on mode
  if (currentButtonState == LOW && !buttonLongPressDetected) {
    // 3-second press to exit auto mode
    if (oledScreen == SCREEN_MODE_AUTO && 
        (currentMillis - buttonPressStartTime >= EXIT_AUTO_PRESS_TIME)) {
      buttonLongPressDetected = true;
      // Exit to main menu
      oledScreen = SCREEN_MAIN;
      warmingUp = false;
      pwmValue = 0;
      ledcWrite(MOTOR_PIN, pwmValue);
      encoderValue = mainMenuSelection;
      currentGaugeScreen = 0;
      Serial.println("Exited auto mode via 3-second press");
    }
    // 2-second press to switch gauge screens
    else if ((oledScreen == SCREEN_MODE_MANUAL || 
             (oledScreen == SCREEN_MODE_AUTO && !warmingUp)) &&
             (currentMillis - buttonPressStartTime >= LONG_PRESS_TIME)) {
      buttonLongPressDetected = true;
      currentGaugeScreen = (currentGaugeScreen + 1) % 2;  // Toggle between gauge screens
      Serial.printf("Long press detected! Screen changed to: %d\n", currentGaugeScreen);
      updateOLED(); // Update display immediately
    }
  }
  
  // Handle button release (rising edge) for normal clicks
  if (currentButtonState == HIGH && lastButtonState == LOW) {
    if (!buttonLongPressDetected && (currentMillis - lastDebounceTime) > debounceDelay) {
      // Normal click (not a long press)
      if (oledScreen == SCREEN_SPLASH) {
        // Exit splash screen to main menu
        oledScreen = SCREEN_MAIN;
        encoderValue = 0;
        mainMenuSelection = 0;
      } else if (oledScreen == SCREEN_MAIN) {
        // Enter selected mode
        oledScreen = (mainMenuSelection == 0) ? SCREEN_MODE_MANUAL : SCREEN_MODE_AUTO;
        if (oledScreen == SCREEN_MODE_AUTO) {
          // Start automatic mode with warmup
          warmingUp = true;
          warmupStart = currentMillis;
          pwmValue = 0;
          autoModePWMTarget = 255; // Default target
          encoderValue = autoModePWMTarget; // Start encoder at target value
          ledcWrite(MOTOR_PIN, pwmValue);
          currentGaugeScreen = 0;
        } else if (oledScreen == SCREEN_MODE_MANUAL) {
          // Start manual mode
          encoderValue = pwmValue;
          currentGaugeScreen = 0;
        }
      } else if (oledScreen == SCREEN_MODE_MANUAL) {
        // Return to main menu from manual mode
        oledScreen = SCREEN_MAIN;
        encoderValue = mainMenuSelection;
        currentGaugeScreen = 0;
      }
      lastDebounceTime = currentMillis;
    }
    buttonLongPressDetected = false;
  }
  
  lastButtonState = currentButtonState;
  
  // In manual mode, adjust PWM with encoder
  if (oledScreen == SCREEN_MODE_MANUAL) {
    pwmValue = constrain(encoderValue, 0, 255);
    ledcWrite(MOTOR_PIN, pwmValue);
  }
  
  // Automatic mode control logic
  if (oledScreen == SCREEN_MODE_AUTO) {
    if (warmingUp) {
      // During warmup, allow PWM target adjustment with encoder
      autoModePWMTarget = constrain(encoderValue, 0, 255);
      
      // Check if warmup period has ended
      if (currentMillis - warmupStart >= warmupDuration) {
        // End warmup period and activate ventilation with target PWM
        warmingUp = false;
        pwmValue = autoModePWMTarget;
        ledcWrite(MOTOR_PIN, pwmValue);
      }
    }
  }
  
  // Update OLED display periodically
  if (currentMillis - lastOLEDUpdate >= oledUpdateInterval) {
    lastOLEDUpdate = currentMillis;
    updateOLED();
  }
  
  // Send WebSocket data to connected clients
  if (oledScreen != SCREEN_SPLASH && 
      currentMillis - lastWebSocketUpdate >= wsUpdateInterval) {
    lastWebSocketUpdate = currentMillis;
    if (webSocket.connectedClients() > 0) {
      sendWebSocketData();
    }
  }
  
  yield();  // Allow ESP32 background tasks to run
}