#include <Arduino.h>

// Configuración de debug de paquetes (0 = desactivado, 1 = activado)
// Cambiar a 1 para ver todos los paquetes enviados y recibidos
#define HERKULEX_DEBUG_PACKETS 0

#include "brazo_config.h"
#include "poses.h"
#include "brazo.h"
#include "pose_manager.h"
#include "esp_bridge.h"
#include "mega_feedback.h"

// Instancias globales
Brazo brazo;
PoseManager poseManager;
EspBridge espBridge;

// Forward declarations
void processSingleCharCommand(char cmd);
void printMenu();
void handleEspCommand(uint8_t command);

// Helper para leer una línea desde Serial. Solo retorna cuando el usuario
// pulsa Enter (\r o \n). Si pasan 5 s sin Enter, retorna "" para no ejecutar
// comandos a medias (evita que se disparen solos).
String readLine() {
  String line = "";
  unsigned long timeout = millis() + 5000;
  
  while (millis() < timeout) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (line.length() > 0) {
          return line;
        }
      } else {
        line += c;
      }
    }
    delay(1);
  }
  // Timeout: no retornar lo escrito a medias, solo vacío
  return "";
}

// Helper para esperar Enter del usuario (sin timeout)
void waitForEnter() {
  // Limpiar buffer primero
  while (Serial.available()) {
    Serial.read();
  }
  
  // Esperar indefinidamente hasta recibir Enter
  while (true) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        // Limpiar cualquier carácter restante
        while (Serial.available()) {
          Serial.read();
        }
        return;
      }
    }
    delay(10);
  }
}

