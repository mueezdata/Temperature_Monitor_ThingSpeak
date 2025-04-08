#include <WiFiS3.h> // For Arduino Uno R4 WiFi
#include "ThingSpeak.h"
#include "secrets.h" // Create this file to store WiFi + API credentials

WiFiClient client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
const int analogPin = A0;

void setup() {
  Serial.begin(9600);
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
  ThingSpeak.begin(client);
}

void loop() {
  int analogValue = analogRead(analogPin);
  float temperature = thermistorToCelsius(analogValue);

  Serial.print("Temperature (°C): ");
  Serial.println(temperature);

  ThingSpeak.setField(1, temperature);
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (x == 200) {
    Serial.println("Data sent to ThingSpeak");
  } else {
    Serial.println("Error sending data");
  }

  delay(60000);
}

float thermistorToCelsius(int analogValue) {
  float R1 = 10000.0; // Resistor value
  float V = analogValue * (3.3 / 1023.0);
  float R2 = (3.3 * R1 / V) - R1;

  float logR2 = log(R2);
  float A = 0.001129148;
  float B = 0.000234125;
  float C = 0.0000000876741;

  float tempK = 1.0 / (A + B * logR2 + C * logR2 * logR2 * logR2);
  return tempK - 273.15;
}
