#include <Arduino.h>

#include <ThingerESP32.h>

#include <config.h>

#define SOIL_MOISTURE_PIN 35
#define WATER_PUMP_PIN 14
#define BUCKET_ID "moisture-data-bucket-id"
#define SEND_INTERVAL_MS 5000 // 10 seconds between bucket writes
#define MOISTURE_THRESHOLD_PERCENT 50.0f

// Calibrate these values based on your sensor's dry and wet readings.
// Measure rawValue when the sensor is fully wet and fully dry, then update.
const int SOIL_MOISTURE_RAW_WET = 1150;
const int SOIL_MOISTURE_RAW_DRY = 2500;

bool pumpOn = false;

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

float getSoilMoisturePercent(int rawValue) {
  rawValue = constrain(rawValue, SOIL_MOISTURE_RAW_WET, SOIL_MOISTURE_RAW_DRY);
  float percent = 100.0f * (float)(SOIL_MOISTURE_RAW_DRY - rawValue) /
                    (float)(SOIL_MOISTURE_RAW_DRY - SOIL_MOISTURE_RAW_WET);
  return roundf(percent);
}

void setPumpState(bool state) {
  pumpOn = state;
  digitalWrite(WATER_PUMP_PIN, pumpOn ? HIGH : LOW);
}

void updateWaterPump(float moisturePercent) {
  bool shouldRun = moisturePercent < MOISTURE_THRESHOLD_PERCENT;
  if (shouldRun != pumpOn) {
    setPumpState(shouldRun);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);
  setPumpState(false);

  // Add WiFi network
  thing.add_wifi(SSID, SSID_PASSWORD);

  // Define soil moisture resource for Thinger and bucket writes
  thing["soil_moisture"] >> [](pson& out){
    int rawValue = analogRead(SOIL_MOISTURE_PIN);
    float moisturePercent = getSoilMoisturePercent(rawValue);

    out["raw"] = rawValue;
    out["percent"] = moisturePercent;
  };
}

void loop() {
  thing.handle();

  static unsigned long lastSend = 0;
  if (millis() - lastSend > SEND_INTERVAL_MS) {
    int rawValue = analogRead(SOIL_MOISTURE_PIN);
    float moisturePercent = getSoilMoisturePercent(rawValue);

    Serial.print("Soil moisture raw: ");
    Serial.print(rawValue);
    Serial.print("  percent: ");
    Serial.print(moisturePercent, 0);
    Serial.print(" %  pump: ");
    Serial.println(pumpOn ? "ON" : "OFF");

    updateWaterPump(moisturePercent);

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