// Helper para parsear comando con parámetros
void parseCommand(const String& line) {
  String s = line;
  s.trim();
  if (s.length() == 0) return;
  
  char cmd = s.charAt(0);
  String args = s.substring(1);
  args.trim();
  
  switch (cmd) {
    case 'l':
    case 'L':
      poseManager.listUserPoses(Serial);
      break;
      
    case 'g':
    case 'G': {
      // Formato: g <n> [name]
      int spaceIdx = args.indexOf(' ');
      if (spaceIdx < 0) {
        Serial.println(F("Error: Formato: g <slot> [nombre]"));
        break;
      }
      
      int slot = args.substring(0, spaceIdx).toInt();
      String name = args.substring(spaceIdx + 1);
      name.trim();
      
      if (slot < 1 || slot > MAX_POSES) {
        Serial.print(F("Error: Slot debe estar entre 1 y "));
        Serial.println(MAX_POSES);
        break;
      }
      
      // Paso 1: Activar torque y mover a POSE_STANDBY
      Serial.println(F("-> Activando torque y moviendo a POSE_STANDBY..."));
      brazo.torqueAllOn();
      delay(100);
      brazo.goPose(POSE_STANDBY, 2000);
      delay(2500);  // Esperar a que termine el movimiento
      
      // Paso 2: Apagar torque (modo teach-in)
      Serial.println(F("-> Torque OFF - Modo teach-in activo"));
      Serial.println(F("-> Mueve el brazo manualmente a la posición deseada"));
      Serial.println(F("-> Presiona Enter cuando estés listo para grabar..."));
      brazo.torqueAllOff();
      delay(200);  // Esperar a que se desactive el torque completamente
      
      // Paso 3: Esperar Enter del usuario (sin timeout)
      waitForEnter();
      
      // Paso 4: Leer posiciones actuales (con torque OFF, después de que el usuario movió el brazo)
      Serial.println();
      Serial.println(F("-> Leyendo posiciones actuales..."));
      uint16_t currentPos[NUM_MOTORES];
      bool allOk = true;
      for (uint8_t i = 0; i < NUM_MOTORES; i++) {
        currentPos[i] = brazo.motors[i].getPosition();
        if (currentPos[i] == 0xFFFF) {
          Serial.print(F("Error: No se pudo leer posición del motor ID "));
          Serial.println(brazo.motors[i].id);
          allOk = false;
          break;
        }
      }
      
      if (!allOk) {
        Serial.println(F("Error: No se pudo capturar la pose"));
        // Dejar torque OFF (modo teach-in)
        brazo.torqueAllOff();
        break;
      }
      
      // Paso 5: Guardar pose
      const char* namePtr = (name.length() > 0) ? name.c_str() : nullptr;
      if (poseManager.capturePoseSlot(slot, namePtr, currentPos)) {
        Serial.print(F("✓ Pose guardada en slot "));
        Serial.print(slot);
        Serial.print(F(": "));
        Serial.println(poseManager.getPoseName(slot));
        Serial.println(F("-> Torque ON - brazo sujeto"));
        brazo.torqueAllOn();
      } else {
        Serial.println(F("Error: No se pudo guardar la pose"));
        brazo.torqueAllOff();
      }
      break;
    }
    
    case 'a':
    case 'A': {
      // Formato: a [name]
      String name = args;
      name.trim();
      
      // Paso 1: Activar torque y mover a POSE_STANDBY
      Serial.println(F("-> Activando torque y moviendo a POSE_STANDBY..."));
      brazo.torqueAllOn();
      delay(100);
      brazo.goPose(POSE_STANDBY, 2000);
      delay(2500);  // Esperar a que termine el movimiento
      
      // Paso 2: Apagar torque (modo teach-in)
      Serial.println(F("-> Torque OFF - Modo teach-in activo"));
      Serial.println(F("-> Mueve el brazo manualmente a la posición deseada"));
      Serial.println(F("-> Presiona Enter cuando estés listo para grabar..."));
      brazo.torqueAllOff();
      delay(200);  // Esperar a que se desactive el torque completamente
      
      // Paso 3: Esperar Enter del usuario (sin timeout)
      waitForEnter();
      
      // Paso 4: Leer posiciones actuales (con torque OFF, después de que el usuario movió el brazo)
      Serial.println();
      Serial.println(F("-> Leyendo posiciones actuales..."));
      uint16_t currentPos[NUM_MOTORES];
      bool allOk = true;
      for (uint8_t i = 0; i < NUM_MOTORES; i++) {
        currentPos[i] = brazo.motors[i].getPosition();
        if (currentPos[i] == 0xFFFF) {
          Serial.print(F("Error: No se pudo leer posición del motor ID "));
          Serial.println(brazo.motors[i].id);
          allOk = false;
          break;
        }
      }
      
      if (!allOk) {
        Serial.println(F("Error: No se pudo capturar la pose"));
        // Dejar torque OFF (modo teach-in)
        brazo.torqueAllOff();
        break;
      }
      
      // Paso 5: Guardar pose
      const char* namePtr = (name.length() > 0) ? name.c_str() : nullptr;
      uint8_t slot = poseManager.capturePoseAuto(namePtr, currentPos);
      if (slot > 0) {
        Serial.print(F("✓ Pose guardada en slot "));
        Serial.print(slot);
        Serial.print(F(": "));
        Serial.println(poseManager.getPoseName(slot));
        Serial.println(F("-> Torque ON - brazo sujeto"));
        brazo.torqueAllOn();
      } else {
        Serial.println(F("Error: No se pudo guardar la pose"));
        brazo.torqueAllOff();
      }
      break;
    }
    
    case 'p': {
      // Formato: p <slot>  (tiempo fijo 2500 ms para no ser brusco)
      const uint16_t POSE_DEFAULT_MS = 2500;  // Límite protocolo ~2845 ms (254*11.2)
      args.trim();
      if (args.length() == 0) {
        Serial.println(F("Error: Formato: p <slot> (ej. p 1)"));
        break;
      }
      int spaceIdx = args.indexOf(' ');
      int slot, ms = (int)POSE_DEFAULT_MS;
      if (spaceIdx >= 0) {
        slot = args.substring(0, spaceIdx).toInt();
        int msArg = args.substring(spaceIdx + 1).toInt();
        if (msArg > 0 && msArg <= 2845) ms = msArg;
      } else {
        slot = args.toInt();
      }
      if (slot < 1 || slot > MAX_POSES) {
        Serial.print(F("Error: Slot debe estar entre 1 y "));
        Serial.println(MAX_POSES);
        break;
      }
      uint16_t posiciones[NUM_MOTORES];
      if (poseManager.playPoseSlot(slot, posiciones)) {
        Serial.print(F("-> Activando torque de todos los motores..."));
        brazo.torqueAllOn();
        delay(100);
        Serial.print(F("-> Moviendo a pose slot "));
        Serial.print(slot);
        Serial.print(F(" ("));
        Serial.print(poseManager.getPoseName(slot));
        Serial.print(F(") en "));
        Serial.print(ms);
        Serial.println(F(" ms..."));
        brazo.goRaw(posiciones, (uint16_t)ms);
        Serial.println(F("✓ Completado"));
      } else {
        Serial.print(F("Error: Slot "));
        Serial.print(slot);
        Serial.println(F(" no tiene pose guardada"));
      }
      break;
    }
    
    case 'd':
    case 'D': {
      // Formato: d <n>
      int slot = args.toInt();
      if (slot < 1 || slot > MAX_POSES) {
        Serial.print(F("Error: Slot debe estar entre 1 y "));
        Serial.println(MAX_POSES);
        break;
      }
      
      if (poseManager.deletePoseSlot(slot)) {
        Serial.print(F("✓ Pose eliminada del slot "));
        Serial.println(slot);
      } else {
        Serial.print(F("Error: Slot "));
        Serial.print(slot);
        Serial.println(F(" no tiene pose guardada"));
      }
      break;
    }
    
    case 'r':
    case 'R': {
      // Formato: r  o  r <ms>  (default 2500 ms por pose)
      const uint16_t RECORRIDO_DEFAULT_MS = 2500;
      int ms = args.toInt();
      if (ms <= 0 || ms > 2845) ms = (int)RECORRIDO_DEFAULT_MS;
      
      uint8_t usedSlots[MAX_POSES];
      uint8_t count = poseManager.getUsedSlots(usedSlots, MAX_POSES);
      
      if (count == 0) {
        Serial.println(F("No hay poses guardadas para recorrer"));
        break;
      }
      
      Serial.print(F("-> Activando torque de todos los motores..."));
      brazo.torqueAllOn();
      delay(100);  // Esperar a que se active el torque
      
      Serial.print(F("-> Recorriendo "));
      Serial.print(count);
      Serial.print(F(" poses con "));
      Serial.print(ms);
      Serial.println(F(" ms cada una..."));
      
      for (uint8_t i = 0; i < count; i++) {
        uint16_t posiciones[NUM_MOTORES];
        if (poseManager.playPoseSlot(usedSlots[i], posiciones)) {
          Serial.print(F("  ["));
          Serial.print(i + 1);
          Serial.print(F("/"));
          Serial.print(count);
          Serial.print(F("] Slot "));
          Serial.print(usedSlots[i]);
          Serial.print(F(": "));
          Serial.print(poseManager.getPoseName(usedSlots[i]));
          Serial.println();
          brazo.goRaw(posiciones, ms);
          delay(ms + 50);  // Esperar movimiento + margen
        }
      }
      Serial.println(F("✓ Recorrido completado"));
      break;
    }
    
    default:
      // Comandos de un solo carácter (compatibilidad)
      processSingleCharCommand(cmd);
      break;
  }
}

