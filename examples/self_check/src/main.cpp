/*
  HerkuleX DRS-0602 - SELF CHECK (Scanner + Tester)
  Usa herkulex_readStatus() como "ping" para detectar servos.

  Requisitos:
  - Serial monitor a 115200 (o el que elijas)
  - Serial1 configurado para comunicación con servos (115200 baudios por defecto)

  NOTAS:
  - commandSetID() escribe EEPROM y requiere herkulex_reboot().
  - Cambiar ID automáticamente NO recomendado. Se deja como opción manual.
*/

#include <Arduino.h>
#include "herkulex_utils.h"

// -------------------- Config --------------------
static const uint32_t PC_BAUD = 115200;
static const uint32_t SERVO_BAUD = 115200;  // Baudrate de comunicación con servos

// Rango de IDs válidos (0..253)
static const int ID_MIN = 0;
static const int ID_MAX = 253;

// Movimiento seguro: delta pequeño (en unidades de "position" de tu librería).
// Ajustá esto según tu escala real. (Ej: si 1 unidad ~ 0.325°, 20-50 suele ser poco)
static const int MOVE_DELTA = 30;

// Si tu safeMoveTo usa "velocity" en counts/s y 0 = default:
static const float SAFE_VEL = 0;  // 0 = usar valor por defecto (250 cuentas/s)

// Tiempo entre lecturas (ms)
static const uint16_t WAIT_SHORT = 120;
static const uint16_t WAIT_MOVE  = 700;

// -------------------- Forward declarations --------------------
int scanIds(uint8_t* out, int maxOut, int minId = ID_MIN, int maxId = ID_MAX);
int scanIdsQuick(uint8_t* out, int maxOut);
int scanIdsDeep(uint8_t* out, int maxOut);
void printIdList(uint8_t* foundIds, int foundCount, const char* label);
bool isIdInUse(uint8_t id, uint8_t* foundIds, int foundCount);

// -------------------- Helpers Serial --------------------
String readLine() {
  String s;
  while (true) {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n') return s;
      if (c != '\r') s += c;
    }
    delay(5);
  }
}

int readInt(const char* prompt) {
  Serial.print(prompt);
  String s = readLine();
  s.trim();
  return s.toInt();
}

bool readYesNo(const char* prompt) {
  Serial.print(prompt);
  Serial.print(" (y/n): ");
  String s = readLine();
  s.trim();
  s.toLowerCase();
  return (s == "y" || s == "yes" || s == "s" || s == "si");
}

void printDivider() {
  Serial.println("--------------------------------------------------");
}

// -------------------- Detection ("Ping") --------------------
bool pingByStatus(uint8_t id) {
  uint8_t st = 0, dt = 0;
  bool ok = herkulex_readStatus(id, st, dt);
  return ok;
}

// -------------------- Identify (LED blink) --------------------
void identifyServo(uint8_t id) {
  Serial.print("[IDENTIFY] ID=");
  Serial.print(id);
  Serial.println(" - LED rojo 3 veces, torque OFF");
  
  // Asegurar torque OFF
  commandTorqueFree(id);
  delay(WAIT_SHORT);
  
  // Blink LED rojo 3 veces
  for (int i = 0; i < 3; i++) {
    herkulex_ledSet(id, LED_RED);
    delay(300);
    herkulex_ledOff(id);
    delay(300);
  }
  
  // Asegurar torque OFF al final
  commandTorqueFree(id);
  delay(WAIT_SHORT);
  
  Serial.println("[IDENTIFY] completado");
}

// -------------------- Move Servo --------------------
void moveServoToPosition(uint8_t id, uint16_t position) {
  Serial.print("[MOVE] ID=");
  Serial.print(id);
  Serial.print(" -> Posicion=");
  Serial.println(position);
  printDivider();
  
  // Verificar que el servo responde
  uint8_t st = 0, dt = 0;
  if (!herkulex_readStatus(id, st, dt)) {
    Serial.println("ERROR: Servo no responde");
    return;
  }
  
  // Leer posición actual
  uint16_t currentPos = readPosition_debug(id);
  if (currentPos != 0xFFFF) {
    Serial.print("Posicion actual: ");
    Serial.println(currentPos);
  } else {
    Serial.println("Posicion actual: ERROR (continuando igual)");
    currentPos = position;  // Usar posición objetivo como fallback
  }
  
  // Activar torque
  Serial.println("Activando torque...");
  commandEnableTorque(id);
  delay(100);
  
  // Limpiar buffer antes de mover (importante para evitar errores)
  while (Serial1.available()) (void)Serial1.read();
  delay(10);
  
  // Calcular playtime usando la función de la librería
  // Usar velocidad por defecto: ~250 cuentas/segundo
  float velocity = 250.0f;  // cuentas por segundo
  uint8_t playtime = herkulex_computePlaytime(currentPos, position, velocity);
  
  // Mover a posición objetivo
  Serial.print("Moviendo de ");
  Serial.print(currentPos);
  Serial.print(" a ");
  Serial.print(position);
  Serial.print(" (playtime: ");
  Serial.print(playtime);
  Serial.print(" = ");
  Serial.print(playtime * 11.2f);
  Serial.println(" ms)...");
  herkulex_moveTo(id, position, playtime, LED_OFF);
  
  // Esperar un poco para que el movimiento comience
  delay(500);
  
  // Leer posición después del movimiento (opcional, para verificar)
  Serial.println("Esperando movimiento...");
  delay(1000);
  
  uint16_t newPos = readPosition_debug(id);
  if (newPos != 0xFFFF) {
    Serial.print("Posicion despues del movimiento: ");
    Serial.println(newPos);
  }
  
  Serial.println("[MOVE] Completado.");
  printDivider();
}

