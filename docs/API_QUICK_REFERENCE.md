# API Quick Reference - HerkuleX DRS-0602

Referencia rápida de todas las funciones públicas de la librería.

## Funciones Básicas

| Función | Descripción |
|---------|-------------|
| `herkulex_ledSet(id, color)` | Controla el LED del servo |
| `herkulex_ledOff(id)` | Apaga el LED del servo |
| `herkulex_reboot(id)` | Reinicia el servo (requerido después de escribir EEPROM) |

## Control de Torque

| Función | Descripción |
|---------|-------------|
| `commandEnableTorque(id)` | Activa el torque (necesario para movimientos) |
| `commandTorqueFree(id)` | Desactiva el torque (eje libre) |
| `commandBrakeOn(id)` | Activa el freno eléctrico |

## Comandos de Movimiento

| Función | Descripción |
|---------|-------------|
| `herkulex_computePlaytime(current, goal, v_counts_per_s)` | Calcula tiempo de movimiento basado en distancia y velocidad |
| `herkulex_moveTo(id, position, playtime, ledFlags=0)` | Mueve a posición usando S_JOG (sincronizado) |
| `herkulex_iJog(id, position, playtime, ledFlags=0)` | Mueve a posición usando I_JOG (individual) |
| `herkulex_continuousRotation(id, speed, playtime, ledFlags=0)` | Giro continuo (modo velocidad) |
| `herkulex_safeMoveTo(id, targetPosition, velocity=0)` | Movimiento seguro con cálculo automático de playtime |
| `herkulex_sJogMultiMove(ids[], posiciones[], cantidad, playtime, ledFlags=0)` | Mueve múltiples servos sincronizados |

## Lectura de Registros

| Función | Descripción | Retorno |
|---------|-------------|---------|
| `readPosition(id)` | Lee posición calibrada actual | `uint16_t` (0xFFFF = error) |
| `herkulex_readAbsolutePosition(id, position)` | Lee posición absoluta sin calibrar | `bool` |
| `herkulex_readTargetPosition(id, position)` | Lee posición objetivo | `bool` |
| `herkulex_readVoltage(id, voltage)` | Lee voltaje (décimas de voltio) | `bool` |
| `herkulex_readTemperature(id, temperature)` | Lee temperatura (°C) | `bool` |
| `herkulex_readPWM(id, pwm)` | Lee PWM actual (0-1023) | `bool` |
| `herkulex_readCurrentMode(id, mode)` | Lee modo de control (0=pos, 1=vel) | `bool` |
| `herkulex_readSpeed(id, speed)` | Lee velocidad deseada | `bool` |

## Escritura de Registros

| Función | Descripción | Requiere REBOOT |
|---------|-------------|----------------|
| `commandSetID(old_id, new_id)` | Cambia el ID del servo | ✓ |
| `commandSetMaxPosition(id, pos)` | Configura posición máxima | ✓ |
| `commandSetMinPosition(id, pos)` | Configura posición mínima | ✓ |
| `commandSetPositionLimits(id, min, max)` | Configura límites de posición | ✓ |
| `commandSetCalibrationDiff(id, offset)` | Configura offset de calibración | ✓ |
| `commandSetAcceleration(id, accelRatio, accelTime)` | Configura perfil de aceleración | ✗ |
| `commandSetServoPolicy(id, policy)` | Configura política de respuesta ACK | ✗ |
| `herkulex_setMaxPWM(id, maxPWM, writeToEEP=false)` | Configura PWM máximo | Opcional |
| `herkulex_setMinPWM(id, minPWM, writeToEEP=false)` | Configura PWM mínimo | Opcional |
| `herkulex_setOverloadThreshold(id, threshold, writeToEEP=false)` | Configura umbral de sobrecarga | Opcional |

## Estado y Errores

| Función | Descripción |
|---------|-------------|
| `herkulex_readStatus(id, statusError, detailError)` | Lee estado de error y detalle |
| `commandReadStatus(id)` | Lee e imprime estado (debug) |
| `commandClearError(id)` | Limpia registros de error |
| `printHerkulexErrors(statusError, detailError)` | Imprime descripción de errores |
| `revisarErroresMotores(ids[], cantidad)` | Revisa errores de múltiples servos |

## Control de Torque Adaptativo

| Función | Descripción |
|---------|-------------|
| `herkulex_monitorOverload(id, monitor)` | Monitorea PWM y detecta sobrecarga |
| `herkulex_adaptiveVelocityControl(id, monitor, baseVelocity)` | Ajusta velocidad basado en sobrecarga |

**Estructura `TorqueMonitor`:**
```cpp
struct TorqueMonitor {
  uint16_t pwmCurrent;
  uint16_t pwmMax;
  uint16_t pwmThreshold;
  bool overloadDetected;
  unsigned long lastCheck;
  float currentVelocity;
};
```

## Estimación de Torque y Corriente

| Función | Descripción | Retorno |
|---------|-------------|---------|
| `herkulex_estimateTorqueFromPWM(pwm, pwmMax=0)` | Estima torque en Nm | `float` |
| `herkulex_estimateCurrentFromPWM(pwm, pwmMax=0)` | Estima corriente en mA | `float` |
| `herkulex_getTorquePercentage(pwm, pwmMax=0)` | Obtiene porcentaje de torque (0-100%) | `float` |
| `herkulex_readTorque(id, torque, pwmMax=0)` | Lee PWM y calcula torque | `bool` |
| `herkulex_readCurrent(id, current, pwmMax=0)` | Lee PWM y calcula corriente | `bool` |
| `herkulex_getTorqueInfo(id, pwm, torque, current, percentage, pwmMax=0)` | Obtiene toda la información de torque/corriente | `bool` |

## Constantes Útiles

### Colores LED
- `LED_OFF`, `LED_GREEN`, `LED_BLUE`, `LED_RED`, `LED_PINK`, `LED_CYAN`, `LED_YELLOW`, `LED_WHITE`

### Valores por Defecto
- `HERKULEX_DEFAULT_VELOCITY_COUNTS_PER_S` = 250.0
- `HERKULEX_MIN_PLAYTIME_MS` = 200
- `HERKULEX_MAX_PLAYTIME_MS` = 2500
- `HERKULEX_PLAYTIME_UNIT_MS` = 11.2

### Límites PWM
- `HERKULEX_PWM_MAX_VALUE` = 1023
- `HERKULEX_PWM_SAFE_LIMIT` = 800
- `HERKULEX_PWM_CRITICAL_LIMIT` = 950

### Torque/Corriente
- `HERKULEX_TORQUE_MAX_NM` = 6.0
- `HERKULEX_CURRENT_MAX_MA` = 2000.0

## Valores de Retorno Especiales

- `readPosition()`: Retorna `0xFFFF` en caso de error
- Funciones `bool`: Retornan `false` en caso de error de comunicación o timeout
- Funciones `float`: Retornan valores calculados (0.0 si hay error en algunos casos)

## Notas Rápidas

- **EEPROM**: Cambios requieren `herkulex_reboot()` para aplicar
- **RAM**: Cambios se aplican inmediatamente
- **IDs válidos**: 0-253 (0xFE = broadcast)
- **Baudrate**: Por defecto 115200 baudios
- **Playtime**: 1 unidad = 11.2ms, rango 1-254

---

Para documentación completa con ejemplos, ver [API.md](API.md)

