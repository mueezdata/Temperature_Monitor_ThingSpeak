#include <WiFiS3.h>              // For Arduino UNO R4 WiFi
#include "secrets.h"             // WiFi credentials and ThingSpeak details
#include "ThingSpeak.h"
#include <math.h>                // For log()

// Constants for NTC thermistor
#define THERMISTOR_PIN A0        // Analog pin connected to voltage divider
#define SERIES_RESISTOR 10000.0  // 10k Ohm resistor
#define B_COEFFICIENT 3950.0     // B value of thermistor (typical: 3950)
#define NOMINAL_RESISTANCE 10000.0 // Resistance at 25°C
#define NOMINAL_TEMPERATURE 25.0   // In Celsius

// Temperature alarm thresholds
#define LOWER_TEMP_THRESHOLD 15.0
#define UPPER_TEMP_THRESHOLD 35.0

WiFiClient client;

void setup() {
  Serial.begin(115200);
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  ThingSpeak.begin(client);
}

void loop() {
  int analogValue = analogRead(THERMISTOR_PIN);

  // Convert analog value to resistance
  float resistance = SERIES_RESISTOR * (1023.0 / analogValue - 1.0);

  // Calculate temperature in Kelvin using the B-parameter equation
  float steinhart;
  steinhart = resistance / NOMINAL_RESISTANCE;
  steinhart = log(steinhart);
  steinhart /= B_COEFFICIENT;
  steinhart += 1.0 / (NOMINAL_TEMPERATURE + 273.15);
  steinhart = 1.0 / steinhart;
  float temperatureC = steinhart - 273.15;  // Convert to Celsius

  // Send temperature to ThingSpeak (Field 1)
  ThingSpeak.setField(1, temperatureC);

  // Set alarm if temperature is outside threshold (Field 2)
  int alarmStatus = (temperatureC < LOWER_TEMP_THRESHOLD || temperatureC > UPPER_TEMP_THRESHOLD) ? 1 : 0;
  ThingSpeak.setField(2, alarmStatus);

  // Write data to ThingSpeak
  int code = ThingSpeak.writeFields(SECRET_CH_ID, SECRET_WRITE_APIKEY);

  if (code == 200) {
    Serial.print("Data sent successfully. Temp: ");
    Serial.print(temperatureC);
    Serial.print(" °C | Alarm: ");
    Serial.println(alarmStatus ? "ON" : "OFF");
  } else {
    Serial.print("Error sending data. Code: ");
    Serial.println(code);
  }

  delay(15000);  // Wait 15 seconds (ThingSpeak limit)
}
