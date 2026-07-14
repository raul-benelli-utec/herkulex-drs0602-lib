#ifndef MEGA_FEEDBACK_H
#define MEGA_FEEDBACK_H

#include <Arduino.h>

// Mega TX2 (pin 16) -> divisor de tensión -> ESP D1 (GPIO5)
void megaFeedbackBegin();
void megaFeedbackSend(uint8_t cmd, bool ok, const char* poseName);

#endif
