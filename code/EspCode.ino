/*
 * SMART DOOR LOCK – ESP32-S3-N16R8
 * Keypad + PIR + Relays + Blynk
 */

#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#include <Adafruit_NeoPixel.h>

// ================== PINS ==================
#define SERVO_PIN   8
#define PIR_PIN     7
#define BUZZER_PIN  6
#define RGB_PIN     48  // Built-in RGB LED on ESP32-S3-N16R8

#define RELAY1_PIN  4
#define RELAY2_PIN  5
#define RELAY3_PIN  15
#define RELAY4_PIN  16

// ================== WIFI ==================
char ssid[] = "Reyan's A51";
char pass[] = "11223344";

// ================== SYSTEM ==================
const String PASSWORD = "1111";
const int SERVO_LOCKED = 0;
const int SERVO_UNLOCKED = 90;
const int MOTION_TIMEOUT = 30000;

// ================== OBJECTS ==================
Servo doorServo;
BlynkTimer timer;
Adafruit_NeoPixel rgbLed(1, RGB_PIN, NEO_GRB + NEO_KHZ800);

// ================== KEYPAD ==================
char keys[4][4] = {
 {'1','2','3','A'},
 {'4','5','6','B'},
 {'7','8','9','C'},
 {'*','0','#','D'}
};

byte rowPins[4] = {17,18,21,38};
byte colPins[4] = {39,40,41,42};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, 4, 4);

// ================== STATE ==================
bool locked = true;
bool relay[4] = {0,0,0,0};
String keyInput = "";
unsigned long lastMotion = 0;
bool checked = false;
bool motionDetected = false;
bool lastPirState = false;
unsigned long pirDebounceTimer = 0;
const int PIR_DEBOUNCE = 2000; // 2 second debounce

// ================== NON-BLOCKING BUZZER ==================
unsigned long buzzerTimer = 0;
int buzzerCount = 0;
bool buzzerState = false;

void beep(int times) {
  buzzerCount = times * 2; // each ON/OFF counts as 1
}

void handleBuzzer() {
  if (buzzerCount == 0) return;
  if (millis() - buzzerTimer > 80) {
    buzzerTimer = millis();
    buzzerState = !buzzerState;
    digitalWrite(BUZZER_PIN, buzzerState);
    buzzerCount--;
  }
}

// ================== RGB LED FUNCTIONS ==================
void setRGB(uint8_t r, uint8_t g, uint8_t b) {
  rgbLed.setPixelColor(0, rgbLed.Color(r, g, b));
  rgbLed.show();
}

void rgbOff() {
  setRGB(0, 0, 0);
}

void rgbRed() {
  setRGB(255, 0, 0);  // Red = Locked
}

void rgbGreen() {
  setRGB(0, 255, 0);  // Green = Unlocked
}

void rgbBlue() {
  setRGB(0, 0, 255);  // Blue = Motion detected
}

void rgbYellow() {
  setRGB(255, 255, 0);  // Yellow = Appliances left on
}

void rgbPurple() {
  setRGB(128, 0, 128);  // Purple = Wrong password
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting Smart Door Lock...");

  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);

  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  digitalWrite(RELAY4_PIN, HIGH);

  doorServo.attach(SERVO_PIN);
  doorServo.write(SERVO_LOCKED);

  // Initialize RGB LED
  rgbLed.begin();
  rgbLed.setBrightness(50);  // Set brightness to 50/255
  rgbRed();  // Start with red (locked)

  // Connect to Blynk
  // Start WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");

  // Wait up to 10 seconds to connect
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.println("\nWiFi connected!");
  } else {
    Serial.println("\nFailed to connect WiFi. Continuing offline...");
  }

  // Start Blynk in non-blocking mode
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  // Set safe timer intervals
  timer.setInterval(200L, checkKeypad);   // keypad every 200ms
  timer.setInterval(250L, checkMotion);   // motion every 250ms for better responsiveness

  Blynk.virtualWrite(V8, "System booted");
  beep(2);
}

// ================== LOOP ==================
void loop() {
  Blynk.run();
  timer.run();
  handleBuzzer();
  yield();  // watchdog safe
}

// ================== DOOR ==================
void lockDoor() {
  locked = true;
  doorServo.write(SERVO_LOCKED);
  Blynk.virtualWrite(V1, "LOCKED");
  Blynk.virtualWrite(V8, "🔒 Door locked");
  Blynk.logEvent("door_locked", "Door locked");
  checked = false;
  rgbRed();  // Red when locked
}

void unlockDoor() {
  locked = false;
  doorServo.write(SERVO_UNLOCKED);
  Blynk.virtualWrite(V1, "UNLOCKED");
  Blynk.virtualWrite(V8, "🔓 Door unlocked");
  Blynk.logEvent("door_unlocked", "Door unlocked");
  beep(2);
  rgbGreen();  // Green when unlocked
}

