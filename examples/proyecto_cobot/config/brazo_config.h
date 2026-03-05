#ifndef BRAZO_CONFIG_H
#define BRAZO_CONFIG_H

#include <Arduino.h>

// Número de motores del brazo
constexpr uint8_t NUM_MOTORES = 5;

// Declaraciones externas (definiciones en src/brazo_config.cpp)
extern const uint8_t MOTOR_IDS[NUM_MOTORES];
extern uint16_t POS_MIN[NUM_MOTORES];
extern uint16_t POS_MAX[NUM_MOTORES];

#endif // BRAZO_CONFIG_H

