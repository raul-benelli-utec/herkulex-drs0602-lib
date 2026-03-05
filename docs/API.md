# API Reference - HerkuleX DRS-0602 Library

Documentación completa de la API pública de la librería para controlar servomotores HerkuleX DRS-0602.

## Tabla de Contenidos

- [Funciones Básicas](#funciones-básicas)
- [Control de Torque](#control-de-torque)
- [Comandos de Movimiento](#comandos-de-movimiento)
- [Lectura de Registros](#lectura-de-registros)
- [Escritura de Registros](#escritura-de-registros)
- [Estado y Errores](#estado-y-errores)
- [Control de Torque Adaptativo](#control-de-torque-adaptativo)
- [Estimación de Torque y Corriente](#estimación-de-torque-y-corriente)
- [Ejemplos de Uso](#ejemplos-de-uso)

---

## Funciones Básicas

### `herkulex_ledSet(byte id, byte color)`

Controla el LED del servo.

**Parámetros:**
- `id`: ID del servo (0-253)
- `color`: Color del LED. Valores disponibles:
  - `LED_OFF` (0x00): LED apagado
  - `LED_GREEN` (0x01): LED verde
  - `LED_BLUE` (0x02): LED azul
  - `LED_RED` (0x04): LED rojo
  - `LED_PINK` (0x05): LED rosa (verde + rojo)
  - `LED_CYAN` (0x03): LED cian (verde + azul)
  - `LED_YELLOW` (0x06): LED amarillo (azul + rojo)
  - `LED_WHITE` (0x07): LED blanco (todos los colores)

**Retorno:** Ninguno

**Notas:** Los colores pueden combinarse usando OR bitwise (ej: `LED_GREEN | LED_RED`).

---

### `herkulex_ledOff(byte id)`

Apaga el LED del servo.

**Parámetros:**
- `id`: ID del servo (0-253)

**Retorno:** Ninguno

---

### `herkulex_reboot(byte id)`

Reinicia el servo. Necesario después de escribir en registros EEPROM.

**Parámetros:**
- `id`: ID del servo (0-253)

**Retorno:** Ninguno

**Notas:** Después de escribir en EEPROM, siempre se debe llamar a esta función para aplicar los cambios.

---

## Control de Torque

### `commandEnableTorque(byte id)`

Activa el torque del servo. El servo puede seguir comandos de movimiento.

**Parámetros:**
- `id`: ID del servo (0-253)

**Retorno:** Ninguno

**Notas:** Los comandos de movimiento (I_JOG, S_JOG) solo funcionan con torque activado.

---

### `commandTorqueFree(byte id)`

Desactiva el torque. El eje queda libre y puede moverse manualmente.

**Parámetros:**
- `id`: ID del servo (0-253)

**Retorno:** Ninguno

---

### `commandBrakeOn(byte id)`

Activa el freno eléctrico. El servo mantiene su posición pero no ejecuta comandos de movimiento.

**Parámetros:**
- `id`: ID del servo (0-253)

**Retorno:** Ninguno

---

## Comandos de Movimiento

### `uint8_t herkulex_computePlaytime(uint16_t current, uint16_t goal, float v_counts_per_s)`

Calcula el tiempo de movimiento (playtime) basado en la distancia y velocidad objetivo.

**Parámetros:**
- `current`: Posición actual en cuentas
- `goal`: Posición objetivo en cuentas
- `v_counts_per_s`: Velocidad objetivo en cuentas por segundo (recomendado: 200-300)

**Retorno:** 
- `uint8_t`: Playtime en unidades de 11.2ms (1-254)

**Notas:**
- El tiempo mínimo es 200ms, máximo 2.5s
- Si `v_counts_per_s` es 0, se usa el valor por defecto (250 cuentas/s)

---

### `void herkulex_moveTo(byte id, uint16_t position, byte playtime, byte ledFlags = 0)`

Mueve el servo a una posición específica usando el comando S_JOG (sincronizado).

**Parámetros:**
- `id`: ID del servo (0-253)
- `position`: Posición objetivo en cuentas (0-32767)
- `playtime`: Tiempo de movimiento en unidades de 11.2ms (1-254)
- `ledFlags`: Flags de LED opcionales (por defecto: sin LED)

**Retorno:** Ninguno

**Notas:**
- Usa modo de control de posición
- El playtime puede calcularse con `herkulex_computePlaytime()`

---

### `void herkulex_iJog(byte id, uint16_t position, byte playtime, byte ledFlags = 0)`

Mueve el servo a una posición usando el comando I_JOG (individual). Útil para controlar múltiples servos con tiempos diferentes.

**Parámetros:**
- `id`: ID del servo (0-253)
- `position`: Posición objetivo en cuentas (0-32767)
- `playtime`: Tiempo de movimiento en unidades de 11.2ms (1-254)
- `ledFlags`: Flags de LED opcionales (por defecto: sin LED)

**Retorno:** Ninguno

**Notas:**
- Similar a `herkulex_moveTo()` pero con formato de comando diferente
- Permite controlar múltiples servos con tiempos individuales

---

### `void herkulex_continuousRotation(byte id, int16_t speed, byte playtime, byte ledFlags = 0)`

Giro continuo del servo (modo velocidad). El servo rota indefinidamente a la velocidad especificada.

**Parámetros:**
- `id`: ID del servo (0-253)
- `speed`: Velocidad en cuentas (-1023 a 1023). El signo indica dirección
- `playtime`: Tiempo de movimiento en unidades de 11.2ms (1-254)
- `ledFlags`: Flags de LED opcionales (por defecto: sin LED)

**Retorno:** Ninguno

**Notas:**
- Velocidad positiva: sentido horario, negativa: sentido antihorario
- El servo continuará girando hasta recibir otro comando

---

### `void herkulex_safeMoveTo(byte id, uint16_t targetPosition, float velocity = 0)`

Mueve el servo de forma segura con cálculo automático de playtime y validación de límites.

**Parámetros:**
- `id`: ID del servo (0-253)
- `targetPosition`: Posición objetivo en cuentas
- `velocity`: Velocidad objetivo en cuentas por segundo (0 = usar valor por defecto: 250)

**Retorno:** Ninguno

**Errores:**
- Si la posición está fuera del rango permitido, el movimiento se cancela
- Si no se puede leer la posición actual, el movimiento se cancela

**Notas:**
- Valida que la posición esté dentro de los límites configurados
- Calcula automáticamente el playtime basado en la distancia
- Imprime información de debug por Serial

---

### `void herkulex_sJogMultiMove(byte ids[], uint16_t posiciones[], byte cantidad, byte playtime, byte ledFlags = 0)`

Mueve múltiples servos de forma sincronizada usando S_JOG.

**Parámetros:**
- `ids`: Array con los IDs de los servos
- `posiciones`: Array con las posiciones objetivo para cada servo
- `cantidad`: Número de servos a mover
- `playtime`: Tiempo compartido en unidades de 11.2ms (1-254)
- `ledFlags`: Flags de LED para todos los servos (por defecto: sin LED)

**Retorno:** Ninguno

**Notas:**
- Todos los servos se mueven con el mismo tiempo
- Máximo 53 servos según documentación
- Usa ID de broadcast (0xFE) para enviar a todos simultáneamente

---

## Lectura de Registros

### `uint16_t readPosition(byte id)`

Lee la posición calibrada actual del servo.

**Parámetros:**
- `id`: ID del servo (0-253)

**Retorno:**
- `uint16_t`: Posición en cuentas (0-32767)
- `0xFFFF`: Error al leer (timeout o comunicación fallida)

**Notas:**
- La posición calibrada incluye el offset de calibración
- Usa el registro RAM 58-59 (POS_CALIB)

---

### `bool herkulex_readAbsolutePosition(byte id, uint16_t& position)`

Lee la posición absoluta sin calibrar del servo.

**Parámetros:**
- `id`: ID del servo (0-253)
- `position`: Variable donde se guardará la posición (referencia)

**Retorno:**
- `true`: Lectura exitosa
- `false`: Error de comunicación o timeout

**Notas:**
- Posición sin aplicar offset de calibración
- Usa el registro RAM 60-61 (POS_ABS)

---

### `bool herkulex_readTargetPosition(byte id, uint16_t& position)`

Lee la posición objetivo actual del servo.

**Parámetros:**
- `id`: ID del servo (0-253)
- `position`: Variable donde se guardará la posición objetivo (referencia)

**Retorno:**
- `true`: Lectura exitosa
- `false`: Error de comunicación o timeout

---

### `bool herkulex_readVoltage(byte id, byte& voltage)`

Lee el voltaje de alimentación del servo.

**Parámetros:**
- `id`: ID del servo (0-253)
- `voltage`: Variable donde se guardará el voltaje (referencia)

**Retorno:**
- `true`: Lectura exitosa
- `false`: Error de comunicación o timeout

**Notas:**
- El voltaje está en décimas de voltio (ej: 120 = 12.0V)
- Rango típico: 92-200 (9.2V - 20.0V)

---

### `bool herkulex_readTemperature(byte id, byte& temperature)`

Lee la temperatura actual del servo.

**Parámetros:**
- `id`: ID del servo (0-253)
- `temperature`: Variable donde se guardará la temperatura (referencia)

**Retorno:**
- `true`: Lectura exitosa
- `false`: Error de comunicación o timeout

**Notas:**
- Temperatura en grados Celsius
- Rango típico: 0-110°C

---

### `bool herkulex_readPWM(byte id, uint16_t& pwm)`

Lee el valor de PWM actual del servo.

**Parámetros:**
- `id`: ID del servo (0-253)
- `pwm`: Variable donde se guardará el PWM (referencia)

**Retorno:**
- `true`: Lectura exitosa
- `false`: Error de comunicación o timeout

**Notas:**
- PWM en rango 0-1023
- El PWM es proporcional a la corriente y torque

---

### `bool herkulex_readCurrentMode(byte id, byte& mode)`

Lee el modo de control actual del servo.

**Parámetros:**
- `id`: ID del servo (0-253)
- `mode`: Variable donde se guardará el modo (referencia)

**Retorno:**
- `true`: Lectura exitosa
- `false`: Error de comunicación o timeout

**Valores de modo:**
- `0`: Control de posición
- `1`: Control de velocidad (giro continuo)

---

### `bool herkulex_readSpeed(byte id, uint16_t& speed)`

Lee la velocidad deseada actual del servo.

**Parámetros:**
- `id`: ID del servo (0-253)
- `speed`: Variable donde se guardará la velocidad (referencia)

**Retorno:**
- `true`: Lectura exitosa
- `false`: Error de comunicación o timeout

**Notas:**
- Velocidad en cuentas por segundo
- 1 unidad ≈ 0.034 grados/s

---

## Escritura de Registros

### `void commandSetID(byte old_id, byte new_id)`

Cambia el ID del servo. Requiere REBOOT para aplicar.

**Parámetros:**
- `old_id`: ID actual del servo
- `new_id`: Nuevo ID (0-253)

**Retorno:** Ninguno

**Notas:**
- Escribe en EEPROM, requiere `herkulex_reboot()` después
- No puede haber dos servos con el mismo ID

---

### `void commandSetMaxPosition(byte id, uint16_t pos)`

Configura la posición máxima permitida. Requiere REBOOT para aplicar.

**Parámetros:**
- `id`: ID del servo (0-253)
- `pos`: Posición máxima en cuentas (0-32767)

**Retorno:** Ninguno

**Notas:**
- Escribe en EEPROM, requiere `herkulex_reboot()` después

---

### `void commandSetMinPosition(byte id, uint16_t pos)`

Configura la posición mínima permitida. Requiere REBOOT para aplicar.

**Parámetros:**
- `id`: ID del servo (0-253)
- `pos`: Posición mínima en cuentas (0-32767)

**Retorno:** Ninguno

**Notas:**
- Escribe en EEPROM, requiere `herkulex_reboot()` después

---

### `void commandSetPositionLimits(byte id, uint16_t min, uint16_t max)`

Configura los límites de posición (mínimo y máximo). Requiere REBOOT para aplicar.

**Parámetros:**
- `id`: ID del servo (0-253)
- `min`: Posición mínima en cuentas
- `max`: Posición máxima en cuentas

**Retorno:** Ninguno

**Notas:**
- Escribe en EEPROM, requiere `herkulex_reboot()` después
- Llama internamente a `commandSetMinPosition()` y `commandSetMaxPosition()`

---

### `void commandSetCalibrationDiff(byte id, int16_t offset)`

Configura el offset de calibración. Requiere REBOOT para aplicar.

**Parámetros:**
- `id`: ID del servo (0-253)
- `offset`: Offset de calibración en cuentas (-1495 a 1495, aproximadamente ±40°)

**Retorno:** Ninguno

**Notas:**
- Escribe en EEPROM, requiere `herkulex_reboot()` después
- Se usa para calibrar el punto cero del servo

---

### `void commandSetAcceleration(byte id, byte accelRatio, byte accelTime)`

Configura el perfil de aceleración del servo. Aplica inmediatamente.

**Parámetros:**
- `id`: ID del servo (0-253)
- `accelRatio`: Relación de aceleración en porcentaje (0-50)
- `accelTime`: Tiempo máximo de aceleración en unidades de 11.2ms (0-254)

**Retorno:** Ninguno

**Notas:**
- Escribe en RAM, aplica inmediatamente
- `accelRatio = 0`: Perfil rectangular (sin aceleración suave)
- `accelRatio > 0`: Perfil trapezoidal

---

### `void commandSetServoPolicy(byte id, byte policy)`

Configura la política de respuesta ACK del servo.

**Parámetros:**
- `id`: ID del servo (0-253)
- `policy`: Política de respuesta (0-2)
  - `0`: No responder
  - `1`: Responder solo a comandos de lectura
  - `2`: Responder a todos los comandos

**Retorno:** Ninguno

---

### `void herkulex_setMaxPWM(byte id, uint16_t maxPWM, bool writeToEEP = false)`

Configura el límite máximo de PWM (torque máximo).

**Parámetros:**
- `id`: ID del servo (0-253)
- `maxPWM`: PWM máximo (0-1023)
- `writeToEEP`: Si es `true`, escribe en EEPROM (requiere REBOOT). Si es `false`, escribe en RAM (aplica inmediatamente)

**Retorno:** Ninguno

**Notas:**
- Reducir el PWM máximo limita el torque máximo del servo
- Útil para protección y optimización de energía

---

### `void herkulex_setMinPWM(byte id, byte minPWM, bool writeToEEP = false)`

Configura el límite mínimo de PWM (torque mínimo).

**Parámetros:**
- `id`: ID del servo (0-253)
- `minPWM`: PWM mínimo (0-254)
- `writeToEEP`: Si es `true`, escribe en EEPROM (requiere REBOOT). Si es `false`, escribe en RAM (aplica inmediatamente)

**Retorno:** Ninguno

---

### `void herkulex_setOverloadThreshold(byte id, uint16_t threshold, bool writeToEEP = false)`

Configura el umbral de detección de sobrecarga.

**Parámetros:**
- `id`: ID del servo (0-253)
- `threshold`: Umbral de PWM para sobrecarga (0-1023)
- `writeToEEP`: Si es `true`, escribe en EEPROM (requiere REBOOT). Si es `false`, escribe en RAM (aplica inmediatamente)

**Retorno:** Ninguno

**Notas:**
- Si el PWM supera este umbral durante el período de detección, se considera sobrecarga

---

## Estado y Errores

### `bool herkulex_readStatus(byte id, byte& statusError, byte& detailError)`

Lee el estado de error y detalle del servo.

**Parámetros:**
- `id`: ID del servo (0-253)
- `statusError`: Variable donde se guardará el código de error (referencia)
- `detailError`: Variable donde se guardará el detalle de error (referencia)

**Retorno:**
- `true`: Lectura exitosa
- `false`: Error de comunicación o timeout

**Códigos de error (statusError):**
- `0x01`: Voltaje fuera de rango
- `0x02`: Límite de posición superado
- `0x04`: Temperatura excedida
- `0x08`: Sobrecarga detectada
- `0x10`: Fallo del controlador (no aplicable en DRS-0602)
- `0x20`: Error en memoria EEPROM

**Notas:**
- El comando STAT siempre responde, independientemente de la política ACK

---

### `void commandReadStatus(uint8_t id)`

Lee e imprime el estado del servo directamente por Serial.

**Parámetros:**
- `id`: ID del servo (0-253)

**Retorno:** Ninguno

**Notas:**
- Versión de debug que imprime directamente
- Usa `Serial.print()` para mostrar el estado

---

### `void commandClearError(byte id)`

Limpia los registros de error del servo.

**Parámetros:**
- `id`: ID del servo (0-253)

**Retorno:** Ninguno

**Notas:**
- Establece STATUS_ERROR y STATUS_DETAIL a 0x00

---

### `void printHerkulexErrors(byte statusError, byte detailError)`

Imprime una descripción legible de los errores.

**Parámetros:**
- `statusError`: Código de error
- `detailError`: Detalle de error

**Retorno:** Ninguno

**Notas:**
- Función de utilidad para debugging
- Imprime descripciones en español

---

### `void revisarErroresMotores(byte ids[], byte cantidad)`

Revisa los errores de múltiples servos.

**Parámetros:**
- `ids`: Array con los IDs de los servos
- `cantidad`: Número de servos a revisar

**Retorno:** Ninguno

**Notas:**
- Imprime el estado de cada servo por Serial

---

## Control de Torque Adaptativo

### Estructura `TorqueMonitor`

Estructura para monitorear el estado del torque del servo.

```cpp
struct TorqueMonitor {
  uint16_t pwmCurrent;        // PWM actual leído
  uint16_t pwmMax;            // PWM máximo configurado
  uint16_t pwmThreshold;      // Umbral de sobrecarga
  bool overloadDetected;      // Flag de sobrecarga detectada
  unsigned long lastCheck;    // Última vez que se verificó
  float currentVelocity;      // Velocidad actual ajustada
};
```

---

### `bool herkulex_monitorOverload(byte id, TorqueMonitor& monitor)`

Monitorea el PWM del servo y detecta sobrecarga.

**Parámetros:**
- `id`: ID del servo (0-253)
- `monitor`: Estructura de monitoreo (referencia, se actualiza)

**Retorno:**
- `true`: Sobrecarga detectada
- `false`: Sin sobrecarga o error de lectura

**Notas:**
- Actualiza `monitor.pwmCurrent` y `monitor.overloadDetected`
- Compara con `monitor.pwmThreshold`

---

### `float herkulex_adaptiveVelocityControl(byte id, TorqueMonitor& monitor, float baseVelocity)`

Ajusta la velocidad dinámicamente basándose en el estado de sobrecarga.

**Parámetros:**
- `id`: ID del servo (0-253)
- `monitor`: Estructura de monitoreo (referencia, se actualiza)
- `baseVelocity`: Velocidad base en cuentas por segundo

**Retorno:**
- `float`: Velocidad ajustada en cuentas por segundo

**Notas:**
- Si hay sobrecarga, reduce la velocidad en 20%
- Velocidad mínima: 50 cuentas/s

---

## Estimación de Torque y Corriente

### `float herkulex_estimateTorqueFromPWM(uint16_t pwm, uint16_t pwmMax = 0)`

Estima el torque en Nm basado en el valor de PWM.

**Parámetros:**
- `pwm`: Valor de PWM (0-1023)
- `pwmMax`: PWM máximo configurado (0 = usar 1023 por defecto)

**Retorno:**
- `float`: Torque estimado en Nm

**Notas:**
- Fórmula: `Torque = (PWM / PWM_MAX) * TORQUE_MAX`
- `TORQUE_MAX` por defecto: 6.0 Nm (configurable en `herkulex_defs.h`)

---

### `float herkulex_estimateCurrentFromPWM(uint16_t pwm, uint16_t pwmMax = 0)`

Estima la corriente en mA basada en el valor de PWM.

**Parámetros:**
- `pwm`: Valor de PWM (0-1023)
- `pwmMax`: PWM máximo configurado (0 = usar 1023 por defecto)

**Retorno:**
- `float`: Corriente estimada en mA

**Notas:**
- Fórmula: `Corriente = (PWM / PWM_MAX) * CURRENT_MAX`
- `CURRENT_MAX` por defecto: 2000 mA (configurable en `herkulex_defs.h`)

---

### `float herkulex_getTorquePercentage(uint16_t pwm, uint16_t pwmMax = 0)`

Obtiene el porcentaje de torque actual.

**Parámetros:**
- `pwm`: Valor de PWM (0-1023)
- `pwmMax`: PWM máximo configurado (0 = usar 1023 por defecto)

**Retorno:**
- `float`: Porcentaje de torque (0.0 - 100.0)

---

### `bool herkulex_readTorque(byte id, float& torque, uint16_t pwmMax = 0)`

Lee el PWM del servo y calcula el torque estimado.

**Parámetros:**
- `id`: ID del servo (0-253)
- `torque`: Variable donde se guardará el torque en Nm (referencia)
- `pwmMax`: PWM máximo configurado (0 = usar 1023 por defecto)

**Retorno:**
- `true`: Lectura exitosa
- `false`: Error de comunicación o timeout

---

### `bool herkulex_readCurrent(byte id, float& current, uint16_t pwmMax = 0)`

Lee el PWM del servo y calcula la corriente estimada.

**Parámetros:**
- `id`: ID del servo (0-253)
- `current`: Variable donde se guardará la corriente en mA (referencia)
- `pwmMax`: PWM máximo configurado (0 = usar 1023 por defecto)

**Retorno:**
- `true`: Lectura exitosa
- `false`: Error de comunicación o timeout

---

### `bool herkulex_getTorqueInfo(byte id, uint16_t& pwm, float& torque, float& current, float& percentage, uint16_t pwmMax = 0)`

Obtiene información completa de torque y corriente del servo.

**Parámetros:**
- `id`: ID del servo (0-253)
- `pwm`: Variable donde se guardará el PWM (referencia)
- `torque`: Variable donde se guardará el torque en Nm (referencia)
- `current`: Variable donde se guardará la corriente en mA (referencia)
- `percentage`: Variable donde se guardará el porcentaje de torque (referencia)
- `pwmMax`: PWM máximo configurado (0 = usar 1023 por defecto)

**Retorno:**
- `true`: Lectura exitosa
- `false`: Error de comunicación o timeout

---

## Ejemplos de Uso

### Ejemplo 1: Inicialización, Ping y Lectura de Posición

```cpp
#include "herkulex_utils.h"

// Configurar Serial1 para comunicación con servos
// Baudrate: 115200 (o el configurado en el servo)

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);  // Puerto de comunicación con servos
  
  delay(1000);
  Serial.println("Inicializando servos HerkuleX DRS-0602");
  
  byte servoID = 1;  // ID del servo a probar
  
  // Ping: Intentar leer posición (si responde, el servo está conectado)
  Serial.print("Haciendo ping al servo ID ");
  Serial.println(servoID);
  
  uint16_t position = readPosition(servoID);
  
  if (position != 0xFFFF) {
    Serial.print("✓ Servo respondió. Posición actual: ");
    Serial.println(position);
  } else {
    Serial.println("✗ Error: Servo no respondió o timeout");
    return;
  }
  
  // Leer información adicional
  byte voltage, temperature;
  uint16_t pwm;
  
  if (herkulex_readVoltage(servoID, voltage)) {
    Serial.print("Voltaje: ");
    Serial.print(voltage / 10.0);
    Serial.println(" V");
  }
  
  if (herkulex_readTemperature(servoID, temperature)) {
    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }
  
  if (herkulex_readPWM(servoID, pwm)) {
    Serial.print("PWM actual: ");
    Serial.println(pwm);
  }
}

void loop() {
  // Nada en el loop para este ejemplo
}
```

### Ejemplo 2: Torque ON, Movimiento Pequeño, Lectura y Torque OFF

```cpp
#include "herkulex_utils.h"

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  
  delay(1000);
  Serial.println("Ejemplo: Control de torque y movimiento");
  
  byte servoID = 1;
  
  // 1. Leer posición inicial
  uint16_t posInicial = readPosition(servoID);
  if (posInicial == 0xFFFF) {
    Serial.println("Error: No se pudo leer posición inicial");
    return;
  }
  Serial.print("Posición inicial: ");
  Serial.println(posInicial);
  
  // 2. Activar torque
  Serial.println("Activando torque...");
  commandEnableTorque(servoID);
  delay(100);  // Dar tiempo para que se active
  
  // 3. Leer posición antes del movimiento
  uint16_t posAntes = readPosition(servoID);
  Serial.print("Posición antes del movimiento: ");
  Serial.println(posAntes);
  
  // 4. Calcular posición objetivo (mover 500 cuentas)
  uint16_t posObjetivo = posAntes + 500;
  if (posObjetivo > 32767) posObjetivo = posAntes - 500;  // Si excede, mover hacia atrás
  
  // 5. Calcular playtime para movimiento suave
  float velocidad = 200.0f;  // 200 cuentas por segundo
  uint8_t playtime = herkulex_computePlaytime(posAntes, posObjetivo, velocidad);
  
  Serial.print("Moviendo a posición: ");
  Serial.print(posObjetivo);
  Serial.print(" con playtime: ");
  Serial.print(playtime);
  Serial.print(" (");
  Serial.print(playtime * 11.2);
  Serial.println(" ms)");
  
  // 6. Ejecutar movimiento
  herkulex_moveTo(servoID, posObjetivo, playtime, LED_GREEN);
  
  // 7. Esperar a que termine el movimiento
  delay(playtime * 11.2 + 100);  // Tiempo del movimiento + margen
  
  // 8. Leer posición después del movimiento
  uint16_t posDespues = readPosition(servoID);
  Serial.print("Posición después del movimiento: ");
  Serial.println(posDespues);
  
  // 9. Leer información de torque/corriente
  float torque, current, percentage;
  uint16_t pwm;
  if (herkulex_getTorqueInfo(servoID, pwm, torque, current, percentage)) {
    Serial.print("PWM: ");
    Serial.print(pwm);
    Serial.print(" | Torque estimado: ");
    Serial.print(torque);
    Serial.print(" Nm | Corriente estimada: ");
    Serial.print(current);
    Serial.print(" mA | Porcentaje: ");
    Serial.print(percentage);
    Serial.println("%");
  }
  
  // 10. Desactivar torque
  Serial.println("Desactivando torque...");
  commandTorqueFree(servoID);
  
  Serial.println("Ejemplo completado");
}

void loop() {
  // Nada en el loop
}
```

---

## Notas Importantes

1. **Configuración Serial**: Asegúrate de configurar `Serial1` con el baudrate correcto (por defecto 115200 baudios).

2. **Delays**: Algunas operaciones requieren pequeños delays para asegurar que el servo procese el comando.

3. **EEPROM vs RAM**: 
   - Cambios en EEPROM requieren `herkulex_reboot()` para aplicar
   - Cambios en RAM se aplican inmediatamente

4. **Timeouts**: Las funciones de lectura tienen timeouts configurables. Si hay problemas de comunicación, verifica la conexión y el baudrate.

5. **IDs**: Los IDs válidos son 0-253. El ID 0xFE es para broadcast (todos los servos).

6. **Valores de Retorno**: Las funciones que retornan `bool` indican éxito/fallo. Las que retornan valores numéricos usan valores especiales para indicar error (ej: `0xFFFF` para `readPosition()`).

