#include <Arduino.h>
#include "LIB_SHT31.h"

// Objeto del sensor
SHT31 sht31;

// Inicialización del sistema
void inicializarComunicacionSerial() {
  Serial.begin(115200);
  delay(100);
}

void inicializarSensor() {
  if (!sht31.begin()) {
    Serial.print("Fallo al conectar con el sensor: ");
    Serial.println(sht31.getErrorMessage());
    while (true) delay(1000);
  }
  Serial.println("Sensor SHT31 conectado exitosamente.");
}

void setup() {
  inicializarComunicacionSerial();
  inicializarSensor();
}

// Lectura e impresión de datos
void mostrarLecturaClimatica() {
  float tempC, humedadRel;

  if (sht31.read(tempC, humedadRel)) {
    Serial.print("Temp: ");
    Serial.print(tempC, 2);
    Serial.print(" °C   Humedad: ");
    Serial.print(humedadRel, 2);
    Serial.println(" %");
  } else {
    Serial.print("Fallo en lectura: ");
    Serial.println(sht31.getErrorMessage());
  }
}

void loop() {
  mostrarLecturaClimatica();
  delay(1000);
}
