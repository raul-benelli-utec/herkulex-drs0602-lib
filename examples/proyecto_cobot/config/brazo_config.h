#ifndef BRAZO_CONFIG_H
#define BRAZO_CONFIG_H

#include <Arduino.h>

// Número de motores del brazo
constexpr uint8_t NUM_MOTORES = 5;

// Declaraciones externas (definiciones en src/brazo_config.cpp)
extern const uint8_t MOTOR_IDS[NUM_MOTORES];
extern uint16_t POS_MIN[NUM_MOTORES];
extern uint16_t POS_MAX[NUM_MOTORES];

// Índice del motor base en MOTOR_IDS (primer articulación — giro lateral)
constexpr uint8_t MOTOR_INDEX_BASE = 0;

// Política de seguridad: límites de PWM (0-1023, proporcional a corriente/torque)
// Por articulación: base, hombro, codo, muñeca, pinza
//
// REGLA OBLIGATORIA:  MONITOR  <  OVERLOAD  <=  MAX
//   MAX      = tope de esfuerzo (torque bajo → valores bajos, ej. 400)
//   OVERLOAD = alarma del firmware del servo (NUNCA mayor que MAX)
//   MONITOR  = alarma software en movimiento (menor que OVERLOAD)
//
constexpr uint16_t SAFETY_PWM_MAX[NUM_MOTORES] = {400, 400, 400, 400, 400};
constexpr uint16_t SAFETY_PWM_MONITOR[NUM_MOTORES] = {140, 110, 180, 160, 160};
constexpr uint16_t SAFETY_PWM_OVERLOAD[NUM_MOTORES] = {380, 380, 380, 380, 380};

// Modo demo: detecta incremento de PWM en motor base respecto al inicio del movimiento
constexpr uint8_t SAFETY_DEMO_MODE = 1;
constexpr uint16_t SAFETY_PWM_DELTA_BASE = 55;
constexpr unsigned long SAFETY_BASELINE_DELAY_MS = 280;

constexpr unsigned long SAFETY_MONITOR_INTERVAL_MS = 50;
constexpr unsigned long SAFETY_MONITOR_TAIL_MS = 700;
constexpr uint8_t SAFETY_PWM_CONSECUTIVE_HITS = 2;
constexpr uint8_t SAFETY_BASE_CONSECUTIVE_HITS = 1;

// 1 = también flags de error del servo (recomendado si OVERLOAD <= MAX)
constexpr uint8_t SAFETY_USE_STATUS_CHECK = 1;
constexpr uint16_t SAFETY_RETREAT_MS = 1500;

// 1 = imprime PWM de cada motor durante el movimiento (para calibrar umbrales)
constexpr uint8_t SAFETY_DEBUG_PWM = 1;

#endif // BRAZO_CONFIG_H