// -------------------- Read Position --------------------
void readPositionInfo(uint8_t id) {
  Serial.print("[READ POSITION] ID=");
  Serial.println(id);
  printDivider();
  
  // Verificar que el servo responde
  uint8_t st = 0, dt = 0;
  if (!herkulex_readStatus(id, st, dt)) {
    Serial.println("ERROR: Servo no responde");
    return;
  }
  
  // Limpiar buffer antes de leer posición calibrada
  while (Serial1.available()) (void)Serial1.read();
  delay(10);
  
  // Posición calibrada (usar readPosition_debug que funciona mejor)
  uint16_t pos = readPosition_debug(id);
  if (pos != 0xFFFF) {
    Serial.print("Posicion (calibrada): ");
    Serial.println(pos);
  } else {
    Serial.println("Posicion (calibrada): ERROR");
  }
  
  // Limpiar buffer antes de leer posición absoluta
  while (Serial1.available()) (void)Serial1.read();
  delay(10);
  
  // Posición absoluta - leer manualmente con limpieza de buffer
  byte data[2] = {ADDR_POS_ABS_L, 0x02};  // ADDR_POS_ABS_L, 2 bytes
  byte packet[9];
  buildPacket(packet, id, CMD_RAM_READ, data, 2);
  Serial1.write(packet, packet[2]);
  delay(10);
  
  byte response[16];
  int len = readResponse(response, sizeof(response), 100);
  uint16_t absPos = 0xFFFF;
  if (len >= 12 && response[4] == ACK_RAM_READ) {
    absPos = response[9] | (response[10] << 8);
  }
  
  if (absPos != 0xFFFF) {
    Serial.print("Posicion (absoluta): ");
    Serial.println(absPos);
  } else {
    Serial.println("Posicion (absoluta): ERROR");
  }
  
  // Limpiar buffer antes de leer posición objetivo
  while (Serial1.available()) (void)Serial1.read();
  delay(10);
  
  // Posición objetivo - leer manualmente con limpieza de buffer
  byte data2[2] = {ADDR_POS_ABS_OBJ_L, 0x02};  // ADDR_POS_ABS_OBJ_L, 2 bytes
  buildPacket(packet, id, CMD_RAM_READ, data2, 2);
  Serial1.write(packet, packet[2]);
  delay(10);
  
  len = readResponse(response, sizeof(response), 100);
  uint16_t target = 0xFFFF;
  if (len >= 12 && response[4] == ACK_RAM_READ) {
    target = response[9] | (response[10] << 8);
  }
  
  if (target != 0xFFFF) {
    Serial.print("Posicion (objetivo): ");
    Serial.println(target);
  } else {
    Serial.println("Posicion (objetivo): ERROR");
  }
  
  printDivider();
}

