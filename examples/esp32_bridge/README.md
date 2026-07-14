# Bridge ESP32 -> Arduino Mega (polling + interfaz web)

La **PC** corre un servidor con botones. La **ESP** pregunta cada ~150 ms si hay comando nuevo y lo envía al Mega por el bus paralelo.

## Arquitectura

```
[Navegador] --click--> [PC: control_server.py] <--GET /api/poll-- [ESP32] --GPIO--> [Arduino Mega]
```

| Paso | Qué pasa |
|------|----------|
| 1 | Abrís `http://<IP_PC>:8080/` en el celular o PC |
| 2 | Pulsás un botón → el servidor encola el comando |
| 3 | La ESP hace polling → recibe el comando → pulsa los pines |
| 4 | El Mega ejecuta la pose |

Latencia típica: **~150–300 ms** (intervalo de poll + WiFi). Ajustable con `POLL_INTERVAL_MS`.

## Cableado ESP → Mega

| ESP32 | Mega | Función |
|-------|------|---------|
| D5 (GPIO 5) | 22 | bit 0 |
| D6 (GPIO 18) | 24 | bit 1 |
| D7 (GPIO 19) | 26 | bit 2 |
| D8 (GPIO 21) | 28 | bit 3 |
| D3 (GPIO 0) | 44 | latch |
| GND | GND | común |

### Feedback serial Mega → ESP (opcional)

| Mega | ESP8266 | Notas |
|------|---------|-------|
| **TX2 pin 16** | **D1 (GPIO 5)** RX | **Divisor de tensión obligatorio** (Mega 5 V → ESP 3.3 V) |
| GND | GND | común |

Divisor sugerido entre pin 16 y D1:

```
Mega TX2 (16) ---[ 1 kΩ ]---+--- ESP D1 (RX)
                             |
                           [ 2 kΩ ]
                             |
                            GND
```

Salida ~3.3 V. **No conectar el TX del Mega directo a la ESP** (riesgo de daño).

El Mega envía líneas como `OK:1:POSE_TRABAJO` a 115200 baud. La ESP las reenvía al servidor.

## Comandos

| Valor | Pose |
|-------|------|
| 0 | INICIAL |
| 1 | TRABAJO |
| 2 | TRABAJO 2 |
| 3 | STANDBY |

## 1. Servidor en la PC

```bash
cd examples/esp32_bridge/server
python3 control_server.py
```

Al arrancar imprime la **IP de tu PC** y levanta un **beacon UDP** (puerto 4210).
La ESP busca ese anuncio y aprende sola `IP:puerto` (útil en hotspot del celular, donde la IP puede cambiar).

`SERVER_HOST` / `SERVER_PORT` en `wifi_config.h` quedan como **fallback** si no hay beacon.

Abrí en el navegador la URL que indica (ej. `http://192.168.1.50:8080/`).

## 2. Firmware ESP (ESP8266 o ESP32)

### Arduino IDE — importante

Tu placa es **ESP8266** (NodeMCU, Wemos D1 mini, etc.), **no ESP32**.

| Opción | Valor correcto |
|--------|----------------|
| Gestor de tarjetas | **esp8266** by ESP8266 Community |
| Placa | **NodeMCU 1.0 (ESP-12E Module)** |
| ❌ No usar | ESP32 Dev Module |

Si elegís ESP32 con un chip 8266, aparece:
`This chip is ESP8266, not ESP32. Wrong chip argument?`

1. `Herramientas` → `Placa` → **NodeMCU 1.0 (ESP-12E Module)**
2. Abrir `arduino/esp32_bridge/esp32_bridge.ino`
3. Editar `wifi_config.h` (`SERVER_HOST`, `SERVER_PORT`)
4. Subir

El monitor serial debe mostrar `Chip: ESP8266`.

### PlatformIO

```bash
# ESP8266 (NodeMCU)
pio run -e nodemcuv2 -t upload

# ESP32 (si tuvieras esa placa)
pio run -e esp32dev -t upload
```

## 3. Firmware Mega

Subir `examples/proyecto_cobot/` (incluye `esp_bridge` integrado).

## API (por si querés extender)

| Método | Ruta | Descripción |
|--------|------|-------------|
| GET | `/` | Interfaz web con botones |
| GET | `/api/poll` | ESP consulta: `-1` = nada, `0-15` = comando (se consume) |
| POST | `/api/command` | Body JSON `{"cmd": 1}` encola comando |

## Solución de problemas

- **`WiFi.h: No such file`**: no tenés el core ESP32 o la placa seleccionada no es ESP32.
- **Poll HTTP error -1**: la PC no alcanzable; revisar misma red WiFi / hotspot, firewall, y que el servidor esté corriendo (beacon UDP en 4210).
- **Comando en web pero el brazo no mueve**: revisar cableado GPIO y monitor serial del Mega.
