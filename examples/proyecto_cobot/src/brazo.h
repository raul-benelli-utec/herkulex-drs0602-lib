#ifndef BRAZO_H
#define BRAZO_H

#include <Arduino.h>
#include "brazo_config.h"
#include "poses.h"
#include "motor.h"
#include "herkulex_utils.h"

// Estructura para representar el brazo robótico completo
struct Brazo {
  Motor motors[NUM_MOTORES];

  // Inicializar comunicación Serial1
  void begin();

  // Llenar array con IDs de los motores
  void fillIds(uint8_t outIds[NUM_MOTORES]) const;

  // Aplicar límites de corriente/torque en cada servo (RAM, al arranque)
  void applySafetyLimits();

  // Mover brazo a una pose específica
  // playtimeMs: tiempo de movimiento en milisegundos (se convierte internamente a unidades de 11.2ms)
  // safetyMonitor: si true, vigila PWM durante el movimiento y retira a standby ante sobrecarga
  // Retorna: false si se detectó sobrecarga y se abortó el movimiento
  bool goPose(const Pose& pose, uint16_t playtimeMs, bool safetyMonitor = true);
  
  // Mover brazo a posiciones directamente (sin estructura Pose)
  // posiciones: array de NUM_MOTORES posiciones
  // playtimeMs: tiempo de movimiento en milisegundos
  // Retorna: false si se detectó sobrecarga y se abortó el movimiento
  bool goRaw(const uint16_t posiciones[NUM_MOTORES], uint16_t playtimeMs, bool safetyMonitor = true);

  // Limpiar errores de todos los motores
  void clearAllErrors();

  // Check de todos los motores: activa torque uno por uno
  // Retorna: true si todos los motores responden correctamente
  bool checkAll();

  // Leer y mostrar posición de todos los motores
  void printAllPositions();

  // PWM/torque/estado actual (comando 't' en Serial, para calibrar umbrales)
  void printTorqueSnapshot();
  
  // Control de torque de todos los motores
  void torqueAllOn();
  void torqueAllOff();

private:
  bool checkMotorPwmHigh(uint8_t motorIndex, uint16_t& pwmOut, bool& readOk);
  bool checkMotorOverload(uint8_t motorIndex, uint16_t pwm, bool readOk,
                          const uint16_t* baseline, bool baselineReady);
  bool checkMotorStatusError(uint8_t motorIndex);
  bool waitAndMonitorMove(uint16_t playtimeMs);
  void retreatToStandbyOnOverload(uint8_t motorIndex, uint16_t pwm);
};

#endif // BRAZO_H

