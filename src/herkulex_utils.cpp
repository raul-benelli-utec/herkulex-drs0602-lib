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

uint16_t readPosition(byte id) {
  byte data[2] = {ADDR_POS_CALIB_L, 0x02};  // Dirección y longitud
  byte packet[9];  // 7 + 2

  buildPacket(packet, id, CMD_RAM_READ, data, 2);
  Serial1.write(packet, packet[2]);

  delay(5);  // Espera corta

  int timeout = 1000;
  while (Serial1.available() < 13 && timeout-- > 0) delay(1);

  if (Serial1.available() >= 13) {
    byte respuesta[13];
    for (int i = 0; i < 13; i++) respuesta[i] = Serial1.read();

    uint16_t pos = respuesta[9] | (respuesta[10] << 8);
    return pos;
  }

  return 0xFFFF; // Error
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

void herkulex_moveTo(byte id, uint16_t position, byte playtime) {
  byte data[5];

  data[0] = playtime;                // Tiempo de movimiento (11.2 ms * N)
  data[1] = position & 0xFF;         // Posición LSB
  data[2] = (position >> 8) & 0xFF;  // Posición MSB
  data[3] = 0x00;                    // Info: Modo posición, sin LED
  data[4] = id;                      // ID del motor

  byte packet[12];
  buildPacket(packet, id, CMD_S_JOG, data, 5);
  sendPacket(packet);
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

struct HerkulexMotor {
  byte id;
  int16_t offset = 0;  // offset de calibración
};

void commandSetAcceleration(byte id, byte accelRatio, byte accelTime) {
  byte packet[13];
  byte data[4];

  data[0] = ADDR_ACCEL_RATIO; // 0x0E
  data[1] = 2;
  data[2] = accelRatio;      // 0–100 %
  data[3] = accelTime;       // ×11.2 ms

  buildPacket(packet, id, CMD_RAM_WRITE, data, 4);
  sendPacket(packet);

  Serial.print(F("Aceleración: ratio="));
  Serial.print(accelRatio);
  Serial.print(F("%, time="));
  Serial.print(accelTime * 11.2);
  Serial.println(F(" ms"));
}

void herkulex_safeMoveTo(byte id, uint16_t targetPosition) {
  // Verificar si la posición objetivo está dentro del rango permitido
  if (targetPosition < POS_MIN_DEF || targetPosition > POS_MAX_DEF) {
    Serial.print(F(" Posición "));
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

  
  /*
  // Calcular diferencia
  uint16_t diff = abs((int32_t)targetPosition - (int32_t)currentPosition);

  // Calcular playtime según distancia
  uint16_t playtime = map(diff, 0, 10000, PLAYTIME_MIN, PLAYTIME_MAX);
  playtime = constrain(playtime, PLAYTIME_MIN, PLAYTIME_MAX);

  // Ejecutar movimiento
  herkulex_moveTo(id, targetPosition, playtime);

  // Debug
  Serial.print(F("✅ Moviendo de "));
  Serial.print(currentPosition);
  Serial.print(F(" a "));
  Serial.print(targetPosition);
  Serial.print(F(" con playtime: "));
  Serial.print(playtime);
  Serial.println(F(" (x11.2ms)"));
  */
  //
  // Calcular playtime según distancia
  uint16_t diff = abs((int32_t)targetPosition - (int32_t)currentPosition);

  // Mapeás a un rango útil, por ejemplo de 10 a 255 (en ticks de 11.2 ms)
  uint8_t playtime = map(diff, 0, 10000, 10, 255); 
  playtime = constrain(playtime, 10, 255);

  // Ejecutar
  herkulex_moveTo(id, targetPosition, playtime);

  // Log
  Serial.print(F("✅ Moviendo de "));
  Serial.print(currentPosition);
  Serial.print(F(" a "));
  Serial.print(targetPosition);
  Serial.print(F(" con playtime: "));
  Serial.print(playtime);
  Serial.println(F(" (x11.2ms)"));

}

void herkulex_sJogMultiMove(byte ids[], uint16_t posiciones[], byte cantidad, byte playtime) {
  const byte CMD = CMD_S_JOG;       // Comando de movimiento múltiple
  const byte ID_BROADCAST = 0xFE;   // ID de broadcast para todos los motores

  byte data[1 + 4 * cantidad];      // 1 byte playtime + 4 por motor
  data[0] = playtime;               // Playtime compartido

  byte idx = 1;
  for (byte i = 0; i < cantidad; i++) {
    data[idx++] = posiciones[i] & 0xFF;         // Pos_L
    data[idx++] = (posiciones[i] >> 8) & 0xFF;  // Pos_H
    data[idx++] = 0x00;                         // Info: modo posición, sin LED ni STOP
    data[idx++] = ids[i];                       // ID del motor
  }

  byte packet[7 + sizeof(data)];  // Cabecera + payload
  buildPacket(packet, ID_BROADCAST, CMD, data, idx);
  sendPacket(packet);
  delay(4000);  // Esperar que terminen de moverse
  revisarErroresMotores(ids, cantidad);
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