// -------------------- Read block --------------------
bool readAllBasic(uint8_t id) {
  bool ok = true;

  // Status Error y Detail
  uint8_t st = 0, dt = 0;
  if (herkulex_readStatus(id, st, dt)) {
    Serial.print("statusError=0x");
    Serial.print(st, HEX);
    Serial.print(" detailError=0x");
    Serial.println(dt, HEX);
  } else {
    Serial.println("status=ERR");
    ok = false;
  }

  // Posición calibrada
  uint16_t pos = readPosition(id);
  if (pos != 0xFFFF) {
    Serial.print("pos(cal)=");
    Serial.println(pos);
  } else {
    Serial.println("pos(cal)=ERR");
    ok = false;
  }

  // Posición absoluta
  uint16_t absPos = 0;
  if (herkulex_readAbsolutePosition(id, absPos)) {
    Serial.print("pos(abs)=");
    Serial.println(absPos);
  } else {
    Serial.println("pos(abs)=ERR");
    ok = false;
  }

  // Posición objetivo
  uint16_t target = 0;
  if (herkulex_readTargetPosition(id, target)) {
    Serial.print("target=");
    Serial.println(target);
  } else {
    Serial.println("target=ERR");
    ok = false;
  }

  // Voltaje
  uint8_t volt = 0;
  if (herkulex_readVoltage(id, volt)) {
    Serial.print("volt(dV)=");
    Serial.print(volt);
    Serial.print(" => ");
    Serial.print(volt / 10.0);
    Serial.println("V");
  } else {
    Serial.println("volt=ERR");
    ok = false;
  }

  // Temperatura
  uint8_t temp = 0;
  if (herkulex_readTemperature(id, temp)) {
    Serial.print("temp=");
    Serial.print(temp);
    Serial.println("C");
  } else {
    Serial.println("temp=ERR");
    ok = false;
  }

  // PWM
  uint16_t pwm = 0;
  if (herkulex_readPWM(id, pwm)) {
    Serial.print("pwm=");
    Serial.println(pwm);
  } else {
    Serial.println("pwm=ERR");
    ok = false;
  }

  // Modo de control
  uint8_t mode = 0;
  if (herkulex_readCurrentMode(id, mode)) {
    Serial.print("mode=");
    Serial.print(mode);
    Serial.println(mode == 0 ? " (pos)" : " (vel)");
  } else {
    Serial.println("mode=ERR");
    ok = false;
  }

  // Velocidad
  uint16_t speed = 0;
  if (herkulex_readSpeed(id, speed)) {
    Serial.print("speed=");
    Serial.println(speed);
  } else {
    Serial.println("speed=ERR");
    ok = false;
  }

  // Información de torque/corriente
  uint16_t pwm2 = 0;
  float torque = 0, current = 0, perc = 0;
  if (herkulex_getTorqueInfo(id, pwm2, torque, current, perc, 0)) {
    Serial.print("torque=");
    Serial.print(torque);
    Serial.print("Nm  current=");
    Serial.print(current);
    Serial.print("mA  perc=");
    Serial.print(perc);
    Serial.println("%");
  } else {
    Serial.println("torqueInfo=ERR");
    ok = false;
  }

  return ok;
}

// -------------------- LED test --------------------
void ledTest(uint8_t id) {
  Serial.println("[LED] ciclo colores");
  herkulex_ledSet(id, LED_RED);
  delay(200);
  herkulex_ledSet(id, LED_GREEN);
  delay(200);
  herkulex_ledSet(id, LED_BLUE);
  delay(200);
  herkulex_ledSet(id, LED_YELLOW);
  delay(200);
  herkulex_ledSet(id, LED_CYAN);
  delay(200);
  herkulex_ledSet(id, LED_PINK);
  delay(200);
  herkulex_ledSet(id, LED_WHITE);
  delay(200);
  herkulex_ledOff(id);
  delay(200);
  Serial.println("[LED] test completado");
}

// -------------------- Movement test --------------------
bool movementTest(uint8_t id) {
  Serial.println("[MOVE] torque ON + mover delta + volver");

  commandEnableTorque(id);
  delay(WAIT_SHORT);

  uint16_t p0 = readPosition(id);
  if (p0 == 0xFFFF) {
    Serial.println("readPosition ERR");
    commandTorqueFree(id);
    return false;
  }

  Serial.print("Posición inicial: ");
  Serial.println(p0);

  // Calcular posición objetivo: intentar p0+MOVE_DELTA primero
  int p1 = (int)p0 + MOVE_DELTA;
  bool validPos = false;
  
  // Validar rango seguro (usando constantes POS_MIN_DEF y POS_MAX_DEF)
  if (p1 >= (int)POS_MIN_DEF && p1 <= (int)POS_MAX_DEF) {
    validPos = true;
  } else {
    // Si p0+MOVE_DELTA está fuera de rango, intentar p0-MOVE_DELTA
    p1 = (int)p0 - MOVE_DELTA;
    if (p1 >= (int)POS_MIN_DEF && p1 <= (int)POS_MAX_DEF) {
      validPos = true;
    }
  }
  
  if (!validPos) {
    Serial.print("⚠️ No se puede mover: p0=");
    Serial.print(p0);
    Serial.print(" ±");
    Serial.print(MOVE_DELTA);
    Serial.println(" está fuera de rango seguro");
    commandTorqueFree(id);
    return false;
  }
  
  Serial.print("Moviendo a posición: ");
  Serial.println(p1);

  herkulex_safeMoveTo(id, (uint16_t)p1, SAFE_VEL);
  delay(WAIT_MOVE);

  uint16_t pA = readPosition(id);
  
  // Volver a posición inicial
  Serial.println("Volviendo a posición inicial...");
  herkulex_safeMoveTo(id, p0, SAFE_VEL);
  delay(WAIT_MOVE);

  uint16_t pB = readPosition(id);
  
  // Línea resumen de movimiento
  Serial.print("[MOVE] resumen: p0=");
  Serial.print(p0);
  Serial.print(" -> p1=");
  Serial.print(p1);
  Serial.print(" -> pA=");
  Serial.print(pA);
  Serial.print(" -> pB=");
  Serial.print(pB);
  Serial.print(" | resultado=");
  Serial.println((abs((int)pB - (int)p0) < 10) ? "OK" : "WARN");

  commandTorqueFree(id);
  delay(WAIT_SHORT);

  Serial.println("[MOVE] test completado");
  return true;
}

