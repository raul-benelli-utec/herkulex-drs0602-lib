#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
  #include <WiFiUdp.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
  #include <WiFiUdp.h>
#else
  #error "Placa no soportada"
#endif

#include "wifi_config.h"
#include "parallel_bus.h"

#ifndef DISCOVERY_PORT
#define DISCOVERY_PORT 4210
#endif

#if defined(ESP8266)
static WiFiClient wifiClient;
#endif

static WiFiUDP udp;
static String g_host = SERVER_HOST;
static uint16_t g_port = SERVER_PORT;
static bool g_discovered = false;
static unsigned long lastWhoMs = 0;

static const char* megaPoseName(uint8_t cmd) {
  switch (cmd) {
    case 0: return "POSE_INICIAL";
    case 1: return "POSE_TRABAJO";
    case 2: return "POSE_TRABAJO_2";
    case 3: return "POSE_STANDBY";
    case 4: return "START";
    case 5: return "CLEAR_ERRORS";
    default: return "reservado";
  }
}

static String serverUrl(const char* path) {
  String url = F("http://");
  url += g_host;
  url += ':';
  url += g_port;
  url += path;
  return url;
}

static bool applyBeacon(const char* buf) {
  if (strncmp(buf, "BRAZO_SRV ", 10) != 0) {
    return false;
  }
  char host[48];
  unsigned port = 0;
  if (sscanf(buf + 10, "%47s %u", host, &port) != 2 || port == 0 || port > 65535) {
    return false;
  }
  const bool changed = (!g_discovered || g_host != host || g_port != static_cast<uint16_t>(port));
  g_host = host;
  g_port = static_cast<uint16_t>(port);
  g_discovered = true;
  if (changed) {
    Serial.print(F("Servidor encontrado: "));
    Serial.print(g_host);
    Serial.print(':');
    Serial.println(g_port);
  }
  return true;
}

static void pollDiscovery() {
  int n = udp.parsePacket();
  while (n > 0) {
    char buf[64];
    const int len = udp.read(buf, sizeof(buf) - 1);
    if (len > 0) {
      buf[len] = '\0';
      applyBeacon(buf);
    }
    n = udp.parsePacket();
  }
}

static void askWho() {
  udp.beginPacket(IPAddress(255, 255, 255, 255), DISCOVERY_PORT);
  udp.write(reinterpret_cast<const uint8_t*>("BRAZO_WHO"), 9);
  udp.endPacket();
  lastWhoMs = millis();
}

static void discoverServer(unsigned long waitMs) {
  Serial.println(F("Buscando servidor (UDP discovery)..."));
  askWho();
  const unsigned long t0 = millis();
  while (millis() - t0 < waitMs) {
    pollDiscovery();
    if (g_discovered) {
      return;
    }
    if (millis() - lastWhoMs > 800) {
      askWho();
    }
    delay(20);
  }
  Serial.print(F("Sin beacon; fallback "));
  Serial.print(g_host);
  Serial.print(':');
  Serial.println(g_port);
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
    udp.begin(DISCOVERY_PORT);
    discoverServer(5000);
  }
}

static void reportToServer(uint8_t cmd) {
  char bits[5];
  ParallelBus::formatBits(cmd, bits);

  String url = serverUrl("/api/esp/report");
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
  String url = serverUrl("/api/poll");
#if defined(ESP8266)
  http.begin(wifiClient, url);
#else
  http.begin(url);
#endif
  http.setTimeout(800);
  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    http.end();
    if (millis() - lastWhoMs > 2000) {
      g_discovered = false;
      askWho();
    }
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
    g_discovered = false;
    connectWiFi();
    return;
  }
  pollDiscovery();
  pollServer();
  delay(POLL_INTERVAL_MS);
}
