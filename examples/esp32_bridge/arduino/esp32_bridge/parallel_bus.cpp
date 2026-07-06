#include "parallel_bus.h"

#if defined(ESP8266)
  static const uint8_t DATA_PINS[4] = {14, 12, 13, 15};
  static const uint8_t LATCH_PIN = 0;   // D3 en NodeMCU
#elif defined(ESP32)
  static const uint8_t DATA_PINS[4] = {5, 18, 19, 21};
  static const uint8_t LATCH_PIN = 4;
#else
  #error "Placa no soportada"
#endif

static const uint32_t SETTLE_US = 100;
static const uint32_t PULSE_LOW_MS = 25;
static const uint32_t RECOVER_US = 100;

namespace ParallelBus {

void formatBits(uint8_t command, char out[5]) {
  command &= 0x0F;
  for (int i = 0; i < 4; i++) {
    out[3 - i] = ((command >> i) & 1) ? '1' : '0';
  }
  out[4] = '\0';
}

void begin() {
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(DATA_PINS[i], OUTPUT);
    digitalWrite(DATA_PINS[i], LOW);
  }
  pinMode(LATCH_PIN, OUTPUT);
  digitalWrite(LATCH_PIN, HIGH);
}

void sendCommand(uint8_t command) {
  command &= 0x0F;

  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(DATA_PINS[i], (command >> i) & 0x01);
  }

  delayMicroseconds(SETTLE_US);
  digitalWrite(LATCH_PIN, LOW);
  delay(PULSE_LOW_MS);
  digitalWrite(LATCH_PIN, HIGH);
  delayMicroseconds(RECOVER_US);
}

}