// -------------------- Error handling --------------------
void clearErrors(uint8_t id) {
  Serial.println("[ERR] clear");
  commandClearError(id);
  delay(WAIT_SHORT);
  
  // Re-leer status después de limpiar
  uint8_t st = 0, dt = 0;
  if (herkulex_readStatus(id, st, dt)) {
    Serial.println("Status después de clear:");
    printHerkulexErrors(st, dt);
  }
}

// -------------------- Helper: verificar si ID está en uso --------------------
bool isIdInUse(uint8_t id, uint8_t* foundIds, int foundCount) {
  for (int i = 0; i < foundCount; i++) {
    if (foundIds[i] == id) {
      return true;
    }
  }
  return false;
}

// -------------------- Helper: imprimir lista de IDs detectados --------------------
void printIdList(uint8_t* foundIds, int foundCount, const char* label) {
  Serial.print(label);
  if (foundCount == 0) {
    Serial.println("(ninguno)");
    return;
  }
  Serial.print("(");
  Serial.print(foundCount);
  Serial.print("): ");
  for (int i = 0; i < foundCount; i++) {
    Serial.print(foundIds[i]);
    if (i < foundCount - 1) Serial.print(", ");
  }
  Serial.println();
}

// -------------------- Mini-scan rápido (solo verifica IDs específicos) --------------------
int quickScanForId(uint8_t targetId, uint8_t* foundIds, int foundCount) {
  // Verificar si el targetId ya está en la lista
  if (isIdInUse(targetId, foundIds, foundCount)) {
    return foundCount;  // Ya está en la lista, no necesita scan
  }
  
  // Hacer ping rápido solo al targetId
  if (pingByStatus(targetId)) {
    // Agregar a la lista si hay espacio
    if (foundCount < 32) {
      foundIds[foundCount] = targetId;
      return foundCount + 1;
    }
  }
  return foundCount;
}

// -------------------- Función común de cambio de ID --------------------
// Retorna: true = éxito, false = fallo/indeterminado
bool performIdChange(uint8_t oldId, uint8_t newId, uint8_t* foundIds, int foundCount) {
  Serial.print("Cambiando ID ");
  Serial.print(oldId);
  Serial.print(" -> ");
  Serial.println(newId);

  // 1) Ejecutar commandSetID
  commandSetID(oldId, newId);
  delay(50);
  
  // 2) Enviar reboot al oldId (paso principal)
  Serial.println("Enviando reboot al ID antiguo...");
  herkulex_reboot(oldId);
  delay(1000);  // Tiempo suficiente para reboot
  
  // 3) Verificación post-reboot
  Serial.println("Verificando cambio de ID...");
  delay(500);  // Dar tiempo adicional para que el servo se reinicie
  
  // Verificar primero ping(newId) - esto es lo que confirma éxito
  bool newIdResponds = pingByStatus(newId);
  
  if (newIdResponds) {
    // ÉXITO: newId responde
    Serial.print("✓ ÉXITO: El nuevo ID ");
    Serial.print(newId);
    Serial.println(" responde correctamente.");
    Serial.print("ID anterior: ");
    Serial.print(oldId);
    Serial.print(" | ID nuevo: ");
    Serial.print(newId);
    Serial.println(" | Resultado: CAMBIO EXITOSO");
    return true;
  }
  
  // Si newId no responde, verificar oldId
  bool oldIdResponds = pingByStatus(oldId);
  
  if (oldIdResponds) {
    // El cambio no se aplicó - oldId todavía responde
    Serial.print("⚠️ El cambio NO se aplicó: el ID antiguo (");
    Serial.print(oldId);
    Serial.println(") todavía responde.");
    Serial.print("ID anterior: ");
    Serial.print(oldId);
    Serial.print(" | ID nuevo: ");
    Serial.print(newId);
    Serial.println(" | Resultado: CAMBIO NO APLICADO");
    return false;
  }
  
  // Ninguno responde - estado indeterminado, intentar fallback
  Serial.println("⚠️ Estado indeterminado: ninguno de los IDs responde.");
  Serial.println("Intentando reboot fallback al ID nuevo...");
  herkulex_reboot(newId);
  delay(1000);
  
  // Verificar nuevamente ping(newId)
  newIdResponds = pingByStatus(newId);
  delay(300);
  
  if (newIdResponds) {
    Serial.print("✓ ÉXITO (fallback): El nuevo ID ");
    Serial.print(newId);
    Serial.println(" responde después del reboot fallback.");
    Serial.print("ID anterior: ");
    Serial.print(oldId);
    Serial.print(" | ID nuevo: ");
    Serial.print(newId);
    Serial.println(" | Resultado: CAMBIO EXITOSO (fallback)");
    return true;
  }
  
  // Aún no responde - hacer SCAN para verificar
  Serial.println("Realizando SCAN para verificar estado...");
  delay(500);
  
  uint8_t scanFound[32];
  int scanCount = scanIds(scanFound, 32);
  
  bool newIdInScan = isIdInUse(newId, scanFound, scanCount);
  bool oldIdInScan = isIdInUse(oldId, scanFound, scanCount);
  
  Serial.println("\n--- Resultado del cambio ---");
  Serial.print("ID anterior: ");
  Serial.print(oldId);
  Serial.print(" | ID nuevo: ");
  Serial.print(newId);
  Serial.print(" | Resultado: ");
  
  if (newIdInScan) {
    Serial.println("CAMBIO EXITOSO (verificado por SCAN)");
    return true;
  } else if (oldIdInScan) {
    Serial.println("CAMBIO NO APLICADO (oldId encontrado en SCAN)");
    return false;
  } else {
    Serial.println("INDETERMINADO (ningún ID encontrado en SCAN)");
    return false;
  }
}

