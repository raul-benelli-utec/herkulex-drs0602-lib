# Plan de la Librería HerkuleX DRS-0602

### Objetivo general
Crear una librería clara, modular y mantenible para controlar servomotores HerkuleX DRS-0602, con funciones de torque, movimiento por ángulo, velocidad, lectura de posición y lectura de errores.

---

## Funciones actuales (desde herkulex_utils.h)
- Por listar mañana con ChatGPT/Cursor.

---

## Funciones mínimas requeridas (API pública)
- `setTorque(id, bool on)`
- `movePos(id, angle_deg, time_ms)`
- `setSpeed(id, speed_value)` (si corresponde)
- `readPosition(id)`
- `readErrorFlags(id)`
- `ping(id)`
- `setLedColor(id, color)`

---

## Objetivos de refactor
1. Separar código en módulos limpios (`core`, `commands`, `utils`).
2. Establecer nombres consistentes y documentación.
3. Crear ejemplos funcionales:
   - Control de un solo servo.
   - Movimiento básico del brazo.
4. Preparar README para publicación final.
