#include "herkulex_utils.h"
#include "herkulex_defs.h"
#include <Arduino.h>

void buildPacket(byte* packet, byte id, byte cmd, const byte* data, byte dataLen) {
  packet[0] = 0xFF;
  packet[1] = 0xFF;
  packet[2] = dataLen + 7;
  packet[3] = id;
  packet[4] = cmd;
  packet[5] = 0x00;  // checksum1
  packet[6] = 0x00;  // checksum2

  for (int i = 0; i < dataLen; i++) {
    packet[7 + i] = data[i];
  }

  byte chksum1 = 0;
  for (int i = 2; i < dataLen + 7; i++) {
    chksum1 ^= packet[i];
  }

  packet[5] = chksum1 & 0xFE;
  packet[6] = (~packet[5]) & 0xFE;
}

void sendPacket(const byte* packet) {
  int len = packet[2];

  // Enviar al motor
  for (int i = 0; i < len; i++) {
    Serial1.write(packet[i]);
  }

  // También mostrar por consola para debug
  Serial.print("Paquete enviado: ");
  for (int i = 0; i < len; i++) {
    Serial.print("0x");
    if (packet[i] < 0x10) Serial.print("0");
    Serial.print(packet[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void herkulex_ledSet(byte id, byte color) {
  byte data[3];
  data[0] = ADDR_LED_CTRL;
  data[1] = 0x01;
  data[2] = color;

  byte packet[10];
  buildPacket(packet, id, CMD_RAM_WRITE, data, 3);
  sendPacket(packet);
}

void commandClearError(byte id) {
  byte data[4] = {
    0x30, // Dirección de inicio: STATUS_ERROR
    0x02, // Longitud: STATUS_ERROR + DETAIL_ERROR
    0x00, // Valor para STATUS_ERROR
    0x00  // Valor para DETAIL_ERROR
  };

  byte packet[11];  // 7 cabecera + 4 datos
  buildPacket(packet, id, CMD_RAM_WRITE, data, 4);
  sendPacket(packet);
}


void herkulex_ledOff(byte id) {
  herkulex_ledSet(id, 0x00);
}

void herkulex_reboot(byte id) {
  byte packet[7];
  buildPacket(packet, id, CMD_REBOOT, nullptr, 0);
  sendPacket(packet);
}

void commandSetID(byte old_id, byte new_id) {
  byte data[3] = {ADDR_SERVO_ID, 0x01, new_id};  // Dir, Len, Valor
  byte packet[10];
  buildPacket(packet, old_id, CMD_EEP_WRITE, data, 3);
  sendPacket(packet);
}

void commandSetMaxPosition(byte id, uint16_t pos) {
  byte data[4] = {
    ADDR_POS_MAX_L, 0x02,             // Dirección LSB de MAX
    byte(pos & 0xFF), byte(pos >> 8)
  };
  byte packet[11];
  buildPacket(packet, id, CMD_EEP_WRITE, data, 4);
  sendPacket(packet);
}

void commandSetMinPosition(byte id, uint16_t pos) {
  byte data[4] = {
    ADDR_POS_MIN_L, 0x02,             // Dirección LSB de MIN
    byte(pos & 0xFF), byte(pos >> 8)
  };
  byte packet[11];
  buildPacket(packet, id, CMD_EEP_WRITE, data, 4);
  sendPacket(packet);
}

void commandSetPositionLimits(byte id, uint16_t min, uint16_t max) {
  commandSetMinPosition(id, min);
  delay(10);
  commandSetMaxPosition(id, max);
}

// Función auxiliar para validar checksum de respuesta
bool validateChecksum(const byte* packet, byte packetSize) {
  if (packetSize < 7) return false;
  
  byte chksum1 = 0;
  for (int i = 2; i < packetSize; i++) {
    chksum1 ^= packet[i];
  }
  chksum1 &= 0xFE;
  
  byte chksum2 = (~chksum1) & 0xFE;
  
  return (packet[5] == chksum1 && packet[6] == chksum2);
}

// Lee respuesta del servo con sincronización y validación
// Retorna número de bytes leídos, o -1 en caso de error
int readResponse(byte* buffer, int maxSize, unsigned long timeout_ms) {
  unsigned long startTime = millis();
  int pos = 0;
  
  // Buscar header 0xFF 0xFF
  while ((millis() - startTime) < timeout_ms) {
    if (Serial1.available() > 0) {
      byte b = Serial1.read();
      
      if (pos == 0 && b == 0xFF) {
        buffer[pos++] = b;
      } else if (pos == 1 && b == 0xFF) {
        buffer[pos++] = b;
      } else if (pos >= 2) {
        buffer[pos++] = b;
        
        // Si ya tenemos el tamaño del paquete
        if (pos >= 3) {
          byte packetSize = buffer[2];
          if (packetSize > maxSize) {
            return -1;  // Paquete demasiado grande
          }
          
          // Esperar el resto del paquete
          while (pos < packetSize && (millis() - startTime) < timeout_ms) {
            if (Serial1.available() > 0) {
              buffer[pos++] = Serial1.read();
            }
          }
          
          if (pos == packetSize) {
            // Validar checksum
            if (validateChecksum(buffer, packetSize)) {
              return pos;
            } else {
              return -1;  // Checksum inválido
            }
          }
        }
      } else {
        pos = 0;  // Reset si no encontramos header
      }
    }
  }
  
  return -1;  // Timeout
}

// Lee un registro de RAM de 1 byte
bool readRAMByte(byte id, byte address, byte& value) {
  byte data[2] = {address, 0x01};  // Dirección y longitud
  byte packet[9];
  
  buildPacket(packet, id, CMD_RAM_READ, data, 2);
  Serial1.write(packet, packet[2]);
  delay(HERKULEX_RESPONSE_DELAY_MS);
  
  byte response[16];
  int len = readResponse(response, sizeof(response), HERKULEX_ACK_TIMEOUT_MS);
  
  if (len >= 11 && response[4] == ACK_RAM_READ) {
    // Estructura: [FF][FF][LEN][ID][CMD][CHK1][CHK2][ADDR][NBYTES][DATA][STATUS_ERROR][STATUS_DETAIL]
    value = response[9];
    return true;
  }
  
  return false;
}

// Lee un registro de RAM de 2 bytes (Little Endian)
bool readRAMWord(byte id, byte address, uint16_t& value) {
  byte data[2] = {address, 0x02};  // Dirección y longitud
  byte packet[9];
  
  buildPacket(packet, id, CMD_RAM_READ, data, 2);
  Serial1.write(packet, packet[2]);
  delay(HERKULEX_RESPONSE_DELAY_MS);
  
  byte response[16];
  int len = readResponse(response, sizeof(response), HERKULEX_ACK_TIMEOUT_MS);
  
  if (len >= 12 && response[4] == ACK_RAM_READ) {
    // Estructura: [FF][FF][LEN][ID][CMD][CHK1][CHK2][ADDR][NBYTES][DATA_LSB][DATA_MSB][STATUS_ERROR][STATUS_DETAIL]
    value = response[9] | (response[10] << 8);
    return true;
  }
  
  return false;
}

// Lee posición calibrada actual
uint16_t readPosition(byte id) {
  uint16_t pos = 0xFFFF;
  if (readRAMWord(id, ADDR_POS_CALIB_L, pos)) {
    return pos;
  }
  return 0xFFFF;  // Error
}

bool herkulex_readStatus(byte id, byte& statusError, byte& detailError) {
  byte packet[7];
  buildPacket(packet, id, CMD_STAT, nullptr, 0);
  Serial1.write(packet, packet[2]);

  delay(5);  // Esperar respuesta

  int timeout = 1000;
  while (Serial1.available() < 9 && timeout-- > 0) delay(1);

  if (Serial1.available() >= 9) {
    byte respuesta[9];
    for (int i = 0; i < 9; i++) respuesta[i] = Serial1.read();

    if (respuesta[0] != 0xFF || respuesta[1] != 0xFF || respuesta[4] != CMD_STAT + 0x40) {
      return false;  // Respuesta no válida
    }

    statusError = respuesta[7];
    detailError = respuesta[8];
    return true;
  }

  return false;  // Timeout
}

void printHerkulexErrors(byte statusError, byte detailError) {
  Serial.println("== Estado del servo ==");

  // STATUS ERROR BYTE
  if (statusError == 0) {
    Serial.println("Estado general: OK");
  } else {
    Serial.print("Status Error (0x");
    Serial.print(statusError, HEX);
    Serial.println("):");
    if (statusError & 0x01) Serial.println(" - Exceso de temperatura");
    if (statusError & 0x02) Serial.println(" - Voltaje fuera de rango");
    if (statusError & 0x04) Serial.println(" - Posición fuera de rango");
    if (statusError & 0x08) Serial.println(" - Error de checksum");
    if (statusError & 0x10) Serial.println(" - Instrucción desconocida");
    if (statusError & 0x20) Serial.println(" - Error de tiempo de operación");
    if (statusError & 0x40) Serial.println(" - Error de sobrecarga");
  }

  // DETAIL ERROR BYTE
  if (detailError == 0) {
    Serial.println("Detalle de error: Ninguno");
  } else {
    Serial.print("Detail Error (0x");
    Serial.print(detailError, HEX);
    Serial.println("):");
    if (detailError & 0x01) Serial.println(" - EEPROM dañada");
    if (detailError & 0x02) Serial.println(" - EEPROM bloqueada");
    if (detailError & 0x04) Serial.println(" - Error en lectura de EEPROM");
    if (detailError & 0x08) Serial.println(" - Detección de bloqueo (stall)");
    if (detailError & 0x10) Serial.println(" - Instrucción no ejecutada");
    if (detailError & 0x20) Serial.println(" - Valor inválido");
    if (detailError & 0x40) Serial.println(" - Sobre corriente detectada");
    if (detailError & 0x80) Serial.println(" - Otra condición de error");
  }

  Serial.println("========================");
}

// Calcula playtime basado en distancia y velocidad objetivo
uint8_t herkulex_computePlaytime(uint16_t current, uint16_t goal, float v_counts_per_s) {
  int32_t diff = (int32_t)goal - (int32_t)current;
  uint32_t dist = (diff >= 0) ? diff : -diff;

  float t = dist / v_counts_per_s;

  const float T_MIN = HERKULEX_MIN_PLAYTIME_MS / 1000.0f;
  const float T_MAX = HERKULEX_MAX_PLAYTIME_MS / 1000.0f;

  if (t < T_MIN) t = T_MIN;
  if (t > T_MAX) t = T_MAX;

  float pt = t / (HERKULEX_PLAYTIME_UNIT_MS / 1000.0f);

  if (pt < 1.0f)   pt = 1.0f;
  if (pt > 254.0f) pt = 254.0f;

  return (uint8_t)(pt + 0.5f);
}

// Movimiento a posición usando S_JOG (sincronizado)
// playtime: tiempo en unidades de 11.2ms (0 = calcular automáticamente)
// ledFlags: flags de LED (LED_OFF, LED_GREEN, etc.) o 0 para sin LED
void herkulex_moveTo(byte id, uint16_t position, byte playtime, byte ledFlags) {
  byte data[5];
  byte setByte = 0x00;  // Modo posición por defecto

  // Configurar flags de LED si se especifican
  if (ledFlags & LED_GREEN) setByte |= SET_LED_GREEN;
  if (ledFlags & LED_BLUE) setByte |= SET_LED_BLUE;
  if (ledFlags & LED_RED) setByte |= SET_LED_RED;

  // Formato S_JOG: [playtime][JOG_LSB][JOG_MSB][SET][ID]
  data[0] = playtime;                // Tiempo de movimiento (11.2 ms * N)
  data[1] = position & 0xFF;         // Posición LSB
  data[2] = (position >> 8) & 0xFF;  // Posición MSB
  data[3] = setByte;                 // SET: Modo posición, LED según flags
  data[4] = id;                      // ID del motor

  byte packet[12];
  buildPacket(packet, id, CMD_S_JOG, data, 5);
  sendPacket(packet);
  delay(HERKULEX_MIN_CMD_GAP_MS);
}

// Movimiento a posición usando I_JOG (individual, tiempo por servo)
// playtime: tiempo en unidades de 11.2ms
// ledFlags: flags de LED o 0 para sin LED
void herkulex_iJog(byte id, uint16_t position, byte playtime, byte ledFlags) {
  byte data[5];
  byte setByte = 0x00;  // Modo posición por defecto

  // Configurar flags de LED si se especifican
  if (ledFlags & LED_GREEN) setByte |= SET_LED_GREEN;
  if (ledFlags & LED_BLUE) setByte |= SET_LED_BLUE;
  if (ledFlags & LED_RED) setByte |= SET_LED_RED;

  // Formato I_JOG: [JOG_LSB][JOG_MSB][SET][ID][playtime]
  data[0] = position & 0xFF;         // Posición LSB
  data[1] = (position >> 8) & 0xFF;  // Posición MSB
  data[2] = setByte;                 // SET: Modo posición, LED según flags
  data[3] = id;                      // ID del motor
  data[4] = playtime;                // Tiempo de movimiento (11.2 ms * N)

  byte packet[12];
  buildPacket(packet, id, CMD_I_JOG, data, 5);
  sendPacket(packet);
  delay(HERKULEX_MIN_CMD_GAP_MS);
}

// Giro continuo (modo velocidad) usando S_JOG
// speed: velocidad en cuentas (0-1023, dirección según signo)
// playtime: tiempo en unidades de 11.2ms
// ledFlags: flags de LED o 0 para sin LED
void herkulex_continuousRotation(byte id, int16_t speed, byte playtime, byte ledFlags) {
  byte data[5];
  byte setByte = SET_MODE_VELOCITY;  // Modo velocidad

  // Configurar flags de LED si se especifican
  if (ledFlags & LED_GREEN) setByte |= SET_LED_GREEN;
  if (ledFlags & LED_BLUE) setByte |= SET_LED_BLUE;
  if (ledFlags & LED_RED) setByte |= SET_LED_RED;

  // Formato S_JOG: [playtime][JOG_LSB][JOG_MSB][SET][ID]
  data[0] = playtime;                // Tiempo de movimiento
  data[1] = speed & 0xFF;            // Velocidad LSB
  data[2] = (speed >> 8) & 0xFF;     // Velocidad MSB
  data[3] = setByte;                 // SET: Modo velocidad, LED según flags
  data[4] = id;                      // ID del motor

  byte packet[12];
  buildPacket(packet, id, CMD_S_JOG, data, 5);
  sendPacket(packet);
  delay(HERKULEX_MIN_CMD_GAP_MS);
}

void commandEnableTorque(byte id) {
  byte data[3] = {
    ADDR_TORQUE_CTRL, 0x01, 0x60  // 0x60 = Torque ON, LED off
  };

  byte packet[10];
  buildPacket(packet, id, CMD_RAM_WRITE, data, 3);
  sendPacket(packet); 
}

void commandTorqueFree(byte id) {
  byte data[3] = {
    ADDR_TORQUE_CTRL, 0x01, 0x00  // 0x00 = Torque Free
  };

  byte packet[10];
  buildPacket(packet, id, CMD_RAM_WRITE, data, 3);
  sendPacket(packet); 
}

void commandBrakeOn(byte id) {
  byte data[3] = {
    ADDR_TORQUE_CTRL, 0x01, 0x40  // 0x40 = Break On
  };

  byte packet[10];
  buildPacket(packet, id, CMD_RAM_WRITE, data, 3);
  sendPacket(packet); 
}
void commandReadStatus(uint8_t id) {
  byte packet[7];
  buildPacket(packet, id, CMD_STAT, nullptr, 0);
  Serial1.write(packet, packet[2]);

  delay(5);  // Tiempo de espera para respuesta

  int timeout = 1000;
  while (Serial1.available() < 9 && timeout-- > 0) delay(1);

  if (Serial1.available() >= 9) {
    byte response[9];
    for (int i = 0; i < 9; i++) {
      response[i] = Serial1.read();
    }

    byte status1 = response[7];
    byte status2 = response[8];

    Serial.print("Status1: 0x");
    Serial.println(status1, HEX);
    Serial.print("Status2: 0x");
    Serial.println(status2, HEX);

    // Opcional: decodificar errores comunes
    if (status1 != 0 || status2 != 0) {
      Serial.println("→ Errores detectados:");
      if (status1 & 0x01) Serial.println("  - Exceso de temperatura");
      if (status1 & 0x02) Serial.println("  - Error de instrucción");
      if (status1 & 0x04) Serial.println("  - Exceso de carga");
      if (status1 & 0x08) Serial.println("  - Voltaje fuera de rango");
      if (status2 & 0x01) Serial.println("  - Posición fuera de rango");
      if (status2 & 0x02) Serial.println("  - Error de EEPROM");
      // Agregar más si querés
    } else {
      Serial.println("→ Sin errores reportados.");
    }

  } else {
    Serial.println("No se recibió respuesta del servo.");
  }
}

void commandSetCalibrationDiff(byte id, int16_t offset) {
  byte data[4] = {
    ADDR_CALIB_DIFF_L, 0x02,
    byte(offset & 0xFF), byte((offset >> 8) & 0xFF)
  };

  byte packet[11];
  buildPacket(packet, id, CMD_EEP_WRITE, data, 4);
  sendPacket(packet);

  Serial.print("Calibración diferencial seteada a: ");
  Serial.println(offset);
}

// ============================================================================
// ESTRUCTURA PARA CONTROL DE TORQUE ADAPTATIVO
// ============================================================================

struct HerkulexMotor {
  byte id;
  int16_t offset = 0;  // offset de calibración
};

struct TorqueMonitor {
  uint16_t pwmCurrent;        // PWM actual leído
  uint16_t pwmMax;            // PWM máximo configurado
  uint16_t pwmThreshold;      // Umbral de sobrecarga
  bool overloadDetected;      // Flag de sobrecarga detectada
  unsigned long lastCheck;    // Última vez que se verificó
  float currentVelocity;      // Velocidad actual ajustada
};

// Monitorea PWM y detecta sobrecarga
bool herkulex_monitorOverload(byte id, TorqueMonitor& monitor) {
  uint16_t pwm = 0;
  if (!herkulex_readPWM(id, pwm)) {
    return false;
  }
  
  monitor.pwmCurrent = pwm;
  monitor.lastCheck = millis();
  
  // Detectar sobrecarga si PWM está por encima del umbral
  if (pwm > monitor.pwmThreshold) {
    monitor.overloadDetected = true;
    return true;
  } else {
    monitor.overloadDetected = false;
    return false;
  }
}

// Ajusta velocidad dinámicamente basado en PWM
// Retorna la velocidad ajustada en cuentas por segundo
float herkulex_adaptiveVelocityControl(byte id, TorqueMonitor& monitor, float baseVelocity) {
  if (!herkulex_monitorOverload(id, monitor)) {
    // Sin sobrecarga, usar velocidad base
    monitor.currentVelocity = baseVelocity;
    return baseVelocity;
  }
  
  // Reducir velocidad si hay sobrecarga
  float reductionFactor = 0.8f;  // Reducir 20%
  monitor.currentVelocity = baseVelocity * reductionFactor;
  
  // Límite mínimo
  if (monitor.currentVelocity < 50.0f) {
    monitor.currentVelocity = 50.0f;
  }
  
  return monitor.currentVelocity;
}

// ============================================================================
// ESTIMACIÓN DE TORQUE Y CORRIENTE BASADA EN PWM
// ============================================================================

// Estima el torque actual en Nm basado en el PWM leído
// pwm: valor de PWM (0-1023)
// pwmMax: PWM máximo configurado (por defecto 1023)
// Retorna: torque estimado en Nm
float herkulex_estimateTorqueFromPWM(uint16_t pwm, uint16_t pwmMax) {
  if (pwmMax == 0) pwmMax = HERKULEX_PWM_MAX_VALUE;
  
  // PWM es proporcional al torque
  // Torque = (PWM / PWM_MAX) * TORQUE_MAX
  float ratio = (float)pwm / (float)pwmMax;
  if (ratio > 1.0f) ratio = 1.0f;  // Limitar a 100%
  
  return ratio * HERKULEX_TORQUE_MAX_NM;
}

// Estima la corriente actual en mA basada en el PWM leído
// pwm: valor de PWM (0-1023)
// pwmMax: PWM máximo configurado (por defecto 1023)
// Retorna: corriente estimada en mA
float herkulex_estimateCurrentFromPWM(uint16_t pwm, uint16_t pwmMax) {
  if (pwmMax == 0) pwmMax = HERKULEX_PWM_MAX_VALUE;
  
  // PWM es proporcional a la corriente
  // Corriente = (PWM / PWM_MAX) * CURRENT_MAX
  float ratio = (float)pwm / (float)pwmMax;
  if (ratio > 1.0f) ratio = 1.0f;  // Limitar a 100%
  
  return ratio * HERKULEX_CURRENT_MAX_MA;
}

// Obtiene el porcentaje de torque actual (0-100%)
// pwm: valor de PWM (0-1023)
// pwmMax: PWM máximo configurado (por defecto 1023)
// Retorna: porcentaje de torque (0.0 - 100.0)
float herkulex_getTorquePercentage(uint16_t pwm, uint16_t pwmMax) {
  if (pwmMax == 0) pwmMax = HERKULEX_PWM_MAX_VALUE;
  
  float ratio = (float)pwm / (float)pwmMax;
  if (ratio > 1.0f) ratio = 1.0f;
  
  return ratio * 100.0f;
}

// Lee PWM y calcula torque estimado
// id: ID del servo
// torque: variable donde se guardará el torque estimado en Nm
// pwmMax: PWM máximo configurado (0 = usar valor por defecto)
// Retorna: true si se pudo leer el PWM, false en caso de error
bool herkulex_readTorque(byte id, float& torque, uint16_t pwmMax) {
  uint16_t pwm = 0;
  if (!herkulex_readPWM(id, pwm)) {
    return false;
  }
  
  torque = herkulex_estimateTorqueFromPWM(pwm, pwmMax);
  return true;
}

// Lee PWM y calcula corriente estimada
// id: ID del servo
// current: variable donde se guardará la corriente estimada en mA
// pwmMax: PWM máximo configurado (0 = usar valor por defecto)
// Retorna: true si se pudo leer el PWM, false en caso de error
bool herkulex_readCurrent(byte id, float& current, uint16_t pwmMax) {
  uint16_t pwm = 0;
  if (!herkulex_readPWM(id, pwm)) {
    return false;
  }
  
  current = herkulex_estimateCurrentFromPWM(pwm, pwmMax);
  return true;
}

// Obtiene información completa de torque/corriente
// id: ID del servo
// pwm: valor de PWM leído
// torque: torque estimado en Nm
// current: corriente estimada en mA
// percentage: porcentaje de torque (0-100%)
// pwmMax: PWM máximo configurado (0 = usar valor por defecto)
// Retorna: true si se pudo leer, false en caso de error
bool herkulex_getTorqueInfo(byte id, uint16_t& pwm, float& torque, float& current, 
                            float& percentage, uint16_t pwmMax) {
  if (!herkulex_readPWM(id, pwm)) {
    return false;
  }
  
  if (pwmMax == 0) pwmMax = HERKULEX_PWM_MAX_VALUE;
  
  torque = herkulex_estimateTorqueFromPWM(pwm, pwmMax);
  current = herkulex_estimateCurrentFromPWM(pwm, pwmMax);
  percentage = herkulex_getTorquePercentage(pwm, pwmMax);
  
  return true;
}

// ============================================================================
// FUNCIONES DE LECTURA
// ============================================================================

// Lee voltaje actual (en décimas de voltio, ej: 120 = 12.0V)
bool herkulex_readVoltage(byte id, byte& voltage) {
  return readRAMByte(id, ADDR_VOLTAGE_SENSOR, voltage);
}

// Lee temperatura actual (en grados Celsius)
bool herkulex_readTemperature(byte id, byte& temperature) {
  return readRAMByte(id, ADDR_TEMPERATURE_SENSOR, temperature);
}

// Lee PWM actual (0-1023)
bool herkulex_readPWM(byte id, uint16_t& pwm) {
  return readRAMWord(id, ADDR_PWM_OUT_L, pwm);
}

// Lee modo de control actual (0=Posición, 1=Velocidad)
bool herkulex_readCurrentMode(byte id, byte& mode) {
  return readRAMByte(id, ADDR_CURRENT_MODE, mode);
}

// Lee posición absoluta (sin calibrar)
bool herkulex_readAbsolutePosition(byte id, uint16_t& position) {
  return readRAMWord(id, ADDR_POS_ABS_L, position);
}

// Lee posición objetivo
bool herkulex_readTargetPosition(byte id, uint16_t& position) {
  return readRAMWord(id, ADDR_POS_ABS_OBJ_L, position);
}

// Lee velocidad deseada actual
bool herkulex_readSpeed(byte id, uint16_t& speed) {
  return readRAMWord(id, ADDR_SPEED_GOAL_L, speed);
}

// ============================================================================
// FUNCIONES DE ESCRITURA
// ============================================================================

void commandSetAcceleration(byte id, byte accelRatio, byte accelTime) {
  byte data[4];

  data[0] = ADDR_RAM_ACCEL_RATIO;
  data[1] = 2;
  data[2] = accelRatio;      // 0–50 %
  data[3] = accelTime;       // ×11.2 ms

  byte packet[11];
  buildPacket(packet, id, CMD_RAM_WRITE, data, 4);
  sendPacket(packet);
  delay(HERKULEX_MIN_CMD_GAP_MS);

  Serial.print(F("Aceleración: ratio="));
  Serial.print(accelRatio);
  Serial.print(F("%, time="));
  Serial.print(accelTime * HERKULEX_PLAYTIME_UNIT_MS);
  Serial.println(F(" ms"));
}

// Configura PWM máximo (0-1023)
void herkulex_setMaxPWM(byte id, uint16_t maxPWM, bool writeToEEP) {
  if (maxPWM > HERKULEX_PWM_MAX_VALUE) maxPWM = HERKULEX_PWM_MAX_VALUE;
  
  byte data[4];
  data[0] = writeToEEP ? ADDR_ROM_PWM_MAX_L : ADDR_RAM_PWM_MAX_L;
  data[1] = 0x02;
  data[2] = maxPWM & 0xFF;         // LSB
  data[3] = (maxPWM >> 8) & 0xFF;  // MSB

  byte packet[11];
  buildPacket(packet, id, writeToEEP ? CMD_EEP_WRITE : CMD_RAM_WRITE, data, 4);
  sendPacket(packet);
  delay(HERKULEX_MIN_CMD_GAP_MS);
  
  if (writeToEEP) {
    Serial.println(F("⚠️ Cambios en EEP requieren REBOOT para aplicar"));
  }
}

// Configura PWM mínimo (0-254)
void herkulex_setMinPWM(byte id, byte minPWM, bool writeToEEP) {
  byte data[3];
  data[0] = writeToEEP ? ADDR_ROM_PWM_MIN : ADDR_RAM_PWM_MIN;
  data[1] = 0x01;
  data[2] = minPWM;

  byte packet[10];
  buildPacket(packet, id, writeToEEP ? CMD_EEP_WRITE : CMD_RAM_WRITE, data, 3);
  sendPacket(packet);
  delay(HERKULEX_MIN_CMD_GAP_MS);
  
  if (writeToEEP) {
    Serial.println(F("⚠️ Cambios en EEP requieren REBOOT para aplicar"));
  }
}

// Configura umbral de sobrecarga PWM (0-1023)
void herkulex_setOverloadThreshold(byte id, uint16_t threshold, bool writeToEEP) {
  if (threshold > HERKULEX_PWM_MAX_VALUE) threshold = HERKULEX_PWM_MAX_VALUE;
  
  byte data[4];
  data[0] = writeToEEP ? ADDR_ROM_PWM_OVERLOAD_L : ADDR_RAM_PWM_OVERLOAD_L;
  data[1] = 0x02;
  data[2] = threshold & 0xFF;         // LSB
  data[3] = (threshold >> 8) & 0xFF;  // MSB

  byte packet[11];
  buildPacket(packet, id, writeToEEP ? CMD_EEP_WRITE : CMD_RAM_WRITE, data, 4);
  sendPacket(packet);
  delay(HERKULEX_MIN_CMD_GAP_MS);
  
  if (writeToEEP) {
    Serial.println(F("⚠️ Cambios en EEP requieren REBOOT para aplicar"));
  }
}

// Movimiento seguro con cálculo automático de playtime
// velocity: velocidad objetivo en cuentas por segundo (0 = usar valor por defecto)
void herkulex_safeMoveTo(byte id, uint16_t targetPosition, float velocity) {
  // Verificar si la posición objetivo está dentro del rango permitido
  if (targetPosition < POS_MIN_DEF || targetPosition > POS_MAX_DEF) {
    Serial.print(F("⚠️ Posición "));
    Serial.print(targetPosition);
    Serial.print(F(" fuera de rango permitido ("));
    Serial.print(POS_MIN_DEF);
    Serial.print(F("–"));
    Serial.print(POS_MAX_DEF);
    Serial.println(F("). Movimiento cancelado."));
    return;
  }

  // Leer posición actual
  uint16_t currentPosition = readPosition(id);
  if (currentPosition == 0xFFFF) {
    Serial.println(F("⚠️ Error al leer posición actual. Movimiento cancelado."));
    return;
  }

  // Usar velocidad por defecto si no se especifica
  if (velocity <= 0) {
    velocity = HERKULEX_DEFAULT_VELOCITY_COUNTS_PER_S;
  }

  // Calcular playtime basado en distancia y velocidad
  uint8_t playtime = herkulex_computePlaytime(currentPosition, targetPosition, velocity);

  // Ejecutar movimiento
  herkulex_moveTo(id, targetPosition, playtime, LED_OFF);

  // Log
  Serial.print(F("✅ Moviendo de "));
  Serial.print(currentPosition);
  Serial.print(F(" a "));
  Serial.print(targetPosition);
  Serial.print(F(" con playtime: "));
  Serial.print(playtime);
  Serial.print(F(" ("));
  Serial.print(playtime * HERKULEX_PLAYTIME_UNIT_MS);
  Serial.print(F(" ms, velocidad: "));
  Serial.print(velocity);
  Serial.println(F(" cuentas/s)"));
}

// Movimiento sincronizado múltiple usando S_JOG
// ids: array de IDs de servos
// posiciones: array de posiciones objetivo
// cantidad: número de servos
// playtime: tiempo compartido en unidades de 11.2ms
// ledFlags: flags de LED para todos los servos (0 = sin LED)
void herkulex_sJogMultiMove(byte ids[], uint16_t posiciones[], byte cantidad, byte playtime, byte ledFlags) {
  const byte CMD = CMD_S_JOG;       // Comando de movimiento múltiple
  const byte ID_BROADCAST = 0xFE;   // ID de broadcast para todos los motores

  byte data[1 + 4 * cantidad];      // 1 byte playtime + 4 por motor
  data[0] = playtime;               // Playtime compartido

  byte setByte = 0x00;  // Modo posición por defecto
  // Configurar flags de LED si se especifican
  if (ledFlags & LED_GREEN) setByte |= SET_LED_GREEN;
  if (ledFlags & LED_BLUE) setByte |= SET_LED_BLUE;
  if (ledFlags & LED_RED) setByte |= SET_LED_RED;

  byte idx = 1;
  for (byte i = 0; i < cantidad; i++) {
    data[idx++] = posiciones[i] & 0xFF;         // Pos_L
    data[idx++] = (posiciones[i] >> 8) & 0xFF;  // Pos_H
    data[idx++] = setByte;                      // SET: modo posición, LED según flags
    data[idx++] = ids[i];                       // ID del motor
  }

  byte packet[7 + sizeof(data)];  // Cabecera + payload
  buildPacket(packet, ID_BROADCAST, CMD, data, idx);
  sendPacket(packet);
  delay(HERKULEX_MIN_CMD_GAP_MS);
}


void moverAPoseInicial() {
  //uint16_t pose[] = POSE_INICIAL;
  //herkulex_sJogMove(motorIDs, pose, 5, 400);  // 50 = ~560 ms de movimiento
}

void revisarErroresMotores(byte ids[], byte cantidad) {
  for (byte i = 0; i < cantidad; i++) {
    byte se = 0, de = 0;
    if (herkulex_readStatus(ids[i], se, de)) {
      Serial.print("🧩 Motor ID ");
      Serial.println(ids[i]);
      printHerkulexErrors(se, de);
    } else {
      Serial.print("⚠️ No se recibió respuesta del motor ");
      Serial.println(ids[i]);
    }
  }
}

void commandSetServoPolicy(byte id, byte policy) {
  byte data[3] = { 0x03, 0x01, policy }; // Addr, length, value

  byte packet[10];
  buildPacket(packet, id, CMD_RAM_WRITE, data, 3);
  sendPacket(packet);
}
// --- helpers de debug ---
void dumpBytes(const uint8_t* buf, int n, const __FlashStringHelper* tag) {
  Serial.print(tag);
  Serial.print(F(" ("));
  Serial.print(n);
  Serial.println(F(" bytes):"));
  for (int i = 0; i < n; i++) {
    if (buf[i] < 16) Serial.print('0');
    Serial.print(buf[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

void flushSerial1() {
  while (Serial1.available()) (void)Serial1.read();
}



// buildPacket(packet, id, CMD_RAM_READ, data, dataLen) ya la tenés
// OJO: packet[2] debe contener LENGTH

// Lee exactamente 'need' bytes con timeout (ms). Devuelve cuántos leyó.
int readExact( HardwareSerial &port, uint8_t *dst, int need, unsigned long timeout_ms ) {
  unsigned long t0 = millis();
  int got = 0;
  while (got < need && (millis() - t0) < timeout_ms) {
    int a = port.available();
    if (a > 0) {
      int chunk = min(a, need - got);
      for (int i = 0; i < chunk; i++) dst[got++] = port.read();
    }
  }
  return got;
}

uint16_t readPosition_debug(uint8_t id) {
  // 1) Limpiar basura previa
  flushSerial1();

  // 2) Construir RAM_READ para 0x3A (2 bytes)
  uint8_t data[2] = { ADDR_POS_CALIB_L, 0x02 };
  uint8_t tx[9];
  buildPacket(tx, id, CMD_RAM_READ, data, 2);

  // Log TX
  dumpBytes(tx, tx[2], F("TX"));

  // 3) Enviar
  Serial1.write(tx, tx[2]);

  // 4) Leer cabecera fija (al menos 7 bytes: FF FF LEN ID CMD CHK1 CHK2)
  uint8_t hdr[7];
  int got = readExact(Serial1, hdr, 7, 100); // 30 ms suele alcanzar
  if (got < 7) {
    Serial.println(F("ERR: header incompleto"));
    return 0xFFFF;
  }

  // Sincronizar cabecera (buscar 0xFF 0xFF)
  int shift = 0;
  while (shift < 6 && !(hdr[shift] == 0xFF && hdr[shift+1] == 0xFF)) shift++;
  if (shift > 0) {
    // desplazamos la ventana: necesitamos rellenar lo faltante
    for (int i = 0; i < 7 - shift; i++) hdr[i] = hdr[i + shift];
    int need = shift;
    if (readExact(Serial1, hdr + (7 - shift), need, 100) < need) {
      Serial.println(F("ERR: resync cabecera"));
      return 0xFFFF;
    }
  }

  // dump header
  dumpBytes(hdr, 7, F("HDR"));

  uint8_t LEN = hdr[2];
  // El paquete total tiene LEN bytes desde 'LENGTH' inclusive.
  // Ya leímos 7, faltan (LEN - 7) si LEN >= 7.
  if (LEN < 7) {
    Serial.print(F("ERR: LEN invalido=")); Serial.println(LEN);
    return 0xFFFF;
  }

  uint8_t restLen = LEN - 7;
  uint8_t rest[64];
  if (restLen > sizeof(rest)) {
    Serial.println(F("ERR: paquete demasiado grande"));
    return 0xFFFF;
  }

  if (restLen) {
    if (readExact(Serial1, rest, restLen, 100) < restLen) {
      Serial.println(F("ERR: payload incompleto"));
      return 0xFFFF;
    }
    dumpBytes(rest, restLen, F("PAY"));
  }

  // Estructura típica de respuesta RAM_READ:
  // [FF][FF][LEN][ID][CMD][CHK1][CHK2][ADDR][NBYTES][DATA...]
  // Nosotros pedimos 2 bytes -> DATA[0]=LSB, DATA[1]=MSB
  if (restLen < 2 + 2) { // necesitamos al menos ADDR (1), NBYTES (1), DATA(2)
    Serial.println(F("ERR: payload muy corto"));
    return 0xFFFF;
  }

  uint8_t addrEcho = rest[0];
  uint8_t nbytes   = rest[1];

  if (addrEcho != ADDR_POS_CALIB_L || nbytes != 0x02) {
    Serial.print(F("WARN: eco addr=")); Serial.print(addrEcho, HEX);
    Serial.print(F(" nbytes=")); Serial.println(nbytes);
    // seguimos igual, pero ojo
  }

  // DATA
  if (restLen < 2 + nbytes) {
    Serial.println(F("ERR: faltan datos"));
    return 0xFFFF;
  }

  uint8_t lsb = rest[2];
  uint8_t msb = rest[3];

  Serial.print(F("LSB=")); Serial.print(lsb, HEX);
  Serial.print(F(" MSB=")); Serial.println(msb, HEX);

  uint16_t pos = (uint16_t)lsb | ((uint16_t)msb << 8);
  Serial.print(F("POS RAW=")); Serial.println(pos);

  return pos;
}
