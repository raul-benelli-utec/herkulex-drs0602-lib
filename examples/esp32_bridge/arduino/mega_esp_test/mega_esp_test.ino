/*
 * Test mínimo: solo escucha el bus paralelo desde la ESP8266.
 * Subir al Mega 2560, abrir monitor serial 115200.
 * Si el cableado está bien, al pulsar un botón en la web verás:
 *   >>> COMANDO RECIBIDO: 1  bits=0001
 *
 * Cableado ESP8266 (NodeMCU) -> Mega:
 *   D5 (GPIO14) -> 22
 *   D6 (GPIO12) -> 24
 *   D7 (GPIO13) -> 26
 *   D8 (GPIO15) -> 28
 *   D3 (GPIO0)  -> 44  latch
 *   GND         -> GND
 */

const int PIN_BITS[4] = {22, 24, 26, 28};
const int PIN_LATCH = 44;

int lastLatch = HIGH;
unsigned long lastEdgeMs = 0;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    pinMode(PIN_BITS[i], INPUT);
  }
  pinMode(PIN_LATCH, INPUT);
  lastLatch = digitalRead(PIN_LATCH);

  Serial.println(F("=== Test bus ESP -> Mega ==="));
  Serial.println(F("Esperando comandos desde la ESP..."));
}

void loop() {
  int latch = digitalRead(PIN_LATCH);
  unsigned long now = millis();

  if (lastLatch == HIGH && latch == LOW && (now - lastEdgeMs) > 5) {
    uint8_t cmd = 0;
    for (int i = 0; i < 4; i++) {
      if (digitalRead(PIN_BITS[i])) {
        cmd |= (1 << i);
      }
    }

    Serial.print(F(">>> COMANDO RECIBIDO: "));
    Serial.print(cmd);
    Serial.print(F("  bits="));
    for (int i = 3; i >= 0; i--) {
      Serial.print((cmd >> i) & 1);
    }
    Serial.println();

    lastEdgeMs = now;
  }

  lastLatch = latch;
}