// ================== MOTION ==================
void checkMotion() {
  bool currentPirState = digitalRead(PIR_PIN);
  
  // Debounce PIR sensor to avoid false triggers
  if (currentPirState != lastPirState) {
    pirDebounceTimer = millis();
    lastPirState = currentPirState;
  }
  
  // Only update motion status after debounce period
  if (millis() - pirDebounceTimer > PIR_DEBOUNCE) {
    if (currentPirState && !motionDetected) {
      // Motion just started
      motionDetected = true;
      lastMotion = millis();
      Blynk.virtualWrite(V2, "MOTION");
      Blynk.virtualWrite(V8, "👁️ Motion detected!");
      Serial.println("Motion detected");
      beep(1);
      rgbBlue();  // Blue when motion detected
    } 
    else if (!currentPirState && motionDetected) {
      // Motion just stopped
      motionDetected = false;
      Blynk.virtualWrite(V2, "No Motion");
      Blynk.virtualWrite(V8, "✓ Motion stopped");
      Serial.println("Motion stopped");
      if (locked) rgbRed();  // Back to red if locked
      else rgbGreen();  // Back to green if unlocked
    }
  }
  
  // Update display even during debounce
  if (currentPirState) {
    lastMotion = millis();
    Blynk.virtualWrite(V2, "MOTION");
  } else {
    Blynk.virtualWrite(V2, "No Motion");
  }

  // Check if room has been empty long enough
  if (locked && !motionDetected && millis() - lastMotion > MOTION_TIMEOUT && !checked) {
    checked = true;
    checkAppliances();
  }
}

// ================== APPLIANCES ==================
void checkAppliances() {
  String msg = "";
  int count = 0;

  if (relay[0]) { msg += "💡 Light ON\n"; count++; }
  if (relay[1]) { msg += "🌀 Fan ON\n"; count++; }
  if (relay[2]) { msg += "❄️ AC ON\n"; count++; }
  if (relay[3]) { msg += "🔌 Extra ON\n"; count++; }

  if (count > 0) {
    Blynk.logEvent("appliances_on", msg);
    Blynk.virtualWrite(V8, msg);
    beep(3);
    rgbYellow();  // Yellow warning for appliances left on
  } else {
    Blynk.virtualWrite(V8, "✓ All appliances OFF");
    if (locked) rgbRed();  // Back to red if locked
  }
}

// ================== KEYPAD ==================
void checkKeypad() {
  char key = keypad.getKey();
  if (!key) return;

  if (key == '#') {
    if (keyInput == PASSWORD) unlockDoor();
    else {
      Blynk.logEvent("unauthorized_access", "🚨 Wrong keypad password");
      Blynk.virtualWrite(V8, "🚨 Wrong password!");
      beep(3);
      rgbPurple();  // Purple for wrong password
      delay(1000);
      if (locked) rgbRed();  // Back to red
    }
    keyInput = "";
  } else if (key == '*') {
    keyInput = "";
    Blynk.virtualWrite(V8, "Keypad cleared");
  } else if (key == 'A') {
    lockDoor();
  } else keyInput += key;
}

// ================== RELAYS ==================
void setRelay(int pin, bool st, int i) {
  digitalWrite(pin, !st);  // active LOW
  relay[i] = st;
}

// ================== BLYNK ==================
BLYNK_WRITE(V0) { param.asInt() ? unlockDoor() : lockDoor(); }

BLYNK_WRITE(V3) { setRelay(RELAY1_PIN, param.asInt(), 0); }
BLYNK_WRITE(V4) { setRelay(RELAY2_PIN, param.asInt(), 1); }
BLYNK_WRITE(V5) { setRelay(RELAY3_PIN, param.asInt(), 2); }
BLYNK_WRITE(V6) { setRelay(RELAY4_PIN, param.asInt(), 3); }

BLYNK_WRITE(V7) {
  if (param.asInt()) {
    for (int i = 0; i < 4; i++) relay[i] = 0;
    digitalWrite(RELAY1_PIN, HIGH);
    digitalWrite(RELAY2_PIN, HIGH);
    digitalWrite(RELAY3_PIN, HIGH);
    digitalWrite(RELAY4_PIN, HIGH);
    Blynk.virtualWrite(V3, 0);
    Blynk.virtualWrite(V4, 0);
    Blynk.virtualWrite(V5, 0);
    Blynk.virtualWrite(V6, 0);
    Blynk.virtualWrite(V8, "🔴 All appliances OFF");
  }
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V0, V3, V4, V5, V6);
  Blynk.virtualWrite(V8, "Connected to Blynk");
}