// -------------------- Set ID (safe) --------------------
void setIdFlowSafe(uint8_t oldId, uint8_t* foundIds, int foundCount) {
  // Impedir cambio si hay más de 1 servo encontrado
  if (foundCount > 1) {
    Serial.print("⚠️ ERROR: Se encontraron ");
    Serial.print(foundCount);
    Serial.println(" servos en el último SCAN.");
    Serial.println("Cambiar ID con múltiples servos conectados es peligroso.");
    Serial.println("Usá 'Set ID (advanced)' o desconectá los demás servos.");
    // Cleanup garantizado
    commandTorqueFree(oldId);
    herkulex_ledOff(oldId);
    return;
  }

  int newId = readInt("Nuevo ID (0-253): ");
  if (newId < 0 || newId > 253) {
    Serial.println("ID invalido.");
    // Cleanup garantizado
    commandTorqueFree(oldId);
    herkulex_ledOff(oldId);
    return;
  }

  // VALIDACIÓN PREVIA
  // 1) Verificar si newId == oldId
  if ((uint8_t)newId == oldId) {
    Serial.println("⚠️ El nuevo ID es igual al ID actual. Operación cancelada.");
    // Cleanup garantizado
    commandTorqueFree(oldId);
    herkulex_ledOff(oldId);
    return;
  }

  // 2) SCAN automático para actualizar lista de IDs
  Serial.println("\n[Validación] Escaneando red para verificar IDs en uso...");
  uint8_t currentIds[32];
  int currentCount = scanIds(currentIds, 32);
  printIdList(currentIds, currentCount, "IDs detectados ANTES del cambio: ");

  // 3) Verificar si newId está en uso (usando lista actualizada)
  if (isIdInUse((uint8_t)newId, currentIds, currentCount)) {
    Serial.print("⚠️ ERROR: El ID ");
    Serial.print(newId);
    Serial.println(" ya está en uso por otro servo conectado.");
    Serial.println("No se puede crear un ID duplicado. Operación cancelada.");
    // Cleanup garantizado
    commandTorqueFree(oldId);
    herkulex_ledOff(oldId);
    return;
  }

  // Hacer IDENTIFY del ID actual antes de cambiar
  Serial.println("Identificando servo actual (LED rojo 3 veces)...");
  identifyServo(oldId);
  delay(500);

  Serial.println("⚠️ Esto escribe EEPROM y requiere reboot. Si falla, vas a tener que re-scanear.");
  if (!readYesNo("Confirmar cambio de ID")) {
    // Cleanup garantizado
    commandTorqueFree(oldId);
    herkulex_ledOff(oldId);
    return;
  }

  // Realizar cambio de ID
  performIdChange(oldId, (uint8_t)newId, currentIds, currentCount);
  
  // SCAN después del cambio para mostrar lista actualizada
  Serial.println("\n[Verificación] Escaneando red después del cambio...");
  delay(500);
  uint8_t afterIds[32];
  int afterCount = scanIds(afterIds, 32);
  printIdList(afterIds, afterCount, "IDs detectados DESPUÉS del cambio: ");
  
  // Cleanup garantizado al final
  commandTorqueFree(oldId);
  herkulex_ledOff(oldId);
  commandTorqueFree((uint8_t)newId);
  herkulex_ledOff((uint8_t)newId);
  delay(WAIT_SHORT);
}

