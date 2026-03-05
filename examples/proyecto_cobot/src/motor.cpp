#include "motor.h"

// Clamp de posición (si min/max = 0, no clampa)
uint16_t Motor::clamp(uint16_t p) const {
  if (posMin > 0 && p < posMin) {
    return posMin;
  }
  if (posMax > 0 && p > posMax) {
    return posMax;
  }
  return p;
}

// Identificar motor (blink LED rojo 3 veces)
void Motor::identify() {
  for (int i = 0; i < 3; i++) {
    herkulex_ledSet(id, LED_RED);
    delay(300);
    herkulex_ledOff(id);
    delay(300);
  }
  // Asegurar torque OFF después de identificar
  torqueOff();
}

// Control de torque
void Motor::torqueOn() {
  commandEnableTorque(id);
}

void Motor::torqueOff() {
  commandTorqueFree(id);
}

// Movimiento seguro a posición objetivo
void Motor::safeMove(uint16_t target, float velocity) {
  // Aplicar clamp si hay límites definidos
  uint16_t clampedTarget = clamp(target);
  
  // Limpiar buffer antes de leer posición (importante para evitar errores)
  while (Serial1.available()) (void)Serial1.read();
  delay(10);
  
  // Leer posición actual para calcular playtime
  uint16_t currentPos = readPosition(id);
  if (currentPos == 0xFFFF) {
    // Si no se puede leer, usar un playtime por defecto
    // Basado en la distancia estimada
    uint16_t distance = abs((int16_t)(clampedTarget - 16384)); // Posición central aproximada
    float timeSeconds = distance / 250.0f;  // Velocidad por defecto: 250 cuentas/s
    uint8_t playtime = (uint8_t)(timeSeconds / 0.0112f);
    if (playtime < 1) playtime = 1;
    if (playtime > 254) playtime = 254;
    
    // Limpiar buffer antes de mover
    while (Serial1.available()) (void)Serial1.read();
    delay(10);
    
    herkulex_moveTo(id, clampedTarget, playtime, LED_OFF);
    return;
  }
  
  // Usar velocidad por defecto si no se especifica
  if (velocity <= 0) {
    velocity = 250.0f;  // Velocidad por defecto en cuentas por segundo
  }
  
  // Calcular playtime usando la función de la librería
  uint8_t playtime = herkulex_computePlaytime(currentPos, clampedTarget, velocity);
  
  // Limpiar buffer antes de mover
  while (Serial1.available()) (void)Serial1.read();
  delay(10);
  
  // Mover usando herkulex_moveTo directamente (más confiable que safeMoveTo)
  herkulex_moveTo(id, clampedTarget, playtime, LED_OFF);
}

// Leer posición actual del servo
uint16_t Motor::getPosition() const {
  // Usar readPosition_debug que limpia el buffer internamente y es más robusto
  return readPosition_debug(id);
}

// Check del motor: LED rojo -> torque ON -> LED verde
bool Motor::check() {
  // LED rojo
  herkulex_ledSet(id, LED_RED);
  delay(100);
  
  // Activar torque
  torqueOn();
  
  // LED verde
  herkulex_ledSet(id, LED_GREEN);
  delay(500);
  
  // Verificar que el motor responde (opcional, pero útil)
  byte statusError, detailError;
  bool responds = herkulex_readStatus(id, statusError, detailError);
  
  return responds;
}

