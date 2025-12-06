# Análisis de la Librería HerkuleX DRS-0602

## Problemas Encontrados

### 1. **Formato de comando S_JOG incorrecto**

**Problema en `herkulex_moveTo()` (línea 202-214):**
```cpp
data[0] = playtime;                // ❌ INCORRECTO
data[1] = position & 0xFF;         
data[2] = (position >> 8) & 0xFF;  
data[3] = 0x00;                    // Info byte
data[4] = id;                      
```

**Según documentación (`packets_and_examples.md`):**
- S_JOG: `[playtime][JOG_LSB][JOG_MSB][SET][ID]` ✅
- El código actual está correcto en el orden, pero el byte SET está mal configurado

**Problema en `herkulex_sJogMultiMove()` (línea 385-405):**
- Usa `ID_BROADCAST = 0xFE` pero debería ser `0xFE` según documentación ✅
- El formato del byte SET está en `0x00` pero debería incluir flags apropiados

### 2. **Comando I_JOG no implementado**

- Solo existe `herkulex_moveTo()` que usa S_JOG
- Falta función para I_JOG (movimiento individual con tiempo por servo)
- I_JOG tiene formato diferente: `[JOG_LSB][JOG_MSB][SET][ID][playtime]`

### 3. **Byte SET mal configurado**

**Problema:** El byte SET (info) está siempre en `0x00`, pero debería incluir:
- Bit 0-2: LED (Green, Blue, Red)
- Bit 3: Modo (0=Posición, 1=Velocidad)
- Bit 4: Stop flag
- Bit 5: JogInvalid
- Bit 6: DisableVOR

**Ejemplo correcto según documentación:**
- Posición + LED verde: `0x04` (bit 2 = LED verde, modo 0 = posición)
- Velocidad + LED azul: `0x0A` (bit 1 = LED azul, bit 3 = modo velocidad)

### 4. **Funciones duplicadas**

- `commandReadStatus()` (línea 245) y `herkulex_readStatus()` (línea 137)
  - Ambas hacen lo mismo pero con diferentes interfaces
  - `commandReadStatus()` imprime directamente, `herkulex_readStatus()` retorna valores
  - Deberían consolidarse o documentar claramente la diferencia

### 5. **Inconsistencias en nombres de funciones**

- Mezcla de prefijos: `herkulex_*`, `command*`
- Algunas funciones tienen `herkulex_` prefix, otras `command*`
- Debería haber una convención consistente

### 6. **Problemas con lectura de respuestas**

**`readPosition()` (línea 114-135):**
- Asume tamaño fijo de respuesta (13 bytes)
- No valida checksum
- No sincroniza con header 0xFF 0xFF
- Timeout hardcodeado (1000ms)
- No maneja errores de comunicación correctamente

**`readPosition_debug()` (línea 471-566):**
- Implementación más robusta con sincronización
- Debería ser la base para `readPosition()`

### 7. **Cálculo de playtime incorrecto**

**`herkulex_safeMoveTo()` (línea 326-383):**
```cpp
uint8_t playtime = map(diff, 0, 10000, 10, 255); 
```
- Mapeo arbitrario sin base en documentación
- No considera velocidad deseada
- Valores hardcodeados sin justificación

### 8. **Funciones no utilizadas o incompletas**

- `moverAPoseInicial()` (línea 408-411): Función vacía/comentada
- `HerkulexMotor` struct (línea 302-305): Definido pero no usado
- `commandSetServoPolicy()` (línea 427-433): Usa dirección hardcodeada `0x03` en lugar de `ADDR_RAM_ACK_POLICY`

### 9. **Falta validación de checksum en respuestas**

- Ninguna función valida checksum de respuestas recibidas
- Riesgo de usar datos corruptos

### 10. **Timeouts y delays hardcodeados**

- `delay(5)`, `delay(10)`, `timeout = 1000` sin documentación
- Deberían ser constantes configurables

### 11. **Falta manejo de errores consistente**

- Algunas funciones retornan `0xFFFF` en error, otras `false`
- No hay códigos de error estandarizados

## Funciones Faltantes Importantes