// -------------------- Set ID (advanced) --------------------
void setIdFlowAdvanced(uint8_t oldId, uint8_t* foundIds, int foundCount) {
  Serial.println("⚠️ MODO AVANZADO: Permite cambiar ID aunque haya múltiples servos conectados.");
  Serial.println("⚠️ Asegurate de identificar correctamente el servo antes de continuar.");

  int newId = readInt("Nuevo ID (0-253): ");
  if (newId < 0 || newId > 253) {
    Serial.println("ID invalido.");
    // Cleanup garantizado
    commandTorqueFree(oldId);
    herkulex_ledOff(oldId);
    return;
  }

  // VALIDACIÓN PREVIA
  // 1) Verificar si newId == oldId
  if ((uint8_t)newId == oldId) {
    Serial.println("⚠️ El nuevo ID es igual al ID actual. Operación cancelada.");
    // Cleanup garantizado
    commandTorqueFree(oldId);
    herkulex_ledOff(oldId);
    return;
  }

  // 2) SCAN automático para actualizar lista de IDs
  Serial.println("\n[Validación] Escaneando red para verificar IDs en uso...");
  uint8_t currentIds[32];
  int currentCount = scanIds(currentIds, 32);
  printIdList(currentIds, currentCount, "IDs detectados ANTES del cambio: ");

  // 3) Verificar si newId está en uso (usando lista actualizada)
  if (isIdInUse((uint8_t)newId, currentIds, currentCount)) {
    Serial.print("⚠️ ERROR: El ID ");
    Serial.print(newId);
    Serial.println(" ya está en uso por otro servo conectado.");
    Serial.println("No se puede crear un ID duplicado. Operación cancelada.");
    // Cleanup garantizado
    commandTorqueFree(oldId);
    herkulex_ledOff(oldId);
    return;
  }

  // 1) Ejecutar identifyServo obligatoriamente
  Serial.println("\n[1/4] Identificando servo (LED rojo 3 veces)...");
  identifyServo(oldId);
  delay(500);

  // 2) Pedir confirmación extra: escribir exactamente "CHANGE"
  Serial.println("\n[2/4] Confirmación de seguridad requerida.");
  Serial.println("⚠️ Esto escribe EEPROM y requiere reboot.");
  Serial.print("Escribí exactamente 'CHANGE' para continuar: ");
  String confirm = readLine();
  confirm.trim();
  
  if (confirm != "CHANGE") {
    Serial.print("Confirmación incorrecta ('");
    Serial.print(confirm);
    Serial.println("'). Operación cancelada.");
    // Cleanup garantizado
    commandTorqueFree(oldId);
    herkulex_ledOff(oldId);
    return;
  }

  Serial.println("✓ Confirmación recibida.");

  // 3) Realizar cambio de ID
  Serial.println("\n[3/4] Cambiando ID...");
  performIdChange(oldId, (uint8_t)newId, currentIds, currentCount);

  // 4) SCAN automático y verificación final
  Serial.println("\n[4/4] Verificación final con SCAN...");
  delay(500);
  
  uint8_t afterIds[32];
  int afterCount = scanIds(afterIds, 32);
  printIdList(afterIds, afterCount, "IDs detectados DESPUÉS del cambio: ");
  
  bool newIdInScan = isIdInUse((uint8_t)newId, afterIds, afterCount);
  bool oldIdInScan = isIdInUse(oldId, afterIds, afterCount);
  
  Serial.println("\n--- Resultado final del cambio ---");
  Serial.print("ID anterior: ");
  Serial.print(oldId);
  Serial.print(" | ID nuevo: ");
  Serial.print(newId);
  Serial.print(" | ");
  
  if (newIdInScan) {
    Serial.println("✓ ÉXITO: Nuevo ID encontrado en SCAN");
  } else if (oldIdInScan) {
    Serial.println("✗ FALLO: ID antiguo todavía presente (cambio no aplicado)");
  } else {
    Serial.println("⚠️ INDETERMINADO: Ningún ID encontrado en SCAN");
  }
  Serial.print("Total de servos encontrados: ");
  Serial.println(afterCount);
  
  // Cleanup garantizado al final (éxito o fallo)
  commandTorqueFree(oldId);
  herkulex_ledOff(oldId);
  commandTorqueFree((uint8_t)newId);
  herkulex_ledOff((uint8_t)newId);
  delay(WAIT_SHORT);
}