// Procesar comandos de un solo carácter (mantener compatibilidad)
void processSingleCharCommand(char cmd) {
  switch (cmd) {
    case 's':
    case 'S':
      Serial.println(F("-> Iniciando secuencia START..."));
      Serial.println();
      if (brazo.checkAll()) {
        Serial.println(F("-> Moviendo a POSE_INICIAL..."));
        delay(500);
        brazo.goPose(POSE_INICIAL, 1500);
        Serial.println(F("✓ Brazo en posición inicial"));
      } else {
        Serial.println(F("⚠️ Check falló. Revisar motores antes de continuar."));
      }
      break;

    case '1':
      Serial.println(F("-> Moviendo a POSE_INICIAL..."));
      if (brazo.goPose(POSE_INICIAL, 1500)) {
        Serial.println(F("✓ Completado"));
      } else {
        Serial.println(F("✗ Abortado por sobrecarga"));
      }
      break;

    case '2':
      Serial.println(F("-> Moviendo a POSE_TRABAJO..."));
      if (brazo.goPose(POSE_TRABAJO, 1500)) {
        Serial.println(F("✓ Completado"));
      } else {
        Serial.println(F("✗ Abortado por sobrecarga"));
      }
      break;

    case '3':
      Serial.println(F("-> Moviendo a POSE_TRABAJO_2..."));
      if (brazo.goPose(POSE_TRABAJO_2, 1500)) {
        Serial.println(F("✓ Completado"));
      } else {
        Serial.println(F("✗ Abortado por sobrecarga"));
      }
      break;

    case 'P':  // 'p' minúscula es para poses user, 'P' mayúscula para posiciones
      Serial.println();
      brazo.printAllPositions();
      break;

    case 'c':
    case 'C':
      Serial.println(F("-> Limpiando errores de todos los motores..."));
      brazo.clearAllErrors();
      Serial.println(F("✓ Completado"));
      break;

    case 't':
    case 'T':
      brazo.printTorqueSnapshot();
      break;

    case '\n':
    case '\r':
      break;

    default:
      Serial.print(F("Comando desconocido: '"));
      Serial.print(cmd);
      Serial.println(F("'"));
      break;
  }
}

