#ifndef PARALLEL_BUS_H
#define PARALLEL_BUS_H

#include <Arduino.h>

namespace ParallelBus {

void begin();
void sendCommand(uint8_t command);
void formatBits(uint8_t command, char out[5]);

#if defined(ESP8266)
  static const char* const PIN_LABELS[4] = {"D5", "D6", "D7", "D8"};
  static const uint8_t PIN_GPIOS[4] = {14, 12, 13, 15};
  static const uint8_t PIN_MEGA[4] = {22, 24, 26, 28};
  static const char* LATCH_LABEL = "D3";
  static const uint8_t LATCH_GPIO = 0;
  static const uint8_t LATCH_MEGA = 44;
#elif defined(ESP32)
  static const char* const PIN_LABELS[4] = {"D5", "D6", "D7", "D8"};
  static const uint8_t PIN_GPIOS[4] = {5, 18, 19, 21};
  static const uint8_t PIN_MEGA[4] = {22, 24, 26, 28};
  static const char* LATCH_LABEL = "D4";
  static const uint8_t LATCH_GPIO = 4;
  static const uint8_t LATCH_MEGA = 44;
#endif

}

#endif
