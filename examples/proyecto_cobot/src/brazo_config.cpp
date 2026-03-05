#include "brazo_config.h"

// IDs de los motores (orden: base, hombro, codo, muñeca, pinza)
const uint8_t MOTOR_IDS[NUM_MOTORES] = {1, 2, 3, 4, 5};

// Límites de posición (pendientes de calibración en laboratorio)
// Si min/max = 0, no se aplica clamp
// TODO: Calibrar estos valores en laboratorio
uint16_t POS_MIN[NUM_MOTORES] = {0, 0, 0, 0, 0};
uint16_t POS_MAX[NUM_MOTORES] = {0, 0, 0, 0, 0};

