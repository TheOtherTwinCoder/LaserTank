#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// ESP32-C3 Pin Mapping
const int IN1 = 0; // Motor A (Right Wheel) Input 1
const int IN2 = 1; // Motor A (Right Wheel) Input 2
const int IN3 = 2; // Motor B (Back Left Wheel) Input 1
const int IN4 = 3; // Motor B (Back Left Wheel) Input 2 (Onboard LED)

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

// Helper: drive a motor given a signed speed (-255..255)
void driveMotor(int posPin, int negPin, long speed) {
  if (speed > 255) speed = 255;
  if (speed < -255) speed = -255;

  if (speed > 0) {
    analogWrite(posPin, speed);
    analogWrite(negPin, 0);
  } else if (speed < 0) {
    analogWrite(posPin, 0);
    analogWrite(negPin, abs(speed));
  } else {
    analogWrite(posPin, 0);
    analogWrite(negPin, 0);
  }
}

void loop() {
  // --- PRINT LIVE DATA ---
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    Serial.print("Mapped X (Turn): ");
    Serial.print(mappedX);
    Serial.print("\t| Mapped Y (Forward/Back): ");
    Serial.println(mappedY);
    lastPrint = millis();
  }

  // Deadzone
  long x = (mappedX > -50 && mappedX < 50) ? 0 : mappedX;
  long y = (mappedY > -50 && mappedY < 50) ? 0 : mappedY;

  // Arcade-drive mixing:
  // Push back  -> y negative -> both motors backward
  // Push fwd   -> y positive -> both motors forward
  // Push left  -> x negative -> Motor A forward, Motor B backward
  // Push right -> x positive -> Motor A backward, Motor B forward
  long speedA = y - x; // Motor A (IN1/IN2)
  long speedB = y + x; // Motor B (IN3/IN4)

  // Motor A: IN2 = forward pin, IN1 = reverse pin (matches your original Up->Forward logic)
  driveMotor(IN2, IN1, speedA);

  // Motor B: IN4 = forward pin, IN3 = reverse pin (matches your original Left->Forward logic)
  driveMotor(IN4, IN3, speedB);

  delay(10); // Keeps the system stable
}
