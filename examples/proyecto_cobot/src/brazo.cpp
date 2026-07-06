#include "brazo.h"
#include "poses.h"

// Inicializar comunicación Serial1
void Brazo::begin() {
  Serial1.begin(115200);
  delay(100);  // Pequeño delay para estabilización
}

void Brazo::applySafetyLimits() {
  Serial.println(F("-> Aplicando limites de corriente/torque..."));

  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    herkulex_setMaxPWM(motors[i].id, SAFETY_PWM_MAX[i], false);
    herkulex_setOverloadThreshold(motors[i].id, SAFETY_PWM_OVERLOAD[i], false);
    motors[i].pwmThreshold = SAFETY_PWM_MONITOR[i];

    Serial.print(F("  Motor ID "));
    Serial.print(motors[i].id);
    Serial.print(F(": maxPWM="));
    Serial.print(SAFETY_PWM_MAX[i]);
    Serial.print(F(" overload="));
    Serial.print(SAFETY_PWM_OVERLOAD[i]);
    Serial.print(F(" monitor="));
    Serial.println(SAFETY_PWM_MONITOR[i]);

    if (SAFETY_PWM_OVERLOAD[i] > SAFETY_PWM_MAX[i]) {
      Serial.println(F("  !! overload > max: alarma hardware NO disparara"));
    }
    if (SAFETY_PWM_MONITOR[i] >= SAFETY_PWM_OVERLOAD[i]) {
      Serial.println(F("  !! monitor >= overload: revisar umbrales"));
    }

    delay(15);
  }

  Serial.println(F("✓ Limites de seguridad aplicados"));
}

bool Brazo::checkMotorPwmHigh(uint8_t motorIndex, uint16_t& pwmOut, bool& readOk) {
  pwmOut = 0;
  readOk = false;

  uint16_t pwm = 0;
  if (!herkulex_readPWM(motors[motorIndex].id, pwm)) {
    return false;
  }

  readOk = true;
  pwmOut = pwm;
  return motors[motorIndex].pwmThreshold > 0 && pwm > motors[motorIndex].pwmThreshold;
}

bool Brazo::checkMotorOverload(uint8_t motorIndex, uint16_t pwm, bool readOk,
                               const uint16_t* baseline, bool baselineReady) {
  if (!readOk) {
    return false;
  }

  if (motorIndex == MOTOR_INDEX_BASE && baselineReady && SAFETY_DEMO_MODE) {
    const uint16_t deltaLimit = baseline[motorIndex] + SAFETY_PWM_DELTA_BASE;
    if (pwm >= deltaLimit && pwm > motors[motorIndex].pwmThreshold) {
      return true;
    }
  }

  return motors[motorIndex].pwmThreshold > 0 && pwm > motors[motorIndex].pwmThreshold;
}

bool Brazo::checkMotorStatusError(uint8_t motorIndex) {
  byte statusError = 0;
  byte detailError = 0;
  if (!herkulex_readStatus(motors[motorIndex].id, statusError, detailError)) {
    return false;
  }

  if ((statusError & 0x40) || (statusError & ERR_OVERLOAD)) {
    return true;
  }
  // Solo sobre-corriente; el bit stall (0x08) dispara mucho en movimiento normal
  return (detailError & 0x40) != 0;
}

