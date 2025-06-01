#include <Arduino.h>
#include <PMS.h>

// Inicializar comunicación serial con el sensor PMS7003
PMS pms(Serial1);
PMS::DATA datosPM;

void inicializarSeriales() {
  Serial.begin(115200);
  Serial1.begin(9600);  // Puerto del PMS7003
  delay(100);
}

void inicializarSensorAire() {
  pms.passiveMode();  // el sensor solo envía datos cuando se solicita
  Serial.println("Sensor de calidad de aire PMS7003 listo.");
}

void solicitarYMostrarDatosPM() {
  pms.wakeUp();              // Despierta el sensor
  delay(1000);               // Esperar estabilización
  pms.requestRead();         // Solicita datos

  if (pms.readUntil(datosPM)) {
    Serial.print("PM1.0: ");
    Serial.print(datosPM.PM_AE_UG_1_0);
    Serial.print(" µg/m³   PM2.5: ");
    Se