// -------------------- Scan --------------------
// Función base de SCAN con rango personalizable
int scanIds(uint8_t* out, int maxOut, int minId, int maxId) {
  Serial.print("[SCAN] buscando servos por readStatus (IDs ");
  Serial.print(minId);
  Serial.print("-");
  Serial.print(maxId);
  Serial.println(")...");
  
  int found = 0;
  unsigned long startTime = millis();
  int totalIds = maxId - minId + 1;
  
  for (int id = minId; id <= maxId; id++) {
    if (pingByStatus((uint8_t)id)) {
      if (found < maxOut) out[found] = (uint8_t)id;
      found++;
      Serial.print("  encontrado ID=");
      Serial.println(id);
    }
    delay(2);  // Delay reducido para acelerar
    
    // Mostrar progreso cada 50 IDs (solo si hay muchos)
    if (totalIds > 50 && ((id - minId) % 50) == 0 && (id - minId) > 0) {
      Serial.print("[SCAN] Progreso: ");
      Serial.print(id - minId + 1);
      Serial.print("/");
      Serial.println(totalIds);
    }
  }
  
  unsigned long elapsed = millis() - startTime;
  Serial.println();
  Serial.print("[SCAN] total encontrados: ");
  Serial.println(found);
  Serial.print("[SCAN] tiempo: ");
  Serial.print(elapsed);
  Serial.println(" ms");
  Serial.println("[SCAN] completado.");
  Serial.println();
  return found;
}

// SCAN rápido: solo IDs 1-20 (común en brazos robóticos)
int scanIdsQuick(uint8_t* out, int maxOut) {
  Serial.println("[SCAN RAPIDO] Escaneando IDs 1-20...");
  return scanIds(out, maxOut, 1, 20);
}

// SCAN completo: todos los IDs 0-253
int scanIdsDeep(uint8_t* out, int maxOut) {
  Serial.println("[SCAN COMPLETO] Escaneando todos los IDs 0-253 (puede tardar varios minutos)...");
  return scanIds(out, maxOut, ID_MIN, ID_MAX);
}

// -------------------- Print summary line --------------------
void printSummaryLine(uint8_t id) {
  uint8_t st = 0, dt = 0;
  uint8_t volt = 0, temp = 0, mode = 0;
  uint16_t pwm = 0, pos = 0, speed = 0;
  
  bool ok = true;
  ok &= herkulex_readStatus(id, st, dt);
  ok &= herkulex_readVoltage(id, volt);
  ok &= herkulex_readTemperature(id, temp);
  ok &= herkulex_readPWM(id, pwm);
  pos = readPosition(id);
  ok &= (pos != 0xFFFF);
  ok &= herkulex_readCurrentMode(id, mode);
  ok &= herkulex_readSpeed(id, speed);
  
  Serial.print("[RESUMEN] ID=");
  Serial.print(id);
  Serial.print(" | status=0x");
  Serial.print(st, HEX);
  Serial.print("/0x");
  Serial.print(dt, HEX);
  Serial.print(" | volt=");
  Serial.print(volt / 10.0);
  Serial.print("V | temp=");
  Serial.print(temp);
  Serial.print("C | pwm=");
  Serial.print(pwm);
  Serial.print(" | pos=");
  Serial.print(pos);
  Serial.print(" | mode=");
  Serial.print(mode);
  Serial.print(" | speed=");
  Serial.print(speed);
  Serial.print(" | ");
  Serial.println(ok ? "OK" : "ERR");
}

// -------------------- Self-check main --------------------
void runSelfCheck(uint8_t id) {
  printDivider();
  Serial.print("SELF CHECK ID=");
  Serial.println(id);

  if (!pingByStatus(id)) {
    Serial.println("FAIL: no responde (ping/status).");
    return;
  }

  // Línea resumen inicial
  Serial.println("[0] Resumen inicial");
  printSummaryLine(id);

  Serial.println("[1] Lecturas basicas");
  bool ok1 = readAllBasic(id);

  Serial.println("[2] LED test");
  ledTest(id);

  Serial.println("[3] Clear errors + re-leer status");
  clearErrors(id);

  Serial.println("[4] Movimiento");
  bool okMove = movementTest(id);

  Serial.println("[5] Lecturas post-movimiento");
  bool ok2 = readAllBasic(id);

  // Línea resumen final
  Serial.println("[6] Resumen final");
  printSummaryLine(id);

  Serial.println("\nRESULTADO:");
  Serial.print("- Lecturas pre: ");
  Serial.println(ok1 ? "OK" : "WARN/FAIL");
  Serial.print("- Movimiento:   ");
  Serial.println(okMove ? "OK" : "FAIL");
  Serial.print("- Lecturas post:");
  Serial.println(ok2 ? "OK" : "WARN/FAIL");
  printDivider();
}