bool Brazo::waitAndMonitorMove(uint16_t playtimeMs) {
  const unsigned long moveStart = millis();
  const unsigned long deadline = moveStart + playtimeMs + SAFETY_MONITOR_TAIL_MS;
  unsigned long lastCheck = 0;
  uint16_t peakPwm[NUM_MOTORES] = {};
  uint16_t pwmBaseline[NUM_MOTORES] = {};
  uint8_t pwmHits[NUM_MOTORES] = {};
  uint8_t statusHits[NUM_MOTORES] = {};
  bool baselineReady = false;

  while (millis() < deadline) {
    if (!baselineReady && millis() - moveStart >= SAFETY_BASELINE_DELAY_MS) {
      for (uint8_t i = 0; i < NUM_MOTORES; i++) {
        uint16_t pwm = 0;
        if (herkulex_readPWM(motors[i].id, pwm)) {
          pwmBaseline[i] = pwm;
        }
        delay(3);
      }
      baselineReady = true;
#if SAFETY_DEBUG_PWM
      Serial.print(F("[MON] baseline base="));
      Serial.print(pwmBaseline[MOTOR_INDEX_BASE]);
      Serial.print(F(" delta>="));
      Serial.println(pwmBaseline[MOTOR_INDEX_BASE] + SAFETY_PWM_DELTA_BASE);
#endif
    }

    if (millis() - lastCheck >= SAFETY_MONITOR_INTERVAL_MS) {
      lastCheck = millis();

      while (Serial1.available()) {
        (void)Serial1.read();
      }

#if SAFETY_DEBUG_PWM
      Serial.print(F("[MON] "));
#endif

      for (uint8_t i = 0; i < NUM_MOTORES; i++) {
        uint16_t pwm = 0;
        bool readOk = false;
        (void)checkMotorPwmHigh(i, pwm, readOk);
        const bool pwmHigh = checkMotorOverload(i, pwm, readOk, pwmBaseline, baselineReady);
#if SAFETY_USE_STATUS_CHECK
        const bool statusErr = checkMotorStatusError(i);
#else
        const bool statusErr = false;
#endif

        const uint8_t hitsNeeded = (i == MOTOR_INDEX_BASE)
          ? SAFETY_BASE_CONSECUTIVE_HITS
          : SAFETY_PWM_CONSECUTIVE_HITS;

#if SAFETY_DEBUG_PWM
        Serial.print(motors[i].id);
        Serial.print(F(":"));
        if (!readOk) {
          Serial.print(F("ERR"));
        } else {
          Serial.print(pwm);
        }
        if (i == MOTOR_INDEX_BASE) {
          Serial.print(F("*"));
        }
        if (i < NUM_MOTORES - 1) {
          Serial.print(F(" "));
        }
#endif

        if (readOk && pwm > peakPwm[i]) {
          peakPwm[i] = pwm;
        }

        if (statusErr) {
          statusHits[i]++;
          if (statusHits[i] >= hitsNeeded) {
#if SAFETY_DEBUG_PWM
            Serial.println();
#endif
            retreatToStandbyOnOverload(i, pwm);
            return false;
          }
        } else {
          statusHits[i] = 0;
        }

        if (readOk) {
          if (pwmHigh) {
            pwmHits[i]++;
            if (pwmHits[i] >= hitsNeeded) {
#if SAFETY_DEBUG_PWM
              Serial.println();
#endif
              retreatToStandbyOnOverload(i, pwm);
              return false;
            }
          } else {
            pwmHits[i] = 0;
          }
        }

        delay(3);
      }

#if SAFETY_DEBUG_PWM
      Serial.println();
#endif
    }
    delay(2);
  }

#if SAFETY_DEBUG_PWM
  Serial.print(F("[MON] pico PWM: "));
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    Serial.print(motors[i].id);
    Serial.print(F("="));
    Serial.print(peakPwm[i]);
    if (i < NUM_MOTORES - 1) {
      Serial.print(F(" "));
    }
  }
  Serial.println();
#endif

  return true;
}

void Brazo::retreatToStandbyOnOverload(uint8_t motorIndex, uint16_t pwm) {
  float torque = 0.0f;
  float current = 0.0f;
  herkulex_readTorque(motors[motorIndex].id, torque);
  herkulex_readCurrent(motors[motorIndex].id, current);

  Serial.println();
  Serial.println(F("⚠️ SOBRECARGA detectada durante movimiento"));
  Serial.print(F("  Motor ID "));
  Serial.print(motors[motorIndex].id);
  if (motorIndex == MOTOR_INDEX_BASE) {
    Serial.print(F(" (BASE)"));
  }
  Serial.print(F(" PWM="));
  Serial.print(pwm);
  Serial.print(F(" torque~"));
  Serial.print(torque, 2);
  Serial.print(F(" Nm corriente~"));
  Serial.print(current, 0);
  Serial.println(F(" mA"));

  clearAllErrors();
  Serial.println(F("-> Retirada a POSE_STANDBY por seguridad"));
  goPose(POSE_STANDBY, SAFETY_RETREAT_MS, false);
}

// Llenar array con IDs de los motores
void Brazo::fillIds(uint8_t outIds[NUM_MOTORES]) const {
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    outIds[i] = motors[i].id;
  }
}

