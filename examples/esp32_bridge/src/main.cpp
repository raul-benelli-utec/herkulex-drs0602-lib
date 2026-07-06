#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
#else
  #error "Placa no soportada"
#endif

#include "wifi_config.h"
#include "parallel_bus.h"

#if defined(ESP8266)
static WiFiClient wifiClient;
#endif

static const char* megaPoseName(uint8_t cmd) {
  switch (cmd) {
    case 0: return "POSE_INICIAL";
    case 1: return "POSE_TRABAJO";
    case 2: return "POSE_TRABAJO_2";
    case 3: return "POSE_STANDBY";
    default: return "reservado";
  }
}

static void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print(F("Conectando a "));
  Serial.println(WIFI_SSID);
  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 60) {
    delay(500);
    Serial.print('.');
    attempts++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("IP: "));
    Serial.println(WiFi.localIP());
  }
}

static void reportToServer(uint8_t cmd) {
  char bits[5];
  ParallelBus::formatBits(cmd, bits);

  String url = String(F("http://")) + SERVER_HOST + ":" + SERVER_PORT + F("/api/esp/report");
  String body = F("{\"cmd\":");
  body += cmd;
  body += F(",\"bits\":\"");
  body += bits;
  body += F("\",\"mega_expected\":\"");
  body += megaPoseName(cmd);
  body += F("\",\"pins\":[");
  for (uint8_t i = 0; i < 4; i++) {
    if (i) body += ',';
    body += F("{\"esp\":\"");
    body += ParallelBus::PIN_LABELS[i];
    body += F("\",\"gpio\":");
    body += ParallelBus::PIN_GPIOS[i];
    body += F(",\"level\":");
    body += (cmd >> i) & 1;
    body += F(",\"mega\":");
    body += ParallelBus::PIN_MEGA[i];
    body += '}';
  }
  body += F("],\"latch\":{\"esp\":\"");
  body += ParallelBus::LATCH_LABEL;
  body += F("\",\"gpio\":");
  body += ParallelBus::LATCH_GPIO;
  body += F(",\"mega\":");
  body += ParallelBus::LATCH_MEGA;
  body += F("}}");

  HTTPClient http;
#if defined(ESP8266)
  http.begin(wifiClient, url);
#else
  http.begin(url);
#endif
  http.addHeader(F("Content-Type"), F("application/json"));
  http.POST(body);
  http.end();
}

static void pollServer() {
  HTTPClient http;
  String url = String(F("http://")) + SERVER_HOST + ":" + SERVER_PORT + F("/api/poll");
#if defined(ESP8266)
  http.begin(wifiClient, url);
#else
  http.begin(url);
#endif
  http.setTimeout(800);
  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    http.end();
    return;
  }
  String body = http.getString();
  body.trim();
  http.end();
  if (body.length() == 0 || body == "-1") return;

  const int cmd = body.toInt();
  if (cmd < 0 || cmd > 15) return;

  ParallelBus::sendCommand(static_cast<uint8_t>(cmd));
  reportToServer(static_cast<uint8_t>(cmd));
  Serial.print(F("CMD "));
  Serial.println(cmd);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  ParallelBus::begin();
  connectWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    connectWiFi();
    return;
  }
  pollServer();
  delay(POLL_INTERVAL_MS);
}
