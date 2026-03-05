# HerkuleX DRS-0602 Library

Librería completa para controlar servomotores HerkuleX DRS-0602 desde Arduino.

## Quick Start

```cpp
#include "herkulex_utils.h"

void setup() {
  Serial1.begin(115200);  // Puerto de comunicación con servos
  commandEnableTorque(1);  // Activar torque del servo ID 1
  herkulex_safeMoveTo(1, 16384);  // Mover a posición 16384
  uint16_t pos = readPosition(1);  // Leer posición actual
}
```

## Documentación

- **[API Reference](docs/API.md)** - Documentación completa de todas las funciones públicas
- **[Quick Reference](docs/API_QUICK_REFERENCE.md)** - Resumen rápido de funciones

## Características

- Control de posición y velocidad
- Lectura de sensores (voltaje, temperatura, PWM)
- Control de torque adaptativo
- Estimación de torque y corriente basada en PWM
- Movimientos sincronizados múltiples
- Manejo de errores y estados

## Ejemplos

- **Brazo (menú s, 1, 2, 3, P, c, poses)**: abrir como carpeta `examples/proyecto_cobot` en VS Code y usar Upload + Monitor. Ver [README del ejemplo](examples/proyecto_cobot/README.md).
- **Self-check (menú SCAN, SelfCheck, etc.)**: abrir `examples/self_check`.

## Requisitos

- Arduino con puerto Serial adicional (Serial1, Serial2, etc.)
- Servomotores HerkuleX DRS-0602
- Baudrate: 115200 (configurable)

## Instalación

1. Copia la carpeta `src` a tu proyecto Arduino
2. Incluye `herkulex_utils.h` en tu sketch
3. Configura el puerto Serial para comunicación con los servos

## Licencia

Ver archivo LICENSE
