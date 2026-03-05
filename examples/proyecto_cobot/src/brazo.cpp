#include "brazo.h"
#include "poses.h"
// Inicializar comunicación Serial1
void Brazo::begin() {
  Serial1.begin(115200);
  delay(100);  // Pequeño delay para estabilización
}

// Llenar array con IDs de los motores
void Brazo::fillIds(uint8_t outIds[NUM_MOTORES]) const {
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    outIds[i] = motors[i].id;
  }
}

// Mover brazo a una pose específica
bool Brazo::goPose(const Pose& pose, uint16_t playtimeMs) {
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

  return true;
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

// Mover brazo a posiciones directamente (sin estructura Pose)
bool Brazo::goRaw(const uint16_t posiciones[NUM_MOTORES], uint16_t playtimeMs) {
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

  return true;
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

