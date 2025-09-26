#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// Config
#define UNO_RX 16
#define UNO_TX 17
#define UNO_RST 4

// State machine
bool usbMode = true;  // Default to USB <-> Uno
unsigned long lastBTActivity = 0;

void setup() {
  // Start ports
  Serial.begin(115200);   // USB to PC
  Serial2.begin(115200, SERIAL_8N1, UNO_RX, UNO_TX);  // UART2 to Uno

  // Bluetooth
  SerialBT.begin("ESP32-Uno-Bridge");

  // Reset pin
  pinMode(UNO_RST, OUTPUT);
  digitalWrite(UNO_RST, HIGH); // Keep Uno running
}

void loop() {
  // -------- USB Bridge Mode --------
  while (Serial.available()) {
    char c = Serial.read();
    Serial2.write(c);
  }
  while (Serial2.available()) {
    char c = Serial2.read();
    Serial.write(c);
  }

  // -------- Bluetooth Bridge Mode --------
  if (SerialBT.hasClient()) {
    // Forward BT <-> Uno
    while (SerialBT.available()) {
      char c = SerialBT.read();
      Serial2.write(c);
      lastBTActivity = millis();
    }
    while (Serial2.available()) {
      char c = Serial2.read();
      SerialBT.write(c);
    }
  }

  // Reset trigger (when Arduino IDE opens BT COM port)
  // IDE usually toggles DTR → we can fake this by pulling UNO_RST low briefly
  if (SerialBT.isReady() && (millis() - lastBTActivity < 200)) {
    digitalWrite(UNO_RST, LOW);
    delay(50);
    digitalWrite(UNO_RST, HIGH);
  }
}