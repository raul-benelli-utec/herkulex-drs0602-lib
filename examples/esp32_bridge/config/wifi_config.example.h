#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#define WIFI_SSID     "TU_RED_WIFI"
#define WIFI_PASSWORD "TU_PASSWORD"

// Fallback si el discovery UDP no encuentra el servidor.
// Con control_server.py corriendo, la ESP suele aprender sola IP y puerto.
#define SERVER_HOST       "192.168.1.100"
#define SERVER_PORT       8080
#define DISCOVERY_PORT    4210
#define POLL_INTERVAL_MS  150

#endif
