#include <Arduino.h>
#include "LIB_MAX30102.h"
#include "COMP_RITMO_CARDIACO.h"
#include "COMP_SPO2.h"
#include "Plantower_PMS7003.h"

// Configuración MAX30102 (ritmo cardiaco y SpO2)
constexpr uint32_t FINGER_TH_ON  = 30000;
constexpr uint32_t FINGER_TH_OFF = 20000;
constexpr uint32_t SERIAL_UPDATE_INTERVAL = 1000;
constexpr uint8_t  LINE_CLEAR_WIDTH = 40;
constexpr float DC_ALPHA = 0.95f;

MAX30102 sensorMAX30102;
HeartRateProcessor hrProcessor;
SpO2Processor spo2Processor;

uint32_t lastSerialPrint = 0;
bool fingerPresent = false;
float dcIR = 0.0f;
float dcRed = 0.0f;
static float lastValidBPM = 0.0f;

// Sensor PMS7003 (calidad del aire)
Plantower_PMS7003 sensorPMS7003;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Inicialización MAX30102
  if (!sensorMAX30102.begin()) {
    Serial.println(F("ERROR: MAX30102 not found."));
    while (true) delay(100);
  }
  sensorMAX30102.setup();
  hrProcessor.reset();
  spo2Processor.reset();
  lastSerialPrint = millis();

  // Inicialización PMS7003
  Serial1.begin(9600);
  sensorPMS7003.init(&Serial1);
  sensorPMS7003.debug = false; // Activar true si quieres debug
}

void loop() {
  uint32_t now = millis();

  // Lectura MAX30102
  std::vector<std::pair<uint32_t,uint32_t>> samples;
  if (sensorMAX30102.readAllFIFO(samples)) {
    for (auto &p : samples) {
      uint32_t rawRed = p.first;
      uint32_t rawIR  = p.second;

      if (!fingerPresent && rawIR > FINGER_TH_ON) {
        Serial.println(F("\n-- Finger placed, starting measurements --"));
        fingerPresent = true;
        lastSerialPrint = now;
      }
      else if (fingerPresent && rawIR < FINGER_TH_OFF) {
        Serial.println(F("\n-- Finger removed, pausing --"));
        fingerPresent = false;
        hrProcessor.reset();
        spo2Processor.reset();
        continue;
      }
      if (!fingerPresent) continue;

      dcIR  = DC_ALPHA * dcIR  + (1.0f - DC_ALPHA) * rawIR;
      dcRed = DC_ALPHA * dcRed + (1.0f - DC_ALPHA) * rawRed;
      float acIR  = float(rawIR)  - dcIR;
      float acRed = float(rawRed) - dcRed;

      bool beat = hrProcessor.update(acIR, now);
      spo2Processor.update(acIR, acRed, beat);
    }

    // Mostrar resultados MAX30102 periódicamente
    if (fingerPresent && (now - lastSerialPrint >= SERIAL_UPDATE_INTERVAL)) {
      float rawBPM = hrProcessor.getBPM();
      if (rawBPM >= 40.0f && rawBPM <= 180.0f) lastValidBPM = rawBPM;
      float displayBPM = (lastValidBPM > 0.0f ? lastValidBPM : rawBPM);
      uint8_t spo2v = spo2Processor.getSpO2();

      Serial.print('\r');
      for (uint8_t i = 0; i < LINE_CLEAR_WIDTH; ++i) Serial.print(' ');
      Serial.print('\r');

      Serial.print(F("BPM: "));
      if (displayBPM > 0.0f) Serial.print(displayBPM, 1);
      else Serial.print(F("--"));

      Serial.print(F("   SpO2: "));
      if (spo2v > 0) {
        Serial.print(spo2v);
        Serial.print('%');
      } else {
        Serial.print(F("--"));
      }
      Serial.println();
      lastSerialPrint = now;
    }
  }

  // Lectura PMS7003 (calidad del aire)
  sensorPMS7003.updateFrame();
  if (sensorPMS7003.hasNewData()) {
    Serial.print(F("PM1.0: "));
    Serial.print(sensorPMS7003.getPM_1_0());
    Serial.print(F("  PM2.5: "));
    Serial.print(sensorPMS7003.getPM_2_5());
    Serial.print(F("  PM10: "));
    Serial.print(sensorPMS7003.getPM_10_0());
    Serial.println();
  }

  delay(50);  // Pequeño delay para no saturar
}
