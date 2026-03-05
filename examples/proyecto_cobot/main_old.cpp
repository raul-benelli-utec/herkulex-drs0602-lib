
#include <Arduino.h>
#include "herkulex_defs.h"
#include "herkulex_utils.h"
void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);  // Comunicación con motores
  delay(500);
  

  // Pines desde la ESP (sin pull-up, la ESP entrega HIGH/L
};
void loop() {};

/*
#include <Arduino.h>
#include "herkulex_defs.h"
#include "herkulex_utils.h"

String inputString = "";
bool inputComplete = false;

enum State { MENU_PRINCIPAL, MENU_MOTOR, CAMBIO_ID, PEDIR_POSICION, PEDIR_OFFSET, PEDIR_ACELERACION, POSICION_CERO };

// --- Pines para comunicación con la ESP ---
const int pinBits[4] = {22, 24, 26, 28};  // LSB a MSB
const int pinLatch = 44;                  // Confirmación desde ESP

// Debounce / detección de borde
int lastLatchState = HIGH;
uint32_t lastLatchTs = 0;
const uint16_t debounce_ms = 20;

State currentState = MENU_PRINCIPAL;

byte motorID = 0;
byte motorIDs[5] = {1, 2, 3, 4, 5};

// --- Prototipos ---
void printMenuPrincipal();
void handleMenuPrincipal();
void handleMenuMotor();
void handleCambioID();
void printMenuMotor();
void serialEvent();
void commandClearAllErrors();
void ejecutarAccion(byte comando);

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);  // Comunicación con motores
  delay(500);
  Serial.println(F("=== Configurador HerkuleX ==="));
  printMenuPrincipal();

  // Pines desde la ESP (sin pull-up, la ESP entrega HIGH/LOW)
  for (int i = 0; i < 4; i++) {
    pinMode(pinBits[i], INPUT);
  }
  pinMode(pinLatch, INPUT);
}

void loop() {
  // --- Procesar entrada Serial ---
  serialEvent();
  if (inputComplete) {
    inputString.trim();

    switch (currentState) {
      case MENU_PRINCIPAL: handleMenuPrincipal(); break;
      case MENU_MOTOR:     handleMenuMotor();     break;
      case CAMBIO_ID:      handleCambioID();      break;

      case PEDIR_POSICION: {
        uint16_t posDeseada = inputString.toInt();
        herkulex_safeMoveTo(motorID, posDeseada);
        currentState = MENU_MOTOR;
        printMenuMotor();
        break;
      }

      case PEDIR_OFFSET: {
        int offset = inputString.toInt();
        commandSetCalibrationDiff(motorID, offset);
        Serial.print(F("Offset aplicado: "));
        Serial.println(offset);
        currentState = MENU_MOTOR;
        printMenuMotor();
        break;
      }

      case PEDIR_ACELERACION: {
        int spaceIndex = inputString.indexOf(' ');
        if (spaceIndex == -1) {
          Serial.println(F("Formato incorrecto. Use: [ratio] [tiempo]"));
        } else {
          byte ratio = inputString.substring(0, spaceIndex).toInt();
          byte tiempo = inputString.substring(spaceIndex + 1).toInt();
          if (ratio > 50) ratio = 50;
          if (tiempo < 1) tiempo = 1;
          if (tiempo > 254) tiempo = 254;
          commandSetAcceleration(motorID, ratio, tiempo);
        }
        currentState = MENU_MOTOR;
        printMenuMotor();
        break;
      }
    }

    inputString = "";
    inputComplete = false;
  }

  // --- Procesar comandos binarios desde la ESP ---
  int latchState = digitalRead(pinLatch);
  uint32_t now = millis();
  if (lastLatchState == HIGH && latchState == LOW && (now - lastLatchTs) > debounce_ms) {
    byte valor = 0;
    Serial.print(F("[ESP] bits = "));
    for (int i = 0; i < 4; i++) {
      int b = digitalRead(pinBits[i]) & 0x01;
      valor |= (b << i);
      Serial.print(b);
      if (i < 3) Serial.print(' ');
    }
    Serial.print(F("  -> comando = "));
    Serial.println(valor);

    ejecutarAccion(valor);
    lastLatchTs = now;
  }
  lastLatchState = latchState;
}

// --- Función para accionar según valor binario ---
void ejecutarAccion(byte comando) {
  Serial.print(F("🚀 Acción binaria recibida: "));
  Serial.println(comando);

  switch (comando) {
    case 0: {
      uint16_t pose[] = POSE_TRABAJO;
      Serial.println(F("-> POSE_TRABAJO"));
      herkulex_sJogMultiMove(motorIDs, pose, 5, 500);
      break;
    }
    case 1: {
      uint16_t pose[] = POSE_TRABAJO_2;
      Serial.println(F("-> POSE_TRABAJO_2"));
      herkulex_sJogMultiMove(motorIDs, pose, 5, 500);
      break;
    }
    case 2: {
      Serial.println(F("-> Comando 2: acción futura"));
      break;
    }
    default: {
      Serial.print(F("-> Comando "));
      Serial.print(comando);
      Serial.println(F(": acción genérica"));
      break;
    }
  }
}

// =============================
// Restante: Menús y funciones
// =============================

void printMenuPrincipal() {
  Serial.println(F("\n--- Menú Principal ---"));
  Serial.println(F("0 - Cambiar ID de motor"));
  Serial.println(F("1-254 - Seleccionar motor por ID"));
  Serial.print(F("Ingrese opción: "));
}

void handleMenuPrincipal() {
  int value = inputString.toInt();

  if (value == 0) {
    Serial.println(F("Ingrese ID actual y nuevo separados por espacio (Ej: 253 11):"));
    currentState = CAMBIO_ID;
  } else if (value >= 1 && value <= 255) {
    motorID = value;
    Serial.print(F("Motor seleccionado: "));
    Serial.println(motorID);
    printMenuMotor();
    currentState = MENU_MOTOR;
  } else if (value == 1000) {
    Serial.println(F("Va a la posición cero "));
    currentState = MENU_PRINCIPAL;
    uint16_t pose[] = POSE_INICIAL;
    herkulex_sJogMultiMove(motorIDs, pose, 5, 500);
    printMenuPrincipal();
  } else if (value == 2000) {
    Serial.println(F("Va a la posición trabajo 1 "));
    currentState = MENU_PRINCIPAL;
    uint16_t pose[] = POSE_TRABAJO;
    herkulex_sJogMultiMove(motorIDs, pose, 5, 500);
    printMenuPrincipal();
  } else if (value == 3000) {
    Serial.println(F("Va a la posición trabajo 2 "));
    currentState = MENU_PRINCIPAL;
    uint16_t pose[] = POSE_TRABAJO_2;
    herkulex_sJogMultiMove(motorIDs, pose, 5, 500);
    printMenuPrincipal();
  } else if (value == 4000) {
    Serial.println(F("Va a la posición mate2 "));
    currentState = MENU_PRINCIPAL;
    uint16_t pose[] = POSE_END;
    herkulex_sJogMultiMove(motorIDs, pose, 5, 500);
    printMenuPrincipal();
  } else if (value == 5000) {
    Serial.println(F("Va a la posición trabajo sup "));
    currentState = MENU_PRINCIPAL;
    uint16_t pose[] = POSE_MATE_2;
    herkulex_sJogMultiMove(motorIDs, pose, 5, 500);
    printMenuPrincipal();
  } else if (value == 6000) {
    Serial.println(F("Limpiando errores de todos los motores..."));
    commandClearAllErrors();
    printMenuPrincipal();
  } else if (value == 7000) {
    Serial.println(F("Leyendo errores de todos los motores..."));
    revisarErroresMotores(motorIDs, 5);
    printMenuPrincipal();
  } else if (value == 8000) {
    Serial.println(F("Va a la posición END "));
    currentState = MENU_PRINCIPAL;
    uint16_t pose[] = POSE_END;
    herkulex_sJogMultiMove(motorIDs, pose, 5, 500);
    printMenuPrincipal();
  } else {
    Serial.println(F("ID inválido."));
    printMenuPrincipal();
  }
}

void commandClearAllErrors() {
  for (byte i = 0; i < 5; i++) {
    commandClearError(motorIDs[i]);
    delay(10);
  }
}

void printMenuMotor() {
  Serial.println(F("\n--- Menú Motor ---"));
  Serial.println(F("1 - Reiniciar"));
  Serial.println(F("2 - Leer posición"));
  Serial.println(F("3 - Aplicar offset"));
  Serial.println(F("4 - Activar torque"));
  Serial.println(F("5 - Encender LED verde"));
  Serial.println(F("6 - Mover a posición 512"));
  Serial.println(F("7 - Configurar aceleración (ratio y tiempo)"));
  Serial.println(F("0 - Volver al menú principal"));
  Serial.print(F("Ingrese opción para motor "));
  Serial.print(motorID);
  Serial.print(F(": "));
}

void handleMenuMotor() {
  int option = inputString.toInt();

  switch (option) {
    case 1:
      Serial.println(F("Reiniciando..."));
      herkulex_reboot(motorID);
      break;
    case 2: {
      uint16_t pos = readPosition_debug(motorID);
      Serial.print(F("Posición actual: "));
      Serial.println(pos);
      break;
    }
    case 3:
      Serial.println(F("Ingrese offset a aplicar (positivo o negativo):"));
      currentState = PEDIR_OFFSET;
      break;
    case 4:
      commandEnableTorque(motorID);
      Serial.println(F("Torque activado."));
      break;
    case 5:
      herkulex_ledSet(motorID, 0x01);
      Serial.println(F("LED verde encendido."));
      break;
    case 6:
      Serial.println(F("Ingrese posición deseada (10100 a 21050):"));
      currentState = PEDIR_POSICION;
      break;
    case 7:
      Serial.println(F("Ingrese aceleración (ratio 0–50) y tiempo (1–254) separados por espacio (Ej: 20 30):"));
      currentState = PEDIR_ACELERACION;
      break;
    case 0:
      currentState = MENU_PRINCIPAL;
      printMenuPrincipal();
      return;
    default:
      Serial.println(F("Opción inválida."));
  }
  if (currentState == MENU_MOTOR) printMenuMotor();
}

void handleCambioID() {
  int spaceIndex = inputString.indexOf(' ');
  if (spaceIndex == -1) {
    Serial.println(F("Formato incorrecto. Use: [viejo_id] [nuevo_id]"));
  } else {
    byte oldID = inputString.substring(0, spaceIndex).toInt();
    byte newID = inputString.substring(spaceIndex + 1).toInt();
    commandSetID(oldID, newID);
    Serial.print(F("ID cambiado de "));
    Serial.print(oldID);
    Serial.print(F(" a "));
    Serial.println(newID);
  }
  currentState = MENU_PRINCIPAL;
  printMenuPrincipal();
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n' || inChar == '\r') {
      if (inputString.length() > 0) {
        inputComplete = true;
      }
    } else {
      inputString += inChar;
    }
  }
}
*/