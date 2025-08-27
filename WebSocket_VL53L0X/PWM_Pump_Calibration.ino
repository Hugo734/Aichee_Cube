#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==========================
// Pin definitions
// ==========================
#define MOTOR_PIN    18    // PWM output to pump
#define ENCODER_CLK  34
#define ENCODER_DT   35
#define ENCODER_SW   32

// ==========================
// OLED setup
// ==========================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ==========================
// Encoder variables
// ==========================
volatile int encoderValue = 0;
int lastEncoded = 0;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;

// ==========================
// PWM
// ==========================
int pwmValue = 0; // 0–255 (8-bit resolution)

// ==========================
// Functions
// ==========================
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("PWM: ");
  display.println(pwmValue);
  display.display();
}

void IRAM_ATTR handleEncoder() {
  int clkState = digitalRead(ENCODER_CLK);
  int dtState = digitalRead(ENCODER_DT);
  if (clkState != lastEncoded) {
    if (dtState != clkState) encoderValue++;
    else encoderValue--;
    encoderValue = constrain(encoderValue, 0, 255);
    lastEncoded = clkState;
  }
}

// Optional: handle encoder button to reset PWM
void handleButton() {
  bool reading = digitalRead(ENCODER_SW);
  if (reading == LOW && lastButtonState == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
    pwmValue = 0;
    encoderValue = 0;
    lastDebounceTime = millis();
  }
  lastButtonState = reading;
}

// ==========================
// Setup
// ==========================
void setup() {
  Serial.begin(115200);

  // Encoder setup
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoder, CHANGE);

  // OLED setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (true);
  }
  display.display();
  delay(500);
  display.clearDisplay();

  // LEDC PWM setup using ESP32 v3.3.0 API
  // Channel is auto-assigned; 20 kHz, 8-bit resolution
  ledcAttach(MOTOR_PIN, 20000, 8);
  ledcWrite(MOTOR_PIN, 0); // start pump off
}

// ==========================
// Loop
// ==========================
void loop() {
  handleButton();

  // Read encoder and update PWM
  pwmValue = encoderValue;
  ledcWrite(MOTOR_PIN, pwmValue);

  // Update OLED
  updateOLED();

  delay(100); // small update delay
}
