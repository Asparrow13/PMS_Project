#include <WiFi.h>
#include <ThingerESP32.h>
#include "config.h"
// =====================
// PIN DEFINITIONS
// =====================
#define SOIL_SENSOR_PIN 34      // Analog pin for moisture sensor
#define PUMP_PIN 13             // Relay / transistor control pin

// =====================
// SETTINGS
// =====================
const int DRY_THRESHOLD_PERCENT = 35;  // Activate pump when moisture is below 30%
const int WET_THRESHOLD_PERCENT = 60;  // Stop pump when moisture is above 60%
const int WATERING_TIME = 3000;        // Pump ON time in ms

// Calibration values for soil moisture sensor
const int DRY_VALUE = 3000;            // ADC value when soil is completely dry
const int WET_VALUE = 1400;            // ADC value when soil is completely wet

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

int moistureValue = 0;
int moisturePercent = 0;
bool pumpState = false;

// Function to calculate moisture percentage
int getMoisturePercent(int rawValue) {
  int percent = map(rawValue, DRY_VALUE, WET_VALUE, 0, 100);
  return constrain(percent, 0, 100);
}

// Function to control pump based on moisture level
void controlPump(int moisturePercent) {
  // Turn pump ON if moisture is below dry threshold
  if (moisturePercent < DRY_THRESHOLD_PERCENT && !pumpState) {
    digitalWrite(PUMP_PIN, HIGH);  // Pump ON
    pumpState = true;
    Serial.println("*** PUMP ACTIVATED - Soil too dry! ***");
  }
  // Turn pump OFF if moisture is above wet threshold
  else if (moisturePercent >= WET_THRESHOLD_PERCENT && pumpState) {
    digitalWrite(PUMP_PIN, LOW);   // Pump OFF
    pumpState = false;
    Serial.println("*** PUMP DEACTIVATED - Soil is wet enough ***");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  // Connect WiFi + Thinger
  thing.add_wifi(SSID, SSID_PASSWORD);

  // Send sensor value to Thinger
  thing["soil_moisture"] >> outputValue(moisturePercent);

  // Send pump state to Thinger
  // thing["pump_status"] >> outputValue(pumpState);
}

void loop() {
  // thing.handle();

  // Read moisture sensor
  moistureValue = analogRead(SOIL_SENSOR_PIN);
  moisturePercent = getMoisturePercent(moistureValue);

  // Control pump based on moisture level
  controlPump(moisturePercent);

  Serial.print("Moisture Raw: ");
  Serial.print(moistureValue);
  Serial.print(" | Moisture %: ");
  Serial.print(moisturePercent);
  Serial.print("% | Pump: ");
  Serial.println(pumpState ? "ON" : "OFF");

  delay(2000); // Check every 2 seconds
}