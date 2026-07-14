#include "mega_feedback.h"

void megaFeedbackBegin() {
  Serial2.begin(115200);
}

void megaFeedbackSend(uint8_t cmd, bool ok, const char* poseName) {
  Serial2.print(ok ? F("OK:") : F("ERR:"));
  Serial2.print(cmd);
  Serial2.print(':');
  Serial2.println(poseName);
}
