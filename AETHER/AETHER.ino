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

// ========================== 
// WiFi Access Point Configuration
// The ESP32 creates its own WiFi network that devices can connect to
// ==========================
const char* ssid = "Aether";      // Network name (SSID)
const char* password = "12345678";      // Network password (min 8 chars required)
DNSServer dnsServer; //DNSServer
const byte DNS_PORT = 53; //DNSServer


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
// This array contains the pixel data for the initial logo/graphic Aether
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
int lastEncoder = 0;                    // Previous encoder state for comparison
bool lastButtonState = HIGH;            // Previous button state for edge detection
unsigned long lastDebounceTime = 0;     // Timestamp for button debounce
unsigned long debounceDelay = 60;      // Debounce delay in milliseconds so adjust was in 200


// So before there was an implementation of pressing the encoder for 2 seconds and 6 seconds, but had some problems of priorities so we change to this new method
// ===========================================================================
// Single and Double Click Detection Variables
const unsigned long DEBOUNCE_MS = 60;
const unsigned long DBLCLICK_MS = 350; // Max time between clicks for double click

// Internal state fo dectection clicks
static bool btn_lastStable = HIGH; // INPUT_PULLUP
static bool btn_lastRead = HIGH;
static unsigned long btn_tEdge = 0;

static bool btn_pressed = false;
static int btn_clicks = 0;
static unsigned long btn_tRelease = 0;
static bool btn_waitingForDouble = false;
// ===========================================================================


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
  SCREEN_QR,          // QR to sing in to the red of the AETHER
  SCREEN_MAIN,        // Main menu for mode selection
  SCREEN_MODE_MANUAL, // Manual control mode screen
  SCREEN_MODE_AUTO    // Automatic control mode screen
};

// Here I will change to first have the QR screen, and after you connect a device, the logo of the cube starts the SCREEN_SPLASH
//OledScreen oledScreen = SCREEN_QR; // First thing


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
//const unsigned long wsUpdateInterval = 1000;    // WebSocket update every 1 second
// For the curiosity try with 5 seconsds
const unsigned long wsUpdateInterval = 1000;    // WebSocket update every 5 seconds

// Here I am thinking to reduce the data sent by calculating an average of the last 5 readings and send that average every 5 seconds

unsigned long lastOLEDUpdate = 0;
// I will consider to augment the refresh to 1 second to reduce the load of the OLED
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
  String csv;
  csv.reserve(64 + dataLog.size() * 40);
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
  }

  return csv;
}

/**
 * HTTP handler for CSV file download
 * Sends logged data as downloadable CSV file
 */

// Comment to try the implementation of the new web server
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


/**
 * HTTP handler for statistics endpoint
 * Returns JSON with current statistics and system status
 */

/*
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
    doc["avgTemp"] = updateStatsFromClient / count;
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

*/

// --------------------------------------------------------
// New handlers with improved logic
/*
void handleStats() {
  static JsonDocument<256> doc;
  doc["totalRecords"] = dataLog.size();
  doc["maxRecords"] = MAX_DATA_RECORDS;
  doc["logInterval"] = dataLogInterval / 1000;
  doc["uptime"] = (millis() - systemStartTime) / 1000;
  doc["loggingEnabled"] = loggingEnabled;

  size_t n = serializeJson(d, buf, sizeof(buf));
  server.send(200, "application/json", String(buf, n));

}
*/

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

// I will implment POO to handle the OLED gauge drawing
// To be used in the updateOLED() function

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