// Mover brazo a una pose específica
bool Brazo::goPose(const Pose& pose, uint16_t playtimeMs, bool safetyMonitor) {
  // Convertir playtime de milisegundos a unidades de 11.2ms
  // playtime = round(playtimeMs / 11.2)
  // Limitar a rango válido: 1-254 (0 no es válido, 255 es reservado)
  float playtimeFloat = (float)playtimeMs / 11.2f;
  if (playtimeFloat < 1.0f) playtimeFloat = 1.0f;
  if (playtimeFloat > 254.0f) playtimeFloat = 254.0f;
  byte playtime = (byte)(playtimeFloat + 0.5f);  // Redondear

  // Preparar arrays para el comando multi-move
  uint8_t ids[NUM_MOTORES];
  uint16_t posiciones[NUM_MOTORES];

  // Llenar IDs
  fillIds(ids);

  // Aplicar clamp a cada posición según los límites del motor
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    posiciones[i] = motors[i].clamp(pose.pos[i]);
  }

  // Limpiar buffer antes de mover (importante para evitar errores)
  // Esto asegura que no haya datos residuales que interfieran con el comando
  while (Serial1.available()) (void)Serial1.read();
  delay(20);  // Delay un poco más largo para asegurar limpieza completa

  // Ejecutar movimiento sincronizado
  herkulex_sJogMultiMove(ids, posiciones, NUM_MOTORES, playtime);

  // Pequeño delay después del movimiento para que el comando se procese
  delay(50);

  if (!safetyMonitor) {
    delay(playtimeMs);
    return true;
  }

  return waitAndMonitorMove(playtimeMs);
}

// Limpiar errores de todos los motores
void Brazo::clearAllErrors() {
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    commandClearError(motors[i].id);
    delay(10);  // Pequeño delay entre comandos
  }
}

// Check de todos los motores: activa torque uno por uno
bool Brazo::checkAll() {
  bool allOk = true;
  
  Serial.println(F("=== Iniciando check de motores ==="));
  
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    Serial.print(F("Motor "));
    Serial.print(motors[i].id);
    Serial.print(F(": "));
    
    bool ok = motors[i].check();
    
    if (ok) {
      Serial.println(F("✓ OK"));
    } else {
      Serial.println(F("✗ ERROR"));
      allOk = false;
    }
    
    // Pequeño delay entre motores
    delay(100);
  }
  
  Serial.println();
  if (allOk) {
    Serial.println(F("✓ Todos los motores responden correctamente"));
  } else {
    Serial.println(F("⚠️ Algunos motores no responden"));
  }
  
  return allOk;
}

// Leer y mostrar posición de todos los motores
void Brazo::printAllPositions() {
  Serial.println(F("=== Posiciones actuales de los motores ==="));
  Serial.println();
  
  // Array para guardar las posiciones leídas
  uint16_t posiciones[NUM_MOTORES];
  
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    Serial.print(F("Motor ID "));
    Serial.print(motors[i].id);
    Serial.print(F(": "));
    
    uint16_t pos = motors[i].getPosition();
    posiciones[i] = pos;  // Guardar para el resumen
    
    if (pos != 0xFFFF) {
      Serial.print(pos);
      
      // Mostrar si está dentro de los límites (si están definidos)
      if (motors[i].posMin > 0 || motors[i].posMax > 0) {
        Serial.print(F(" ["));
        if (motors[i].posMin > 0) {
          Serial.print(F("min:"));
          Serial.print(motors[i].posMin);
        }
        if (motors[i].posMin > 0 && motors[i].posMax > 0) {
          Serial.print(F(" "));
        }
        if (motors[i].posMax > 0) {
          Serial.print(F("max:"));
          Serial.print(motors[i].posMax);
        }
        Serial.print(F("]"));
        
        // Indicar si está fuera de límites
        if (motors[i].posMin > 0 && pos < motors[i].posMin) {
          Serial.print(F(" ⚠️ FUERA DE LIMITE (min)"));
        } else if (motors[i].posMax > 0 && pos > motors[i].posMax) {
          Serial.print(F(" ⚠️ FUERA DE LIMITE (max)"));
        }
      }
      
      Serial.println();
    } else {
      Serial.println(F("ERROR - No se pudo leer posición"));
    }
    
    // Pequeño delay entre lecturas
    delay(50);
  }
  
  Serial.println();
  Serial.println(F("=== Resumen ==="));
  Serial.print(F("ID:  "));
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    Serial.print(motors[i].id);
    if (i < NUM_MOTORES - 1) Serial.print(F("  "));
  }
  Serial.println();
  Serial.print(F("Pos: "));
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    if (posiciones[i] != 0xFFFF) {
      // Formatear con 5 dígitos
      if (posiciones[i] < 10000) Serial.print(F("0"));
      if (posiciones[i] < 1000) Serial.print(F("0"));
      if (posiciones[i] < 100) Serial.print(F("0"));
      if (posiciones[i] < 10) Serial.print(F("0"));
      Serial.print(posiciones[i]);
    } else {
      Serial.print(F("ERR"));
    }
    if (i < NUM_MOTORES - 1) Serial.print(F(" "));
  }
  Serial.println();
  Serial.println(F("=== Fin de posiciones ==="));
}

