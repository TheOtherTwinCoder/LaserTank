#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// ESP32-C3 Pin Mapping
const int IN1 = 5; // Motor A (Right Wheel) Input 1
const int IN2 = 6; // Motor A (Right Wheel) Input 2
const int IN3 = 7; // Motor B (Back Left Wheel) Input 1
const int IN4 = 8; // Motor B (Back Left Wheel) Input 2 (Onboard LED)

// EXACT structure from your working example
typedef struct struct_message {
  int id;
  float reading;
} struct_message;

struct_message myData;

// Global variables for motor speed & direction
volatile long mappedX = 0;
volatile long mappedY = 0;

void OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t *incomingData, int len) {
  // Copy incoming ESP-NOW data
  memcpy(&myData, incomingData, sizeof(myData));

  // Map joystick values (0 to 4095) to PWM speed range (-255 to 255)
  mappedX = map(myData.id, 0, 4095, -255, 255);
  mappedY = map((long)myData.reading, 0, 4095, -255, 255); 
}

void setup() {
  Serial.begin(115200);
  delay(1500); // Give native USB time to connect to Serial Monitor
  
  Serial.println("--- ESP32-C3 Receiver Booting Up ---");

  // Set all motor driver control pins as outputs
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Initialize WiFi & ESP-NOW
  WiFi.mode(WIFI_STA);
  esp_wifi_set_max_tx_power(34);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW Initialized Successfully!");

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // --- PRINT LIVE DATA ---
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    Serial.print("Mapped X (Left Wheel): ");
    Serial.print(mappedX);
    Serial.print("\t| Mapped Y (Right Wheel): ");
    Serial.println(mappedY);
    lastPrint = millis();
  }

  // --- MOTOR B (Back Left Wheel) - Controlled by Joystick X-Axis ---
  if (mappedX > -50 && mappedX < 50) {
    // Stop (inside joystick deadzone)
    analogWrite(IN3, 0);
    analogWrite(IN4, 0);
  } 
  else if (mappedX <= -50) {
    // Joystick Left -> Spin Forward
    analogWrite(IN3, 0);
    analogWrite(IN4, abs(mappedX));
  } 
  else if (mappedX >= 50) {
    // Joystick Right (X = 255) -> Spin Reverse (Opposite way)
    analogWrite(IN3, mappedX);
    analogWrite(IN4, 0);
  }

  // --- MOTOR A (Back Right Wheel) - Controlled by Joystick Y-Axis ---
  if (mappedY > -50 && mappedY < 50) {
    // Stop (inside joystick deadzone)
    analogWrite(IN1, 0);
    analogWrite(IN2, 0);
  } 
  else if (mappedY <= -50) {
    // Joystick Down -> Spin Reverse
    analogWrite(IN1, abs(mappedY));
    analogWrite(IN2, 0);
  } 
  else if (mappedY >= 50) {
    // Joystick Up -> Spin Forward
    analogWrite(IN1, 0);
    analogWrite(IN2, abs(mappedY));
  }

  delay(10); // Keeps the system stable
}
