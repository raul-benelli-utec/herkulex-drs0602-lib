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

  // Mover brazo a una pose específica
  // playtimeMs: tiempo de movimiento en milisegundos (se convierte internamente a unidades de 11.2ms)
  // Retorna: true si la operación fue exitosa
  bool goPose(const Pose& pose, uint16_t playtimeMs);
  
  // Mover brazo a posiciones directamente (sin estructura Pose)
  // posiciones: array de NUM_MOTORES posiciones
  // playtimeMs: tiempo de movimiento en milisegundos
  // Retorna: true si la operación fue exitosa
  bool goRaw(const uint16_t posiciones[NUM_MOTORES], uint16_t playtimeMs);

  // Limpiar errores de todos los motores
  void clearAllErrors();

  // Check de todos los motores: activa torque uno por uno
  // Retorna: true si todos los motores responden correctamente
  bool checkAll();

  // Leer y mostrar posición de todos los motores
  void printAllPositions();
  
  // Control de torque de todos los motores
  void torqueAllOn();
  void torqueAllOff();
};

#endif // BRAZO_H

