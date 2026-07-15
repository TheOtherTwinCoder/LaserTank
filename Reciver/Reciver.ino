#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define MOTOR_PIN_A 0
#define MOTOR_PIN_B 1

// EXACT structure from your working example
typedef struct struct_message {
  int id;
  float reading;
} struct_message;

struct_message myData;

void OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t *incomingData, int len) {
  // 1. MUST copy incoming data to myData first
  memcpy(&myData, incomingData, sizeof(myData));

  // 2. Correct map usage: Assign the return value to new variables
  long mappedX = map(myData.id, 0, 4095, -255, 255);
  long mappedY = map((long)myData.reading, 0, 4095, -255, 255); // Cast float to long for map()

  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("X-Axis (id): ");
  Serial.print(myData.id);
  Serial.print(" -> Mapped X: ");
  Serial.println(mappedX);
  
  Serial.print("Y-Axis (reading): ");
  Serial.print(myData.reading);
  Serial.print(" -> Mapped Y: ");
  Serial.println(mappedY);
  Serial.println();

  

  // Motor logic using raw ESP32 ADC values (0 to 4095)
  if (mappedX > 0) {
    digitalWrite(MOTOR_PIN_A, HIGH);
    digitalWrite(MOTOR_PIN_B, LOW);
  } 
  else if (myData.id < 1500) {
    digitalWrite(MOTOR_PIN_A, LOW);
    digitalWrite(MOTOR_PIN_B, HIGH);
  } 
  else {
    // Regular 'else' covers everything in the middle (1500 to 2500)
    digitalWrite(MOTOR_PIN_A, LOW);
    digitalWrite(MOTOR_PIN_B, LOW);
  }
  
  
}

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_PIN_A, OUTPUT);
  pinMode(MOTOR_PIN_B, OUTPUT);

  WiFi.mode(WIFI_STA);
  esp_wifi_set_max_tx_power(34);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Relying entirely on the OnDataRecv callback
}
