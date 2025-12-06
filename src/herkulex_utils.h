// herkulex_utils.h
#ifndef HERKULEX_UTILS_H
#define HERKULEX_UTILS_H

#include <Arduino.h>
#include "herkulex_defs.h"

// ============================================================================
// FUNCIONES AUXILIARES - CONSTRUCCIÓN Y ENVÍO DE PAQUETES
// ============================================================================
void buildPacket(byte* packet, byte id, byte cmd, const byte* data, byte dataLen);
void sendPacket(const byte* packet);
bool validateChecksum(const byte* packet, byte packetSize);
int readResponse(byte* buffer, int maxSize, unsigned long timeout_ms);

// ============================================================================
// FUNCIONES BÁSICAS
// ============================================================================
void herkulex_ledSet(byte id, byte color);
void herkulex_ledOff(byte id);
void herkulex_reboot(byte id);

// ============================================================================
// CONTROL DE TORQUE
// ============================================================================
void commandEnableTorque(byte id);
void commandTorqueFree(byte id);
void commandBrakeOn(byte id);

// ============================================================================
// COMANDOS DE MOVIMIENTO
// ============================================================================
// Calcula playtime basado en distancia y velocidad
uint8_t herkulex_computePlaytime(uint16_t current, uint16_t goal, float v_counts_per_s);

// Movimiento a posición usando S_JOG (sincronizado)
// playtime: tiempo en unidades de 11.2ms (0 = calcular automáticamente)
// ledFlags: flags de LED (LED_OFF, LED_GREEN, etc.) o 0 para sin LED
void herkulex_moveTo(byte id, uint16_t position, byte playtime, byte ledFlags = 0);

// Movimiento a posición usando I_JOG (individual, tiempo por servo)
void herkulex_iJog(byte id, uint16_t position, byte playtime, byte ledFlags = 0);

// Giro continuo (modo velocidad)
void herkulex_continuousRotation(byte id, int16_t speed, byte playtime, byte ledFlags = 0);

// Movimiento seguro con cálculo automático de playtime
// velocity: velocidad objetivo en cuentas por segundo (0 = usar valor por defecto)
void herkulex_safeMoveTo(byte id, uint16_t targetPosition, float velocity = 0);

// Movimiento sincronizado múltiple usando S_JOG
void herkulex_sJogMultiMove(byte ids[], uint16_t posiciones[], byte cantidad, byte playtime, byte ledFlags = 0);

// ============================================================================
// LECTURA DE REGISTROS
// ============================================================================
// Funciones auxiliares de lectura
bool readRAMByte(byte id, byte address, byte& value);
bool readRAMWord(byte id, byte address, uint16_t& value);

// Lectura de posición
uint16_t readPosition(byte id);
bool herkulex_readAbsolutePosition(byte id, uint16_t& position);
bool herkulex_readTargetPosition(byte id, uint16_t& position);

// Lectura de sensores
bool herkulex_readVoltage(byte id, byte& voltage);
bool herkulex_readTemperature(byte id, byte& temperature);
bool herkulex_readPWM(byte id, uint16_t& pwm);
bool herkulex_readCurrentMode(byte id, byte& mode);
bool herkulex_readSpeed(byte id, uint16_t& speed);

// ============================================================================
// ESCRITURA DE REGISTROS
// ============================================================================
// Configuración EEPROM (requiere REBOOT para aplicar)
void commandSetID(byte old_id, byte new_id);
void commandSetMaxPosition(byte id, uint16_t pos);
void commandSetMinPosition(byte id, uint16_t pos);
void commandSetPositionLimits(byte id, uint16_t min, uint16_t max);
void commandSetCalibrationDiff(byte id, int16_t offset);

// Configuración RAM (aplica inmediatamente)
void commandSetAcceleration(byte id, byte accelRatio, byte accelTime);
void commandSetServoPolicy(byte id, byte policy);
void herkulex_setMaxPWM(byte id, uint16_t maxPWM, bool writeToEEP = false);
void herkulex_setMinPWM(byte id, byte minPWM, bool writeToEEP = false);
void herkulex_setOverloadThreshold(byte id, uint16_t threshold, bool writeToEEP = false);

// ============================================================================
// ESTADO Y ERRORES
// ============================================================================
bool herkulex_readStatus(byte id, byte& statusError, byte& detailError);
void commandReadStatus(uint8_t id);  // Versión que imprime directamente
void commandClearError(byte id);
void printHerkulexErrors(byte statusError, byte detailError);
void revisarErroresMotores(byte ids[], byte cantidad);

// ============================================================================
// CONTROL DE TORQUE ADAPTATIVO
// ============================================================================
struct TorqueMonitor {
  uint16_t pwmCurrent;        // PWM actual leído
  uint16_t pwmMax;            // PWM máximo configurado
  uint16_t pwmThreshold;      // Umbral de sobrecarga
  bool overloadDetected;      // Flag de sobrecarga detectada
  unsigned long lastCheck;    // Última vez que se verificó
  float currentVelocity;      // Velocidad actual ajustada
};

bool herkulex_monitorOverload(byte id, TorqueMonitor& monitor);
float herkulex_adaptiveVelocityControl(byte id, TorqueMonitor& monitor, float baseVelocity);

// ============================================================================
// ESTIMACIÓN DE TORQUE Y CORRIENTE BASADA EN PWM
// ============================================================================
// Nota: El PWM es proporcional a la corriente, y la corriente es proporcional al torque.
// Estas funciones estiman torque/corriente basándose en esta relación lineal.

// Estima el torque actual en Nm basado en el PWM
float herkulex_estimateTorqueFromPWM(uint16_t pwm, uint16_t pwmMax = 0);

// Estima la corriente actual en mA basada en el PWM
float herkulex_estimateCurrentFromPWM(uint16_t pwm, uint16_t pwmMax = 0);

// Obtiene el porcentaje de torque actual (0-100%)
float herkulex_getTorquePercentage(uint16_t pwm, uint16_t pwmMax = 0);

// Lee PWM y calcula torque estimado
bool herkulex_readTorque(byte id, float& torque, uint16_t pwmMax = 0);

// Lee PWM y calcula corriente estimada
bool herkulex_readCurrent(byte id, float& current, uint16_t pwmMax = 0);

// Obtiene información completa de torque/corriente
bool herkulex_getTorqueInfo(byte id, uint16_t& pwm, float& torque, float& current, 
                            float& percentage, uint16_t pwmMax = 0);

// ============================================================================
// FUNCIONES DE DEBUG (pueden eliminarse en producción)
// ============================================================================
uint16_t readPosition_debug(uint8_t id);
void moverAPoseInicial();  // Función vacía/comentada

#endif