### Lectura de registros
- [ ] `readVoltage()` - Leer voltaje (RAM 54)
- [ ] `readTemperature()` - Leer temperatura (RAM 55)
- [ ] `readPWM()` - Leer PWM actual (RAM 64-65)
- [ ] `readCurrentMode()` - Leer modo de control (RAM 56)
- [ ] `readAbsolutePosition()` - Leer posición absoluta (RAM 60-61)
- [ ] `readTargetPosition()` - Leer posición objetivo (RAM 68-69)
- [ ] `readSpeed()` - Leer velocidad deseada (RAM 72-73)

### Escritura de registros
- [ ] `setMaxPWM()` - Configurar PWM máximo (ROM/RAM 16-17)
- [ ] `setMinPWM()` - Configurar PWM mínimo (ROM/RAM 15)
- [ ] `setOverloadThreshold()` - Configurar umbral de sobrecarga (ROM/RAM 18-19)
- [ ] `setDeadZone()` - Configurar zona muerta (ROM/RAM 10)
- [ ] `setPositionMargin()` - Configurar margen de posición (ROM/RAM 44)
- [ ] `setAlarmPolicy()` - Configurar política de alarmas (ROM/RAM 2)
- [ ] `setBaudRate()` - Configurar velocidad de comunicación (ROM 4)

### Comandos
- [ ] `herkulex_iJog()` - Implementar comando I_JOG
- [ ] `herkulex_moveToWithSpeed()` - Movimiento con velocidad específica
- [ ] `herkulex_continuousRotation()` - Giro continuo (modo velocidad)
- [ ] `herkulex_stop()` - Detener movimiento (flag STOP)

### Control de torque adaptativo
- [ ] `readPWMCurrent()` - Leer PWM actual
- [ ] `adjustTorqueLimit()` - Ajustar límite de torque dinámicamente
- [ ] `monitorOverload()` - Monitorear sobrecarga
- [ ] `adaptiveTorqueControl()` - Control adaptativo basado en PWM

### Utilidades
- [ ] `validateChecksum()` - Validar checksum de respuesta
- [ ] `readPacket()` - Leer paquete con sincronización robusta
- [ ] `waitForResponse()` - Esperar respuesta con timeout configurable
- [ ] `flushSerial()` - Limpiar buffer serial (ya existe pero debería ser pública)

## Mejoras Sugeridas

### 1. **Organización del código**
- Separar en módulos:
  - `herkulex_packet.cpp` - Construcción y envío de paquetes
  - `herkulex_read.cpp` - Lectura de registros
  - `herkulex_write.cpp` - Escritura de registros
  - `herkulex_motion.cpp` - Comandos de movimiento
  - `herkulex_torque.cpp` - Control de torque

### 2. **Constantes configurables**
```cpp
#define HERKULEX_DEFAULT_TIMEOUT_MS 100
#define HERKULEX_RESPONSE_DELAY_MS 5
#define HERKULEX_COMMAND_DELAY_MS 10
```

### 3. **Códigos de error estandarizados**
```cpp
enum HerkulexError {
  HERKULEX_OK = 0,
  HERKULEX_TIMEOUT,
  HERKULEX_CHECKSUM_ERROR,
  HERKULEX_INVALID_RESPONSE,
  HERKULEX_COMMUNICATION_ERROR
};
```

### 4. **Estructura para control de torque adaptativo**
```cpp
struct TorqueMonitor {
  uint16_t pwmCurrent;
  uint16_t pwmMax;
  uint16_t pwmThreshold;
  bool overloadDetected;
  unsigned long lastCheck;
};
```

## Validación con Documentación

### ✅ Correcto
- Checksum calculation (XOR) - Coincide con documentación
- Estructura básica de paquetes - Correcta
- Comandos básicos (EEP_WRITE, RAM_WRITE, etc.) - Correctos
- Valores de torque control (0x60, 0x40, 0x00) - Correctos

### ❌ Incorrecto o Incompleto
- Formato de byte SET en comandos JOG
- Falta implementación de I_JOG
- Lectura de respuestas sin validación robusta
- Cálculo de playtime sin base documentada

## Prioridades de Corrección

1. **ALTA**: Corregir formato de byte SET en comandos JOG
2. **ALTA**: Implementar I_JOG correctamente
3. **ALTA**: Mejorar lectura de respuestas con validación
4. **MEDIA**: Agregar funciones de lectura faltantes
5. **MEDIA**: Implementar control de torque adaptativo
6. **BAJA**: Refactorizar organización del código
7. **BAJA**: Estandarizar nombres y convenciones

