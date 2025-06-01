#include <Arduino.h>
#include "LIB_MAX30102.h"
#include "COMP_RITMO_CARDIACO.h"
#include "COMP_SPO2.h"

// Umbrales de detección del dedo (con histéresis)
constexpr uint32_t UMBRAL_ENTRADA = 30000;
constexpr uint32_t UMBRAL_SALIDA  = 20000;

// Parámetros para impresión por serial
constexpr uint32_t INTERVALO_SERIAL_MS = 1000;
constexpr uint8_t  ANCHO_LIMPIEZA_LINEA = 40;

// Constante para eliminación de componente DC
constexpr float FACTOR_DC = 0.95f;

// Objetos globales
MAX30102 sensor;
HeartRateProcessor procesadorHR;
SpO2Processor procesadorSpO2;

// Variables de estado
float dcIR  = 0.0f;
float dcRed = 0.0f;
float bpmValidoPrevio = 0.0f;
uint32_t tiempoUltimaSalida = 0;
bool dedoDetectado = false;

void inicializarSensor() {
  if (!sensor.begin()) {
    Serial.println(F("ERROR: Sensor MAX30102 no detectado."));
    while (true) delay(100);
  }
  sensor.setup();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  inicializarSensor();

  procesadorHR.reset();
  procesadorSpO2.reset();
  tiempoUltimaSalida = millis();
}

void loop() {
  std::vector<std::pair<uint32_t, uint32_t>> muestras;
  if (!sensor.readAllFIFO(muestras)) {
    return;  // esperar siguientes muestras
  }

  uint32_t tiempoActual = millis();

  for (const auto& muestra : muestras) {
    uint32_t rojo = muestra.first;
    uint32_t infrarrojo = muestra.second;

    // Detectar dedo con histéresis
    if (!dedoDetectado && infrarrojo > UMBRAL_ENTRADA) {
      Serial.println(F("\n-- Dedo detectado, iniciando medición --"));
      dedoDetectado = true;
      tiempoUltimaSalida = tiempoActual;
    } else if (dedoDetectado && infrarrojo < UMBRAL_SALIDA) {
      Serial.println(F("\n-- Dedo retirado, pausando --"));
      dedoDetectado = false;
      procesadorHR.reset();
      procesadorSpO2.reset();
      continue;
    }

    if (!dedoDetectado) continue;

    // Eliminación de componente DC (filtro EMA)
    dcIR  = FACTOR_DC * dcIR  + (1.0f - FACTOR_DC) * infrarrojo;
    dcRed = FACTOR_DC * dcRed + (1.0f - FACTOR_DC) * rojo;
    float acIR  = static_cast<float>(infrarrojo) - dcIR;
    float acRed = static_cast<float>(rojo) - dcRed;

    // Procesamiento de ritmo cardíaco
    bool latidoDetectado = procesadorHR.update(acIR, tiempoActual);

    // Procesamiento de SpO2
    procesadorSpO2.update(acIR, acRed, latidoDetectado);
  }

  if (dedoDetectado && (tiempoActual - tiempoUltimaSalida >= INTERVALO_SERIAL_MS)) {
    float bpmCalculado = procesadorHR.getBPM();
    if (bpmCalculado >= 40.0f && bpmCalculado <= 180.0f) {
      bpmValidoPrevio = bpmCalculado;
    }

    float bpmMostrar = (bpmValidoPrevio > 0.0f) ? bpmValidoPrevio : bpmCalculado;
    uint8_t spo2Valor = procesadorSpO2.getSpO2();

    // Limpiar línea anterior
    Serial.print('\r');
    for (uint8_t i = 0; i < ANCHO_LIMPIEZA_LINEA; ++i) Serial.print(' ');
    Serial.print('\r');

    // Mostrar resultados
    Serial.print(F("BPM: "));
    if (bpmMostrar > 0.0f) Serial.print(bpmMostrar, 1);
    else                   Serial.print(F("--"));

    Serial.print(F("   SpO2: "));
    if (spo2Valor > 0) {
      Serial.print(spo2Valor);
      Serial.print('%');
    } else {
      Serial.print(F("--"));
    }

    tiempoUltimaSalida = tiempoActual;
  }
}
