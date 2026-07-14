#include "esp_bridge.h"

void EspBridge::begin() {
  for (uint8_t i = 0; i < NUM_BITS; i++) {
    pinMode(dataPins_[i], INPUT);
  }
  pinMode(latchPin_, INPUT);
  lastLatchState_ = digitalRead(latchPin_);
}

void EspBridge::setHandler(CommandHandler handler) {
  handler_ = handler;
}

uint8_t EspBridge::readCommand() const {
  uint8_t value = 0;
  for (uint8_t i = 0; i < NUM_BITS; i++) {
    if (digitalRead(dataPins_[i])) {
      value |= (1u << i);
    }
  }
  return value;
}

void EspBridge::poll() {
  const int latchState = digitalRead(latchPin_);
  const uint32_t now = millis();

  if (lastLatchState_ == HIGH && latchState == LOW &&
      (now - lastLatchTs_) > DEBOUNCE_MS) {
    const uint8_t command = readCommand();

    Serial.print(F("[ESP] cmd="));
    Serial.print(command);
    Serial.print(F(" bits="));
    for (int i = 3; i >= 0; i--) {
      Serial.print((command >> i) & 1);
    }
    Serial.println();

    if (handler_) {
      handler_(command);
    }

    lastLatchTs_ = now;
  }

  lastLatchState_ = latchState;
}
