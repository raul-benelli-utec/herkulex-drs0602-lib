// herkulex_utils.h
#ifndef HERKULEX_UTILS_H
#define HERKULEX_UTILS_H

#include <Arduino.h>
#include "herkulex_defs.h"

// Funciones fundamentales
void herkulex_ledSet(byte id, byte color);
void herkulex_ledOff(byte id);
void herkulex_reboot(byte id);
uint16_t readPosition(byte id);

void herkulex_moveTo(byte id, uint16_t position, byte playtime);
void commandEnableTorque(byte id);

// Funciones auxiliares para construcción de paquetes
void buildPacket(byte* packet, byte id, byte cmd, const byte* data, byte dataLen);
void sendPacket(const byte* packet);

// Configuración EEPROM
void commandSetID(byte old_id, byte new_id);
void commandSetMaxPosition(byte id, uint16_t pos);
void commandSetMinPosition(byte id, uint16_t pos);
void commandSetPositionLimits(byte id, uint16_t min, uint16_t max);
void commandReadStatus(uint8_t id);
void commandSetCalibrationDiff(byte id, int16_t offset);
void commandSetAcceleration(byte id, byte accelRatio, byte accelTime);
void herkulex_safeMoveTo(byte id, uint16_t targetPosition);
void herkulex_sJogMultiMove(byte ids[], uint16_t posiciones[], byte cantidad, byte playtime);
void moverAPoseInicial(); 
void revisarErroresMotores(byte ids[], byte cantidad);
void commandClearError(byte id);
void commandTorqueFree(byte id);
void commandBrakeOn(byte id);
void commandSetServoPolicy(byte id, byte policy);

uint16_t readPosition_debug(uint8_t id);

// Lectura de error
bool herkulex_readStatus(byte id, byte& statusError, byte& detailError);
void printHerkulexErrors(byte statusError, byte detailError);
#endif
