#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>
#include "config.h"

OneWire oneWire(Config::ONE_WIRE_PIN);
DallasTemperature temperatureSensor(&oneWire);
WebServer server(80);

struct SoilReading {
  uint32_t timestampMs = 0;
  int moistureRaw = 0;
  float moisturePercent = NAN;
  int conductivityRaw = 0;
  float conductivityMsCm = NAN;
  float temperatureC = NAN;
  bool temperatureValid = false;
};

SoilReading lastReading;
uint32_t lastSampleMs = 0;

float clampFloat(float value, float low, float high) {
  return value < low ? low : (value > high ? high : value);
}

int readAnalogAverage(uint8_t pin) {
  uint32_t total = 0;
  for (uint8_t index = 0; index < Config::ANALOG_SAMPLES; ++index) {
    total += analogRead(pin);
    delayMicroseconds(250);
  }
  return static_cast<int>(total / Config::ANALOG_SAMPLES);
}

float mapMoisturePercent(int raw) {
  const float span = static_cast<float>(Config::MOISTURE_ADC_DRY - Config::MOISTURE_ADC_WET);
  if (span == 0) return NAN;
  return clampFloat((Config::MOISTURE_ADC_DRY - raw) * 100.0f / span, 0.0f, 100.0f);
}

float readConductivityMsCm(int raw, float temperatureC) {
  // Conversão inicial para módulos analógicos. A calibração final depende do sensor.
  const float voltage = static_cast<float>(raw) * 3.3f / 4095.0f;
  float ec = (voltage * Config::EC_SLOPE) + Config::EC_OFFSET;
  if (!isnan(temperatureC)) {
    ec /= (1.0f + Config::EC_TEMPERATURE_COEFFICIENT * (temperatureC - 25.0f));
  }
  return ec < 0 ? 0 : ec;
}

void sampleSensors() {
  lastReading.timestampMs = millis();
  lastReading.moistureRaw = readAnalogAverage(Config::SOIL_MOISTURE_PIN);
  lastReading.moisturePercent = mapMoisturePercent(lastReading.moistureRaw);
  lastReading.conductivityRaw = readAnalogAverage(Config::CONDUCTIVITY_PIN);

  temperatureSensor.requestTemperatures();
  const float temp = temperatureSensor.getTempCByIndex(0);
  lastReading.temperatureValid = temp != DEVICE_DISCONNECTED_C && temp > -55.0f && temp < 125.0f;
  lastReading.temperatureC = lastReading.temperatureValid ? temp : NAN;
  lastReading.conductivityMsCm = readConductivityMsCm(lastReading.conductivityRaw, lastReading.temperatureC);
}

String numberOrNull(float value, unsigned int decimals = 2) {
  return isnan(value) ? "null" : String(value, decimals);
}

String readingJson() {
  String status = "ok";
  if (!lastReading.temperatureValid) status = "temperature_sensor_error";
  else if (lastReading.moisturePercent < Config::MOISTURE_LOW_PERCENT) status = "soil_dry";
  else if (lastReading.temperatureC < Config::TEMPERATURE_LOW_C || lastReading.temperatureC > Config::TEMPERATURE_HIGH_C) status = "temperature_out_of_range";

  String json = "{";
  json += "\"device\":\"" + String(Config::DEVICE_NAME) + "\",";
  json += "\"firmware\":\"" + String(Config::FIRMWARE_VERSION) + "\",";
  json += "\"uptime_ms\":" + String(millis()) + ",";
  json += "\"sampled_at_ms\":" + String(lastReading.timestampMs) + ",";
  json += "\"status\":\"" + status + "\",";
  json += "\"moisture\":{\"raw\":" + String(lastReading.moistureRaw) + ",\"percent\":" + numberOrNull(lastReading.moisturePercent) + "},";
  json += "\"temperature_c\":" + numberOrNull(lastReading.temperatureC) + ",";
  json += "\"conductivity\":{\"raw\":" + String(lastReading.conductivityRaw) + ",\"ms_cm\":" + numberOrNull(lastReading.conductivityMsCm) + "}";
  json += "}";
  return json;
}

void handleRoot() {
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html; charset=utf-8", "<h1>SENTINELA-SOLO</h1><p>Use <a href='/api/reading'>/api/reading</a> para JSON.</p>");
}

void handleReading() {
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json; charset=utf-8", readingJson());
}

void handleHealth() {
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json; charset=utf-8", "{\"device\":\"" + String(Config::DEVICE_NAME) + "\",\"status\":\"online\",\"uptime_ms\":" + String(millis()) + "}");
}

void startNetwork() {
  WiFi.mode(WIFI_STA);
  if (strlen(Config::WIFI_SSID) > 0) {
    WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASSWORD);
    Serial.printf("Conectando ao Wi-Fi %s", Config::WIFI_SSID);
    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
      delay(300);
      Serial.print('.');
    }
    Serial.println();
  }

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(Config::AP_SSID, Config::AP_PASSWORD);
    Serial.printf("Modo AP ativo: %s | IP: %s\n", Config::AP_SSID, WiFi.softAPIP().toString().c_str());
  } else {
    Serial.printf("Wi-Fi conectado | IP: %s\n", WiFi.localIP().toString().c_str());
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/reading", HTTP_GET, handleReading);
  server.on("/api/health", HTTP_GET, handleHealth);
  server.begin();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  temperatureSensor.begin();
  sampleSensors();
  startNetwork();
  Serial.println("SENTINELA-SOLO iniciado.");
}

void loop() {
  server.handleClient();
  if (millis() - lastSampleMs >= Config::SAMPLE_INTERVAL_MS) {
    lastSampleMs = millis();
    sampleSensors();
    Serial.println(readingJson());
  }
}
