# Pendientes — cobot + interfaz web

Notas del 14/07/2026. Retomar mañana.

## 1. LED verde estable tras el start check

Hoy el `Motor::check()` deja el LED en verde, pero los movimientos posteriores llaman a `herkulex_moveTo(..., LED_OFF)` en `proyecto_cobot/src/motor.cpp`, así que la luz se apaga al primer movimiento.

**Objetivo:** mientras el motor esté OK (sin error tras el start check), mantener LED verde encendido durante el uso normal. Apagarlo o pasar a rojo solo si hay fallo / clear / apagado intencional.

Archivos probables: `proyecto_cobot/src/motor.cpp` (y revisar otros `LED_OFF` en movimientos).

## 2. Comandos de prueba de posiciones desde la web

La interfaz (`esp32_bridge/server/control_server.py`) ya tiene botones 0–15 y el Mega mapea:

| Cmd | Acción |
|-----|--------|
| 0–3 | Poses (INICIAL, TRABAJO, TRABAJO_2, STANDBY) |
| 4 | START (check + INICIAL) |
| 5 | Clear errors |
| 6–15 | Reservados |

**Objetivo:** poder probar/acceder a posiciones (las actuales y, si hace falta, más) desde la web de forma clara: labels útiles, y si faltan poses, asignar cmds reservados + cableado en Mega.

## 3. Secuencia de movimientos programada

**Objetivo:** definir una secuencia fija de poses (ej. INICIAL → TRABAJO → TRABAJO_2 → STANDBY) y un botón en la web para iniciarla (un cmd nuevo, p. ej. 6).

Pendiente decidir: si la secuencia corre en el Mega (más seguro / timing local) o se orquesta desde el servidor PC (más fácil de editar, más latencia).

## Fuera de alcance por ahora

No implementar hoy; solo dejar constancia para la próxima sesión.
