#pragma once

// SENTINELA-SOLO — configuração central
// Autoria: Luís Brito | Copyright: Curié Edge

namespace Config {
  // Identidade do dispositivo
  constexpr char DEVICE_NAME[] = "SENTINELA-SOLO";
  constexpr char FIRMWARE_VERSION[] = "1.1.0";

  // Wi-Fi: deixe vazio para operar somente pela Serial.
  constexpr char WIFI_SSID[] = "";
  constexpr char WIFI_PASSWORD[] = "";
  constexpr char AP_SSID[] = "SENTINELA-SOLO";
  constexpr char AP_PASSWORD[] = "sentinela123"; // mínimo de 8 caracteres

  // GPIOs do ESP32 DevKit V1
  constexpr uint8_t SOIL_MOISTURE_PIN = 34; // ADC1: sensor capacitivo analógico
  constexpr uint8_t CONDUCTIVITY_PIN = 35; // ADC1: módulo de condutividade analógico
  constexpr uint8_t ONE_WIRE_PIN = 4;      // DS18B20 + resistor de 4,7 kΩ para 3V3

  // Intervalo de amostragem
  constexpr uint32_t SAMPLE_INTERVAL_MS = 5000;
  constexpr uint8_t ANALOG_SAMPLES = 8;

  // Calibração da umidade: valores ADC medidos em ar/seco e em água/muito úmido.
  constexpr int MOISTURE_ADC_DRY = 3000;
  constexpr int MOISTURE_ADC_WET = 1300;

  // Calibração simplificada da condutividade.
  // Ajuste EC_SLOPE e EC_OFFSET com solução padrão ou um medidor de referência.
  constexpr float EC_SLOPE = 1.00f;
  constexpr float EC_OFFSET = 0.00f;
  constexpr float EC_TEMPERATURE_COEFFICIENT = 0.019f;

  // Limites para indicação no JSON/endpoint.
  constexpr float MOISTURE_LOW_PERCENT = 30.0f;
  constexpr float TEMPERATURE_LOW_C = 10.0f;
  constexpr float TEMPERATURE_HIGH_C = 40.0f;
}
