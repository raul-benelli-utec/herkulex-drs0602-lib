#ifndef ESP_BRIDGE_H
#define ESP_BRIDGE_H

#include <Arduino.h>

class EspBridge {
 public:
  using CommandHandler = void (*)(uint8_t command);

  void begin();
  void poll();
  void setHandler(CommandHandler handler);

 private:
  static const uint8_t NUM_BITS = 4;

  const uint8_t dataPins_[NUM_BITS] = {22, 24, 26, 28};
  const uint8_t latchPin_ = 44;

  CommandHandler handler_ = nullptr;
  int lastLatchState_ = HIGH;
  uint32_t lastLatchTs_ = 0;
  static const uint16_t DEBOUNCE_MS = 5;

  uint8_t readCommand() const;
};

#endif
