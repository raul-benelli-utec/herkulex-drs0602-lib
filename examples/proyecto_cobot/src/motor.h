#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "herkulex_utils.h"

// Estructura para representar un motor HerkuleX
struct Motor {
  uint8_t id;
  uint16_t posMin;
  uint16_t posMax;
  uint16_t home;
  uint8_t accelRatio;
  uint8_t accelTime;
  uint16_t pwmThreshold;

  // Clamp de posición (si min/max = 0, no clampa)
  uint16_t clamp(uint16_t p) const;

  // Identificar motor (blink LED rojo 3 veces)
  void identify();

  // Control de torque
  void torqueOn();
  void torqueOff();

  // Movimiento seguro a posición objetivo
  // velocity: velocidad en cuentas por segundo (0 = usar valor por defecto)
  void safeMove(uint16_t target, float velocity = 0);

  // Leer posición actual del servo
  // Retorna: posición absoluta en cuentas (0-1023)
  uint16_t getPosition() const;

  // Check del motor: LED rojo -> torque ON -> LED verde
  // Retorna: true si el motor responde correctamente
  bool check();
};

#endif // MOTOR_H