// -------------------- Setup / Menu --------------------
void printMenu() {
  Serial.println("\nMenu:");
  Serial.println("1) SCAN IDs RAPIDO (1-20)");
  Serial.println("2) SCAN IDs COMPLETO (0-253)");
  Serial.println("3) SELF CHECK (elegir ID)");
  Serial.println("4) IDENTIFY (LED blink + torque OFF)");
  Serial.println("5) Set ID (safe) - solo si hay 1 servo");
  Serial.println("6) Set ID (advanced) - permite múltiples servos");
  Serial.println("7) Reboot servo (elegir ID)");
  Serial.println("8) Read Position (elegir ID)");
  Serial.println("9) Move Servo (elegir ID y posicion 10627-22129)");
  Serial.println("10) Clear Errors (elegir ID)");
  Serial.println("0) Exit (no hace nada)");
}

void setup() {
  Serial.begin(PC_BAUD);
  while(!Serial) {}
  Serial.println("\n=== HerkuleX DRS-0602 - Scanner + SelfCheck ===");

  // Inicializar bus del servo (Serial1)
  Serial1.begin(SERVO_BAUD);
  Serial.print("Serial1 inicializado a ");
  Serial.print(SERVO_BAUD);
  Serial.println(" baudios");
  delay(100);  // Pequeño delay para estabilización

  uint8_t found[32];
  int lastFound = 0;

  while (true) {
    printMenu();
    int op = readInt("Opcion: ");

    if (op == 1) {
      lastFound = scanIdsQuick(found, 32);
      Serial.println();  // Línea en blanco después del SCAN
      delay(100);  // Pequeño delay para que se vea el resultado
    }
    else if (op == 2) {
      lastFound = scanIdsDeep(found, 32);
      Serial.println();  // Línea en blanco después del SCAN
      delay(100);  // Pequeño delay para que se vea el resultado
    }
    else if (op == 3) {
      if (lastFound <= 0) Serial.println("Tip: primero hace SCAN (op 1 o 2).");
      int id = readInt("ID a testear: ");
      if (id < 0 || id > 253) {
        Serial.println("ID invalido.");
        continue;
      }
      runSelfCheck((uint8_t)id);
    }
    else if (op == 4) {
      int id = readInt("ID a identificar: ");
      if (id < 0 || id > 253) {
        Serial.println("ID invalido.");
        continue;
      }
      identifyServo((uint8_t)id);
    }
    else if (op == 5) {
      if (lastFound <= 0) {
        Serial.println("Tip: primero hace SCAN (op 1 o 2) para detectar servos.");
        continue;
      }
      int id = readInt("ID actual (old): ");
      if (id < 0 || id > 253) {
        Serial.println("ID invalido.");
        continue;
      }
      setIdFlowSafe((uint8_t)id, found, lastFound);
    }
    else if (op == 6) {
      if (lastFound <= 0) {
        Serial.println("Tip: primero hace SCAN (op 1 o 2) para detectar servos.");
        continue;
      }
      int id = readInt("ID actual (old): ");
      if (id < 0 || id > 253) {
        Serial.println("ID invalido.");
        continue;
      }
      setIdFlowAdvanced((uint8_t)id, found, lastFound);
    }
    else if (op == 7) {
      int id = readInt("ID a reboot: ");
      if (id < 0 || id > 253) {
        Serial.println("ID invalido.");
        continue;
      }
      Serial.print("Reiniciando servo ID ");
      Serial.println(id);
      herkulex_reboot((uint8_t)id);
      delay(500);
      Serial.println("Reboot enviado.");
    }
    else if (op == 8) {
      int id = readInt("ID a leer posicion: ");
      if (id < 0 || id > 253) {
        Serial.println("ID invalido.");
        continue;
      }
      readPositionInfo((uint8_t)id);
    }
    else if (op == 9) {
      int id = readInt("ID del servo a mover: ");
      if (id < 0 || id > 253) {
        Serial.println("ID invalido.");
        continue;
      }
      int pos = readInt("Posicion (10627-22129): ");
      if (pos < 10627 || pos > 22129) {
        Serial.println("Posicion fuera de rango valido (10627-22129).");
        continue;
      }
      moveServoToPosition((uint8_t)id, (uint16_t)pos);
    }
    else if (op == 10) {
      int id = readInt("ID del servo a limpiar errores: ");
      if (id < 0 || id > 253) {
        Serial.println("ID invalido.");
        continue;
      }
      printDivider();
      Serial.print("[CLEAR ERRORS] ID=");
      Serial.println(id);
      clearErrors((uint8_t)id);
      printDivider();
    }
    else if (op == 0) {
      Serial.println("OK.");
      break;
    }
    else {
      Serial.println("Opcion invalida.");
    }
  }
}

void loop() {}

