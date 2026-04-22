#include <Arduino.h>

#include <ThingerESP32.h>

#include <config.h>

#define SOIL_MOISTURE_PIN 35
#define BUCKET_ID "moisture-data-bucket-id"
#define SEND_INTERVAL_MS 60000 // 60 seconds between bucket writes

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(SOIL_MOISTURE_PIN, INPUT);

  // Add WiFi network
  thing.add_wifi(SSID, SSID_PASSWORD);

  // Define soil moisture resource for Thinger and bucket writes
  thing["soil_moisture"] >> [](pson& out){
    int rawValue = analogRead(SOIL_MOISTURE_PIN);
    float moisturePercent = (rawValue / 4095.0f) * 100.0f;
    moisturePercent = roundf(moisturePercent); // Round to 0 decimal places

    out["raw"] = rawValue;
    out["percent"] = moisturePercent;
  };
}

void loop() {
  thing.handle();

  static unsigned long lastSend = 0;
  if (millis() - lastSend > SEND_INTERVAL_MS) {
    int rawValue = analogRead(SOIL_MOISTURE_PIN);
    float moisturePercent = (rawValue / 4095.0f) * 100.0f;
    moisturePercent = roundf(moisturePercent); // Round to 0 decimal places

    Serial.print("Soil moisture raw: ");
    Serial.print(rawValue);
    Serial.print("  percent: ");
    Serial.print(moisturePercent, 0);
    Serial.println(" %");

    if (thing.is_connected()) {
      if (thing.write_bucket(BUCKET_ID, "soil_moisture")) {
        Serial.println("Bucket write succeeded.");
      } else {
        Serial.println("Bucket write failed.");
      }
    } else {
      Serial.println("Not connected to Thinger.io; skipping bucket write.");
    }

    lastSend = millis();
  }
}
