# Proyecto Salud - Monitor Cardíaco y Calidad de Aire
En este repositorio descansa la documentación requerida para realizar el proyecto integrador del equipo de Ingenieria Electronica de la Unidad Central del Valle del Cauca, este proyecto integra dos sensores en Arduino para monitorear salud y ambiente:

- *MAX30102* para medición de ritmo cardíaco (BPM) y saturación de oxígeno en sangre (SpO2).
- *Plantower PMS7003* para medición de partículas en el aire (PM1.0, PM2.5, PM10).

# Hardware requerido

- Arduino o ESP32 con al menos un puerto Serial1 hardware (ej. Arduino Mega, Leonardo, o similar)
- Sensor MAX30102 conectado por I2C (SDA, SCL)
- Sensor PMS7003 conectado a puerto Serial1 (RX, TX) con nivel lógico 3.3V o 5V con conversor si es necesario

# Conexiones

# MAX30102

- VCC a 3.3V o 5V (según modelo)
- GND a GND
- SDA a Arduino SDA (ejemplo: A4 en Uno)
- SCL a Arduino SCL (ejemplo: A5 en Uno)

# PMS7003

- VCC a 5V
- GND a GND
- RX a TX de Arduino (Serial1 TX)
- TX a RX de Arduino (Serial1 RX)