void Brazo::printTorqueSnapshot() {
  Serial.println(F("=== PWM / torque (empujar brazo a mano y repetir 't') ==="));
  Serial.println(F("ID  PWM  umbral  torque~Nm  mA   status"));

  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    while (Serial1.available()) {
      (void)Serial1.read();
    }

    const byte id = motors[i].id;
    uint16_t pwm = 0;
    float torque = 0.0f;
    float current = 0.0f;
    byte statusError = 0;
    byte detailError = 0;

    const bool pwmOk = herkulex_readPWM(id, pwm);
    if (pwmOk) {
      herkulex_readTorque(id, torque);
      herkulex_readCurrent(id, current);
    }
    const bool statusOk = herkulex_readStatus(id, statusError, detailError);

    Serial.print(F(" "));
    Serial.print(id);
    Serial.print(F("   "));
    if (pwmOk) {
      Serial.print(pwm);
    } else {
      Serial.print(F("ERR"));
    }
    Serial.print(F("   "));
    Serial.print(motors[i].pwmThreshold);
    Serial.print(F("     "));
    if (pwmOk) {
      Serial.print(torque, 2);
      Serial.print(F("      "));
      Serial.print((int)current);
    } else {
      Serial.print(F("-         -"));
    }
    Serial.print(F("   "));
    if (statusOk) {
      Serial.print(F("0x"));
      Serial.print(statusError, HEX);
      Serial.print(F("/0x"));
      Serial.println(detailError, HEX);
    } else {
      Serial.println(F("ERR"));
    }

    delay(20);
  }

  Serial.println(F("Regla: MONITOR < OVERLOAD <= MAX (ver brazo_config.h)"));
  if (SAFETY_DEMO_MODE) {
    Serial.print(F("Demo: motor base ID "));
    Serial.print(motors[MOTOR_INDEX_BASE].id);
    Serial.print(F(" dispara si PWM sube +"));
    Serial.print(SAFETY_PWM_DELTA_BASE);
    Serial.println(F(" sobre baseline al inicio del movimiento"));
  }
}

// Mover brazo a posiciones directamente (sin estructura Pose)
bool Brazo::goRaw(const uint16_t posiciones[NUM_MOTORES], uint16_t playtimeMs, bool safetyMonitor) {
  // Convertir playtime de milisegundos a unidades de 11.2ms
  float playtimeFloat = (float)playtimeMs / 11.2f;
  if (playtimeFloat < 1.0f) playtimeFloat = 1.0f;
  if (playtimeFloat > 254.0f) playtimeFloat = 254.0f;
  byte playtime = (byte)(playtimeFloat + 0.5f);  // Redondear

  // Preparar arrays para el comando multi-move
  uint8_t ids[NUM_MOTORES];
  uint16_t posicionesClamped[NUM_MOTORES];

  // Llenar IDs
  fillIds(ids);

  // Aplicar clamp a cada posición según los límites del motor
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    posicionesClamped[i] = motors[i].clamp(posiciones[i]);
  }

  // Limpiar buffer antes de mover (importante para evitar errores)
  while (Serial1.available()) (void)Serial1.read();
  delay(20);

  // Ejecutar movimiento sincronizado
  herkulex_sJogMultiMove(ids, posicionesClamped, NUM_MOTORES, playtime);

  // Pequeño delay después del movimiento para que el comando se procese
  delay(50);

  if (!safetyMonitor) {
    delay(playtimeMs);
    return true;
  }

  return waitAndMonitorMove(playtimeMs);
}

// Activar torque de todos los motores
void Brazo::torqueAllOn() {
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    motors[i].torqueOn();
    delay(10);  // Pequeño delay entre comandos
  }
}

// Desactivar torque de todos los motores (modo teach-in)
void Brazo::torqueAllOff() {
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    motors[i].torqueOff();
    delay(10);  // Pequeño delay entre comandos
  }
}

