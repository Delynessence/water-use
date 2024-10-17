#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TimeLib.h>  // Library for time handling

int led = 18;
int ledWifi = 5;
Servo servoMotor;  // Define servo motor
int servoPin = 21;  // Define the servo pin

const char* ssid = "Magister";  // Ganti dengan SSID Wi-Fi
const char* password = "R!$np4n1viz";  // Ganti dengan password Wi-Fi
const char* serverName = "https://script.google.com/macros/s/AKfycbzp7dlKrN37DDqCkKjmuoDl3rlQwymAto03igGOXpEBsFu9hc_kAlv_N2PzqRcZA9-VEQ/exec";  // Ganti dengan URL Google Apps Script kamu

const int flowSensorPin = 19;  // Ganti sesuai pin yang kamu gunakan
volatile int flowPulseCount = 0;
float flowRate = 0.0;
float totalLiters = 0.0;
float totalCost = 0.0;

// Daily usage limit in liters
const float dailyWaterLimit = 0.5;  // Set daily water usage limit
bool waterShutOff = false;  // Flag for water shutoff

// Variables for time
int shutoffHour = 4;  // Time to reopen the valve (6 AM)
unsigned long oldTime = 0;
float calibrationFactor = 1.6;  // Nilai kalibrasi baru berdasarkan hasil pengukuran
unsigned long wifiReconnectTime = 0;
const unsigned long wifiReconnectInterval = 300000;  // 30 detik

// Fungsi interrupt untuk menghitung pulsa
void IRAM_ATTR pulseCounter() {
  flowPulseCount++;
}

// Fungsi untuk menghitung tarif PDAM
float calculatePDAMCost(float totalLiters) {
  float totalCubicMeters = totalLiters / 1000.0;
  float totalCost = 0.0;

  if (totalCubicMeters <= 10) {
    totalCost = totalCubicMeters * 3500;
  } else if (totalCubicMeters <= 20) {
    totalCost = (10 * 3500) + ((totalCubicMeters - 10) * 4500);
  } else {
    totalCost = (10 * 3500) + (10 * 4500) + ((totalCubicMeters - 20) * 6500);
  }

  return totalCost;
}

void setup() {
  pinMode(led, OUTPUT);
  pinMode(ledWifi, OUTPUT);
  servoMotor.attach(servoPin);  
  servoMotor.write(0);  // Valve is open at 90 degrees (adjust based on your servo)

  Serial.begin(115200);
  pinMode(flowSensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);

  digitalWrite(led, HIGH);
  connectToWiFi();
}

void loop() {
  checkWiFiStatus();

  if ((millis() - oldTime) > 1000) {
    flowRate = ((1000.0 / (millis() - oldTime)) * flowPulseCount) / calibrationFactor;

    if (flowRate > 0 && !waterShutOff) {
      totalLiters += (flowRate / 60.0);
      totalCost = calculatePDAMCost(totalLiters);

      if (totalLiters > dailyWaterLimit) {
        shutOffWater();
      }

      Serial.print("Flow Rate: ");
      Serial.print(flowRate);
      Serial.println(" L/min");
      Serial.print("Total Liters: ");
      Serial.print(totalLiters);
      Serial.println(" L");
      Serial.print("Total Cost (PDAM): Rp ");
      Serial.println(totalCost);

      sendToGoogleSheets(flowRate, totalLiters, totalCost);
    } else if (waterShutOff) {
      Serial.println("Water shutoff, no data sent.");
    }

    oldTime = millis();
    flowPulseCount = 0;
  }

  // Check if it's time to reset the daily usage
  if (hour() == shutoffHour && minute() == 0) {
    resetDailyUsage();
  }
}

// Fungsi untuk shut off water
void shutOffWater() {
  Serial.println("Daily limit exceeded, shutting off water.");
  waterShutOff = true;
  servoMotor.write(90);  // Close valve by setting servo to 0 degrees
}

// Fungsi untuk membuka aliran air keesokan hari
void resetDailyUsage() {
  Serial.println("Resetting daily usage, reopening water.");
  totalLiters = 0;
  waterShutOff = false;
  servoMotor.write(0);  // Reopen valve by setting servo to 90 degrees
}

// Fungsi untuk koneksi Wi-Fi
void connectToWiFi() {
  Serial.println("Menghubungkan ke Wi-Fi...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nTerhubung ke Wi-Fi");
    digitalWrite(ledWifi, HIGH);
  } else {
    Serial.println("\nGagal terhubung ke Wi-Fi");
    digitalWrite(ledWifi, LOW);
  }
}

// Fungsi untuk cek status Wi-Fi
void checkWiFiStatus() {
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ledWifi, LOW);
    if (millis() - wifiReconnectTime > wifiReconnectInterval) {
      Serial.println("Wi-Fi terputus, mencoba untuk reconnect...");
      connectToWiFi();
      wifiReconnectTime = millis();
    }
  } else {
    digitalWrite(ledWifi, HIGH);
  }
}

void sendToGoogleSheets(float flowRate, float totalLiters, float totalCost) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{\"flowRate\": " + String(flowRate) + ", \"totalLiters\": " + String(totalLiters) + ", \"totalCost\": " + String(totalCost) + "}";

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
    digitalWrite(ledWifi, LOW);
  }
}