// Función para mostrar el menú
void handleEspCommand(uint8_t command) {
  const uint16_t playtimeMs = 1500;
  bool ok = false;
  bool checkFailed = false;
  const char* poseName = "reservado";

  switch (command) {
    case 0:
      poseName = "POSE_INICIAL";
      Serial.println(F("-> POSE_INICIAL"));
      ok = brazo.goPose(POSE_INICIAL, playtimeMs);
      break;
    case 1:
      poseName = "POSE_TRABAJO";
      Serial.println(F("-> POSE_TRABAJO"));
      ok = brazo.goPose(POSE_TRABAJO, playtimeMs);
      break;
    case 2:
      poseName = "POSE_TRABAJO_2";
      Serial.println(F("-> POSE_TRABAJO_2"));
      ok = brazo.goPose(POSE_TRABAJO_2, playtimeMs);
      break;
    case 3:
      poseName = "POSE_STANDBY";
      Serial.println(F("-> POSE_STANDBY"));
      ok = brazo.goPose(POSE_STANDBY, playtimeMs);
      break;
    case 4:
      // START: check de motores + ir a posición inicial (mismo flujo que 's' por Serial)
      poseName = "START";
      Serial.println(F("-> START: check de motores..."));
      if (brazo.checkAll()) {
        Serial.println(F("-> Moviendo a POSE_INICIAL..."));
        delay(500);
        ok = brazo.goPose(POSE_INICIAL, playtimeMs);
        if (ok) {
          Serial.println(F("✓ Brazo en posición inicial"));
        }
      } else {
        poseName = "CHECK_FAIL";
        checkFailed = true;
        Serial.println(F("⚠️ Check falló. Revisar motores antes de continuar."));
      }
      break;
    case 5:
      // Limpiar errores de todos los motores (mismo flujo que 'c' por Serial)
      poseName = "CLEAR_ERRORS";
      Serial.println(F("-> Limpiando errores de todos los motores..."));
      brazo.clearAllErrors();
      ok = true;
      Serial.println(F("✓ Errores limpiados"));
      break;
    default:
      Serial.print(F("-> Comando reservado: "));
      Serial.println(command);
      megaFeedbackSend(command, false, poseName);
      return;
  }

  if (!ok && !checkFailed) {
    Serial.println(F("✗ Movimiento abortado por sobrecarga"));
  }

  megaFeedbackSend(command, ok, poseName);
}

void printMenu() {
  Serial.println(F("Menú de control:"));
  Serial.println(F("  's' - START: Check de motores + ir a posición inicial"));
  Serial.println(F("  '1' - Ir a POSE_INICIAL"));
  Serial.println(F("  '2' - Ir a POSE_TRABAJO"));
  Serial.println(F("  '3' - Ir a POSE_TRABAJO_2"));
  Serial.println(F("  'P' - Mostrar posiciones de todos los motores"));
  Serial.println(F("  't' - PWM/torque actual (calibrar umbrales)"));
  Serial.println(F("  'c' - Limpiar errores de todos los motores"));
  Serial.println();
  Serial.println(F("Poses del usuario (RAM):"));
  Serial.println(F("  'l' - Listar poses guardadas"));
  Serial.println(F("  'g <n> [name]' - Grabar pose actual en slot n (1-20)"));
  Serial.println(F("  'a [name]' - Grabar pose actual en slot autoincremental"));
  Serial.println(F("  'p <n>' - Ir a pose user n (2500 ms)"));
  Serial.println(F("  'd <n>' - Borrar pose del slot n"));
  Serial.println(F("  'r' - Recorrer todas las poses guardadas (2500 ms cada una)"));
  Serial.println();
  Serial.print(F("Comando: "));
}

void setup() {
  // Inicializar Serial para comunicación con PC
  Serial.begin(115200);
  delay(500);

  Serial.println(F("=== Brazo Robótico HerkuleX DRS-0602 ==="));
  Serial.println();

  // Inicializar motores con IDs y límites
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    brazo.motors[i].id = MOTOR_IDS[i];
    brazo.motors[i].posMin = POS_MIN[i];
    brazo.motors[i].posMax = POS_MAX[i];
    brazo.motors[i].home = 0;  // Pendiente de calibración
    brazo.motors[i].accelRatio = 0;
    brazo.motors[i].accelTime = 0;
  }

  // Inicializar comunicación con servos
  brazo.begin();
  brazo.applySafetyLimits();

  espBridge.begin();
  espBridge.setHandler(handleEspCommand);
  megaFeedbackBegin();
  Serial.println(F("Bridge ESP activo (D5-D8 -> 22/24/26/28, latch D3 -> 44)."));
  Serial.println(F("Feedback serial Mega TX2 pin16 -> ESP D1 (con divisor)."));

  // Mostrar menú inicial
  printMenu();
}

void loop() {
  // Leer comandos desde Serial (esperar Enter para comandos con parámetros)
  if (Serial.available()) {
    String line = readLine();
    
    if (line.length() > 0) {
      parseCommand(line);
      Serial.println();
      printMenu();
    }
  }

  espBridge.poll();
}