// Here I want to implment a FSM to handle the different OLED screens
// Starting with splash qr, screen splash, main menu, manual mode, auto mode, performance mode etc. 
void updateOLED() {
  switch (oledScreen) {
    
    /*
    case SCREEN_QR:
      // Display QR code bitmap (64x64) centered
      display.drawBitmap(32, 0, qr_bitmap, 64, 64, WHITE);
      display.display();
      //
      break;    
    */


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

/**
 * Interrupt service routine for rotary encoder rotation
 * Handles clockwise and counter-clockwise rotation detection
 */
void IRAM_ATTR handleEncoder() {
  int clkState = digitalRead(ENCODER_CLK);
  int dtState  = digitalRead(ENCODER_DT);

  if (clkState != lastEncoder) {
    if (dtState != clkState) {
      encoderValue++;
    } else {
      encoderValue--;
    }
    lastEncoder = clkState;
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
    
    lastEncoder = clkState;
  }


// HTTP handler for root URL - serves main HTML dashboard
void handleRoot() {
  server.send_P(200, "text/html", AETHER_html);
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
  StaticJsonDocument<160> doc;
  char out[160];

  doc["t"]    = millis();     // timestamp para el navegador
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

  size_t n = serializeJson(doc, out, sizeof(out));
  if (n) webSocket.broadcastTXT(out, n);
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
  
  // Configurar DNS para redirigir todas las peticiones al ESP32
  dnsServer.start(DNS_PORT, "*", IP); 
  
  // Configure HTTP server routes
  server.on("/", handleRoot);
  server.on("/download", handleDownloadCSV);
  //server.on("/stats", handleStats);
  server.on("/clear", HTTP_POST, handleClearData);
  server.on("/startlog", HTTP_POST, handleStartLogging);
  server.on("/toggle", HTTP_POST, handleToggleLogging);

  // ====== AGREGAR HANDLER PARA DETECTAR PORTAL CAUTIVO ======
  server.on("/generate_204", handleRoot);  // Android
  server.on("/gen_204", handleRoot);       // Android
  server.on("/hotspot-detect.html", handleRoot);  // iOS


  
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
  // Websocket heartbeat to avoid zombie clients
  webSocket.enableHeartbeat(15000, 3000, 2);


  // Configure rotary encoder pins and interrupt
  pinMode(ENCODER_CLK, INPUT); // Already set in handleEncoder
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  lastEncoder = digitalRead(ENCODER_CLK); // Just to prevent a false trigger at start

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
    airSensor.setAutoSelfCalibration(false); //Deactivate ASC
    airSensor.setMeasurementInterval(3);  // Set to 3-second intervals
  }
  
  // Reserve memory for data logging
  dataLog.reserve(MAX_DATA_RECORDS);
  
  // Start WiFi access point and web servers
  setupWiFiAP();
  
  // Show splash screen initially
  oledScreen = SCREEN_SPLASH;
  updateOLED();
}

/*
FUNCTIONS TO CHANGE MODES 
*/

void onSingleClick(){
    switch (oledScreen)
    {
        case  SCREEN_SPLASH:
            oledScreen = SCREEN_MAIN;
            encoderValue = 0;
            mainMenuSelection = 0;
            break;
        case SCREEN_MAIN:
            oledScreen = (mainMenuSelection == 0) ? SCREEN_MODE_MANUAL : SCREEN_MODE_AUTO;
            if (oledScreen == SCREEN_MODE_AUTO) {
                // Start automatic mode with warmup
                warmingUp = true;
                warmupStart = millis();
                pwmValue = 0;
                autoModePWMTarget = 255; // Default target   
                encoderValue = autoModePWMTarget; // Start encoder at target value
                ledcWrite(MOTOR_PIN, pwmValue);
                currentGaugeScreen = 0;
            } else {
                // Start manual mode
                encoderValue = pwmValue;
                currentGaugeScreen = 0;
            } 
            break;
        case SCREEN_MODE_MANUAL:
        case SCREEN_MODE_AUTO:
        // change gauge 1/2
            currentGaugeScreen = (currentGaugeScreen + 1) % 2;  // Toggle between gauge screens

    }
}

void onDoubleClick() {
  switch (oledScreen) {
    case SCREEN_MODE_MANUAL:
    case SCREEN_MODE_AUTO:
      // Exit to main menu from manual or auto mode
      oledScreen = SCREEN_MAIN;
      warmingUp = false;
      // pwmValue = 0; ledcWrite(MOTOR_PIN, pwmValue);
      currentGaugeScreen = 0;
      encoderValue = mainMenuSelection;
      break;
    default:
      // En SPLASH/MAIN ignore double click
      break;
  }
}

void handleModeButton() {
  bool raw = digitalRead(ENCODER_SW);

  // Debounce
  if (raw != btn_lastRead) { btn_lastRead = raw; btn_tEdge = millis(); }
  if (millis() - btn_tEdge < DEBOUNCE_MS) return;

  if (raw != btn_lastStable) {
    btn_lastStable = raw;

    if (btn_lastStable == LOW) {
      // PRESS
      btn_pressed = true;
    } else {
      // RELEASE -> count click
      if (btn_pressed) {
        btn_pressed = false;
        unsigned long now = millis();

        if (!btn_waitingForDouble) {
          // Primer clic
          btn_clicks = 1;
          btn_tRelease = now;          // guardamos tiempo del 1er release
          btn_waitingForDouble = true; // open window for 2nd click
        } else {
          // Second click: calculate BEFORE touching btn_tRelease
          if (now - btn_tRelease <= DBLCLICK_MS) {
            // double click
            btn_clicks = 0;
            btn_waitingForDouble = false;
            onDoubleClick();
          } else {
            // Too late: treat as single of the previous and open new window
            onSingleClick();
            btn_clicks = 1;
            btn_tRelease = now;
            btn_waitingForDouble = true;
          }
        }
      }
    }
  }

  // The window closed and there was only one click -> single
  if (btn_waitingForDouble && (millis() - btn_tRelease > DBLCLICK_MS)) {
    if (btn_clicks == 1) onSingleClick();
    btn_clicks = 0;
    btn_waitingForDouble = false;
  }
}



/**
 * Main program loop
 * Implements non-blocking task scheduling for concurrent operations
 */
void loop() {
  unsigned long currentMillis = millis();

  dnsServer.processNextRequest();

  // Button encoder: single and double click detection
  handleModeButton();

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
    // Update PWM based on mode and encoder value
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

    // 7) OLED
    if (currentMillis - lastOLEDUpdate >= oledUpdateInterval) {
        lastOLEDUpdate = currentMillis;
        updateOLED();
    }

    // 8) WS broadcast
    if (oledScreen != SCREEN_SPLASH && 
        currentMillis - lastWebSocketUpdate >= wsUpdateInterval) {
        lastWebSocketUpdate = currentMillis;
        if (webSocket.connectedClients() > 0) {
        sendWebSocketData();
        } 
    }

    yield();
    }  
