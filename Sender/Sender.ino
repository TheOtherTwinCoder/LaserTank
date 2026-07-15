#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define X_PIN 0
#define Y_PIN 1
#define SW_PIN 2

// Your exact ESP32-C3 Receiver MAC Address
uint8_t receiverAddress[] = {0xD4, 0x05, 0x92, 0xE7, 0xD5, 0xD0};
// D4:05:92:E7:D5:D0

// EXACT structure from your working example
typedef struct struct_message {
  int id;
  float reading;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  pinMode(SW_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);

  // Matches your working Tx power setting
  esp_wifi_set_max_tx_power(34);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Your working cast fix
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));

  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  // Map joystick values into your exact working data types
  myData.id = analogRead(X_PIN);       // Storing X-axis in the integer 'id'
  myData.reading = analogRead(Y_PIN);  // Storing Y-axis in the float 'reading'

  esp_err_t result = esp_now_send(receiverAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }
  Serial.printf("X: %d  Y: %.0f\n", myData.id, myData.reading);
  delay(50); // Fast enough for responsive joystick control
}
