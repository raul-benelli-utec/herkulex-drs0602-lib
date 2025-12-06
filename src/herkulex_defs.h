#ifndef HERKULEX_DEFS_H
#define HERKULEX_DEFS_H

// ============================================================================
// Broadcast ID
// ============================================================================
#define HERKULEX_BROADCAST_ID 0xFD

// ============================================================================
// REGISTROS ROM (EEP) - Direcciones para uso con CMD_EEP_READ/CMD_EEP_WRITE
// ============================================================================

// Identificación del modelo
#define ADDR_ROM_MODEL_NO1         0x00  // Dígito alto del número de modelo         // RO
#define ADDR_ROM_MODEL_NO2         0x01  // Dígito bajo del número de modelo         // RO
#define ADDR_ROM_VERSION1          0x02  // Versión mayor del firmware               // RO
#define ADDR_ROM_VERSION2          0x03  // Versión menor del firmware               // RO
#define ADDR_ROM_BAUD_RATE         0x04  // Velocidad de comunicación serial         // RW
// Reservado: 0x05
#define ADDR_ROM_SERVO_ID          0x06  // ID del servo (0x00–0xFD)                 // RW
#define ADDR_ROM_ACK_POLICY        0x07  // Política de respuesta (0–2)              // RW
#define ADDR_ROM_ALARM_POLICY      0x08  // Política de alarmas                      // RW
#define ADDR_ROM_TORQUE_POLICY     0x09  // Política de torque y errores             // RW
// Reservado: 0x0A
#define ADDR_ROM_MAX_TEMP          0x0B  // Temperatura máxima [°C]                  // RW
#define ADDR_ROM_MIN_VOLTAGE       0x0C  // Voltaje mínimo (×0.1 V)                  // RW
#define ADDR_ROM_MAX_VOLTAGE       0x0D  // Voltaje máximo (×0.1 V)                  // RW
#define ADDR_ROM_ACCEL_RATIO       0x0E  // Relación de aceleración (%)              // RW
#define ADDR_ROM_MAX_ACCEL_TIME    0x0F  // Tiempo máx de aceleración (×11.2ms)      // RW
#define ADDR_ROM_DEAD_ZONE         0x10  // Zona muerta (solo en control de posición) // RW
#define ADDR_ROM_SAT_OFFSET        0x11  // Offset de saturación                     // RW
#define ADDR_ROM_SAT_SLOPE_L       0x12  // Pendiente de saturación (LSB)            // RW
#define ADDR_ROM_SAT_SLOPE_H       0x13  // Pendiente de saturación (MSB)            // RW
#define ADDR_ROM_PWM_OFFSET        0x14  // Compensación PWM                         // RW
#define ADDR_ROM_PWM_MIN           0x15  // PWM mínimo permitido                     // RW
#define ADDR_ROM_PWM_MAX_L         0x16  // PWM máximo (LSB)                         // RW
#define ADDR_ROM_PWM_MAX_H         0x17  // PWM máximo (MSB)                         // RW
#define ADDR_ROM_PWM_OVERLOAD_L    0x18  // Umbral sobrecarga PWM (LSB)              // RW
#define ADDR_ROM_PWM_OVERLOAD_H    0x19  // Umbral sobrecarga PWM (MSB)              // RW
#define ADDR_ROM_POS_MIN_L         0x1A  // Posición mínima permitida (LSB)          // RW
#define ADDR_ROM_POS_MIN_H         0x1B  // Posición mínima permitida (MSB)          // RW
#define ADDR_ROM_POS_MAX_L         0x1C  // Posición máxima permitida (LSB)          // RW
#define ADDR_ROM_POS_MAX_H         0x1D  // Posición máxima permitida (MSB)          // RW
#define ADDR_ROM_KP_POS_L          0x1E  // Ganancia P de posición (LSB)             // RW
#define ADDR_ROM_KP_POS_H          0x1F  // Ganancia P de posición (MSB)             // RW
#define ADDR_ROM_KD_POS_L          0x20  // Ganancia D de posición (LSB)             // RW
#define ADDR_ROM_KD_POS_H          0x21  // Ganancia D de posición (MSB)             // RW
#define ADDR_ROM_KI_POS_L          0x22  // Ganancia I de posición (LSB)             // RW
#define ADDR_ROM_KI_POS_H          0x23  // Ganancia I de posición (MSB)             // RW
#define ADDR_ROM_FEEDFORWARD1_L    0x24  // Avance 1ª ganancia (LSB)                 // RW
#define ADDR_ROM_FEEDFORWARD1_H    0x25  // Avance 1ª ganancia (MSB)                 // RW
#define ADDR_ROM_FEEDFORWARD2_L    0x26  // Avance 2ª ganancia (LSB)                 // RW
#define ADDR_ROM_FEEDFORWARD2_H    0x27  // Avance 2ª ganancia (MSB)                 // RW
#define ADDR_ROM_KP_SPEED_L        0x28  // Ganancia P de velocidad (LSB)            // RW
#define ADDR_ROM_KP_SPEED_H        0x29  // Ganancia P de velocidad (MSB)            // RW
#define ADDR_ROM_KI_SPEED_L        0x2A  // Ganancia I de velocidad (LSB)            // RW
#define ADDR_ROM_KI_SPEED_H        0x2B  // Ganancia I de velocidad (MSB)            // RW
#define ADDR_ROM_LED_BLINK_PERIOD  0x2C  // Período de parpadeo LED (×11.2ms)        // RW
#define ADDR_ROM_ADC_ERROR_PERIOD  0x2D  // Período de check de error ADC (×11.2ms)  // RW
#define ADDR_ROM_GARBAGE_CHECK_PERIOD 0x2E  // Período de check de paquetes basura (×11.2ms) // RW
#define ADDR_ROM_STOP_DETECT_PERIOD 0x2F  // Período de detección de parada (×11.2ms) // RW
#define ADDR_ROM_OVERLOAD_DETECT_PERIOD 0x30  // Período de detección de sobrecarga (×4ms) // RW
#define ADDR_ROM_STOP_THRESHOLD    0x31  // Umbral de parada                         // RW
#define ADDR_ROM_POS_MARGIN        0x32  // Margen de posición                       // RW
// Reservado: 0x33
#define ADDR_ROM_CALIB_DIFF_L      0x34  // Calibración diferencial (LSB)            // RW
#define ADDR_ROM_CALIB_DIFF_H      0x35  // Calibración diferencial (MSB)            // RW

// ============================================================================
// REGISTROS RAM - Direcciones para uso con CMD_RAM_READ/CMD_RAM_WRITE
// ============================================================================

// Configuración básica
#define ADDR_RAM_SERVO_ID          0x00  // ID del servo (0x00–0xFD)                 // RW
#define ADDR_RAM_ACK_POLICY        0x01  // Política de respuesta (0–2)              // RW
#define ADDR_RAM_ALARM_POLICY      0x02  // Política de alarmas                      // RW
#define ADDR_RAM_TORQUE_POLICY     0x03  // Política de torque y errores             // RW
// Reservado: 0x04
#define ADDR_RAM_MAX_TEMP          0x05  // Temperatura máxima [°C]                  // RW
#define ADDR_RAM_MIN_VOLTAGE       0x06  // Voltaje mínimo (×0.1 V)                  // RW
#define ADDR_RAM_MAX_VOLTAGE       0x07  // Voltaje máximo (×0.1 V)                  // RW
#define ADDR_RAM_ACCEL_RATIO       0x08  // Relación de aceleración (%)              // RW
#define ADDR_RAM_MAX_ACCEL_TIME    0x09  // Tiempo máx de aceleración (×11.2ms)      // RW
#define ADDR_RAM_DEAD_ZONE         0x0A  // Zona muerta (solo en control de posición) // RW
#define ADDR_RAM_SAT_OFFSET        0x0B  // Offset de saturación                     // RW
#define ADDR_RAM_SAT_SLOPE_L       0x0C  // Pendiente de saturación (LSB)            // RW
#define ADDR_RAM_SAT_SLOPE_H       0x0D  // Pendiente de saturación (MSB)            // RW
#define ADDR_RAM_PWM_OFFSET        0x0E  // Compensación PWM                         // RW
#define ADDR_RAM_PWM_MIN           0x0F  // PWM mínimo permitido                     // RW
#define ADDR_RAM_PWM_MAX_L         0x10  // PWM máximo (LSB)                         // RW
#define ADDR_RAM_PWM_MAX_H         0x11  // PWM máximo (MSB)                         // RW
#define ADDR_RAM_PWM_OVERLOAD_L    0x12  // Umbral sobrecarga PWM (LSB)              // RW
#define ADDR_RAM_PWM_OVERLOAD_H    0x13  // Umbral sobrecarga PWM (MSB)              // RW
#define ADDR_RAM_POS_MIN_L         0x14  // Posición mínima permitida (LSB)          // RW
#define ADDR_RAM_POS_MIN_H         0x15  // Posición mínima permitida (MSB)          // RW
#define ADDR_RAM_POS_MAX_L         0x16  // Posición máxima permitida (LSB)          // RW
#define ADDR_RAM_POS_MAX_H         0x17  // Posición máxima permitida (MSB)          // RW
#define ADDR_RAM_KP_POS_L          0x18  // Ganancia P de posición (LSB)             // RW
#define ADDR_RAM_KP_POS_H          0x19  // Ganancia P de posición (MSB)             // RW
#define ADDR_RAM_KD_POS_L          0x1A  // Ganancia D de posición (LSB)             // RW
#define ADDR_RAM_KD_POS_H          0x1B  // Ganancia D de posición (MSB)             // RW
#define ADDR_RAM_KI_POS_L          0x1C  // Ganancia I de posición (LSB)             // RW
#define ADDR_RAM_KI_POS_H          0x1D  // Ganancia I de posición (MSB)             // RW
#define ADDR_RAM_FEEDFORWARD1_L    0x1E  // Avance 1ª ganancia (LSB)                 // RW
#define ADDR_RAM_FEEDFORWARD1_H    0x1F  // Avance 1ª ganancia (MSB)                 // RW
#define ADDR_RAM_FEEDFORWARD2_L    0x20  // Avance 2ª ganancia (LSB)                 // RW
#define ADDR_RAM_FEEDFORWARD2_H    0x21  // Avance 2ª ganancia (MSB)                 // RW
#define ADDR_RAM_KP_SPEED_L        0x22  // Ganancia P de velocidad (LSB)            // RW
#define ADDR_RAM_KP_SPEED_H        0x23  // Ganancia P de velocidad (MSB)            // RW
#define ADDR_RAM_KI_SPEED_L        0x24  // Ganancia I de velocidad (LSB)            // RW
#define ADDR_RAM_KI_SPEED_H        0x25  // Ganancia I de velocidad (MSB)            // RW
#define ADDR_RAM_LED_BLINK_PERIOD  0x26  // Período de parpadeo LED (×11.2ms)        // RW
#define ADDR_RAM_ADC_ERROR_PERIOD  0x27  // Período de check de error ADC (×11.2ms)  // RW
#define ADDR_RAM_GARBAGE_CHECK_PERIOD 0x28  // Período de check de paquetes basura (×11.2ms) // RW
#define ADDR_RAM_STOP_DETECT_PERIOD 0x29  // Período de detección de parada (×11.2ms) // RW
#define ADDR_RAM_OVERLOAD_DETECT_PERIOD 0x2A  // Período de detección de sobrecarga (×4ms) // RW
#define ADDR_RAM_STOP_THRESHOLD    0x2B  // Umbral de parada                         // RW
#define ADDR_RAM_POS_MARGIN        0x2C  // Margen de posición                       // RW
// Reservado: 0x2D
#define ADDR_RAM_CALIB_DIFF_L      0x2E  // Calibración diferencial (LSB)            // RW
#define ADDR_RAM_CALIB_DIFF_H      0x2F  // Calibración diferencial (MSB)            // RW

// Estado y control (solo RAM)
#define ADDR_STATUS_ERROR          0x30  // Código de error actual                  // RW
#define ADDR_STATUS_DETAIL         0x31  // Detalle del error                       // RW
#define ADDR_AUX1                  0x32  // Auxiliar: reinicio posición absoluta    // RW
// Reservado: 0x33
#define ADDR_TORQUE_CTRL           0x34  // Control de torque                       // RW
#define ADDR_LED_CTRL              0x35  // Control directo del LED (bits G/R/B)   // RW
#define ADDR_VOLTAGE_SENSOR        0x36  // Lectura de voltaje real ×0.1V          // RO
#define ADDR_TEMPERATURE_SENSOR    0x37  // Temperatura actual en [°C]             // RO
#define ADDR_CURRENT_MODE          0x38  // Modo actual (posición o velocidad)     // RO
#define ADDR_TICK                  0x39  // Reloj interno del servo (0–255)        // RO
#define ADDR_POS_CALIB_L           0x3A  // Posición calibrada actual (LSB)        // RO
#define ADDR_POS_CALIB_H           0x3B  // Posición calibrada actual (MSB)        // RO
#define ADDR_POS_ABS_L             0x3C  // Posición absoluta sin calibrar (LSB)   // RO
#define ADDR_POS_ABS_H             0x3D  // Posición absoluta sin calibrar (MSB)   // RO
#define ADDR_POS_DIFF_L            0x3E  // Diferencial de posición (LSB)          // RO
#define ADDR_POS_DIFF_H            0x3F  // Diferencial de posición (MSB)          // RO
#define ADDR_PWM_OUT_L             0x40  // PWM actual (LSB)                        // RO
#define ADDR_PWM_OUT_H             0x41  // PWM actual (MSB)                        // RO
// Reservado: 0x42-0x43
#define ADDR_POS_ABS_OBJ_L         0x44  // Posición absoluta objetivo (LSB)        // RO
#define ADDR_POS_ABS_OBJ_H         0x45  // Posición absoluta objetivo (MSB)        // RO
#define ADDR_TRAJ_POS_L            0x46  // Posición deseada de trayectoria (LSB)   // RO
#define ADDR_TRAJ_POS_H            0x47  // Posición deseada de trayectoria (MSB)   // RO
#define ADDR_SPEED_GOAL_L          0x48  // Velocidad deseada actual (LSB)          // RO
#define ADDR_SPEED_GOAL_H          0x49  // Velocidad deseada actual (MSB)          // RO

// Aliases para compatibilidad con código existente
// Nota: Algunos se usan con CMD_EEP_WRITE (ROM) y otros con CMD_RAM_WRITE (RAM)
// según el contexto de uso en el código
#define ADDR_SERVO_ID          ADDR_ROM_SERVO_ID      // Usado con CMD_EEP_WRITE
#define ADDR_ACK_POLICY        ADDR_RAM_ACK_POLICY
#define ADDR_ALARM_POLICY      ADDR_RAM_ALARM_POLICY
#define ADDR_TORQUE_POLICY     ADDR_RAM_TORQUE_POLICY
#define ADDR_MAX_TEMP          ADDR_RAM_MAX_TEMP
#define ADDR_MIN_VOLTAGE       ADDR_RAM_MIN_VOLTAGE
#define ADDR_MAX_VOLTAGE       ADDR_RAM_MAX_VOLTAGE
#define ADDR_ACCEL_RATIO       ADDR_RAM_ACCEL_RATIO   // Usado con CMD_RAM_WRITE
#define ADDR_MAX_ACCEL_TIME    ADDR_RAM_MAX_ACCEL_TIME
#define ADDR_DEAD_ZONE         ADDR_RAM_DEAD_ZONE
#define ADDR_SAT_OFFSET        ADDR_RAM_SAT_OFFSET
#define ADDR_SAT_SLOPE_L       ADDR_RAM_SAT_SLOPE_L
#define ADDR_SAT_SLOPE_H       ADDR_RAM_SAT_SLOPE_H
#define ADDR_PWM_OFFSET        ADDR_RAM_PWM_OFFSET
#define ADDR_PWM_MIN           ADDR_RAM_PWM_MIN
#define ADDR_PWM_MAX_L         ADDR_RAM_PWM_MAX_L
#define ADDR_PWM_MAX_H         ADDR_RAM_PWM_MAX_H
#define ADDR_PWM_OVERLOAD_L    ADDR_RAM_PWM_OVERLOAD_L
#define ADDR_PWM_OVERLOAD_H    ADDR_RAM_PWM_OVERLOAD_H
#define ADDR_POS_MIN_L         ADDR_ROM_POS_MIN_L     // Usado con CMD_EEP_WRITE
#define ADDR_POS_MIN_H         ADDR_ROM_POS_MIN_H     // Usado con CMD_EEP_WRITE
#define ADDR_POS_MAX_L         ADDR_ROM_POS_MAX_L     // Usado con CMD_EEP_WRITE
#define ADDR_POS_MAX_H         ADDR_ROM_POS_MAX_H     // Usado con CMD_EEP_WRITE
#define ADDR_KP_POS_L          ADDR_RAM_KP_POS_L
#define ADDR_KP_POS_H          ADDR_RAM_KP_POS_H
#define ADDR_KD_POS_L          ADDR_RAM_KD_POS_L
#define ADDR_KD_POS_H          ADDR_RAM_KD_POS_H
#define ADDR_KI_POS_L          ADDR_RAM_KI_POS_L
#define ADDR_KI_POS_H          ADDR_RAM_KI_POS_H
#define ADDR_FEEDFORWARD1_L    ADDR_RAM_FEEDFORWARD1_L
#define ADDR_FEEDFORWARD1_H    ADDR_RAM_FEEDFORWARD1_H
#define ADDR_FEEDFORWARD2_L    ADDR_RAM_FEEDFORWARD2_L
#define ADDR_FEEDFORWARD2_H    ADDR_RAM_FEEDFORWARD2_H
#define ADDR_KP_SPEED_L        ADDR_RAM_KP_SPEED_L
#define ADDR_KP_SPEED_H        ADDR_RAM_KP_SPEED_H
#define ADDR_KI_SPEED_L        ADDR_RAM_KI_SPEED_L
#define ADDR_KI_SPEED_H        ADDR_RAM_KI_SPEED_H
#define ADDR_CALIB_DIFF_L      ADDR_ROM_CALIB_DIFF_L  // Usado con CMD_EEP_WRITE
#define ADDR_CALIB_DIFF_H      ADDR_ROM_CALIB_DIFF_H  // Usado con CMD_EEP_WRITE

// ============================================================================
// CÓDIGOS DE COMANDO (OPCODES)
// ============================================================================

// Comandos Request (Controlador → Servo)
#define CMD_EEP_WRITE    0x01  // Escribir en memoria EEP (requiere reinicio para aplicar cambios)
#define CMD_EEP_READ     0x02  // Leer valores desde EEP
#define CMD_RAM_WRITE    0x03  // Escribir en memoria RAM (inmediato)
#define CMD_RAM_READ     0x04  // Leer desde RAM
#define CMD_I_JOG        0x05  // Movimiento individual (tiempo distinto por servo)
#define CMD_S_JOG        0x06  // Movimiento sincronizado (tiempo común)
#define CMD_STAT         0x07  // Solicita estado del servo (siempre responde)
#define CMD_ROLLBACK     0x08  // Restaura valores de fábrica en EEP
#define CMD_REBOOT       0x09  // Reinicia el servo

// Comandos ACK (Servo → Controlador)
#define ACK_EEP_WRITE    0x41  // Respuesta a escritura en EEP
#define ACK_EEP_READ     0x42  // Respuesta a lectura desde EEP
#define ACK_RAM_WRITE    0x43  // Respuesta a escritura en RAM
#define ACK_RAM_READ     0x44  // Respuesta a lectura desde RAM
#define ACK_I_JOG        0x45  // Respuesta a comando I_JOG
#define ACK_S_JOG        0x46  // Respuesta a comando S_JOG
#define ACK_STAT         0x47  // Respuesta a STAT (siempre se envía)
#define ACK_ROLLBACK     0x48  // Respuesta a restaurar valores de fábrica
#define ACK_REBOOT       0x49  // Respuesta a reinicio del servo

// ============================================================================
// VALORES Y MÁSCARAS AUXILIARES
// ============================================================================

// Status Error Flags (registro ADDR_STATUS_ERROR = 0x30)
#define ERR_INPUT_VOLTAGE       0x01  // Voltaje fuera del rango permitido
#define ERR_POS_LIMIT           0x02  // Límite de posición superado
#define ERR_OVERHEAT            0x04  // Temperatura excedida
#define ERR_OVERLOAD            0x08  // Sobrecarga detectada
#define ERR_DRIVER_FAULT        0x10  // Fallo del controlador (no aplicable en DRS-0602)
#define ERR_EEPROM_CORRUPT      0x20  // Error en memoria EEP (checksum)
#define ERR_RESERVED1           0x40  // Reservado
#define ERR_RESERVED2           0x80  // Reservado

// Status Detail Flags (registro ADDR_STATUS_DETAIL = 0x31)
#define STAT_MOVING             0x01  // El servo está en movimiento
#define STAT_IN_POSITION        0x02  // El servo alcanzó la posición objetivo
#define STAT_CHECKSUM_ERROR     0x04  // Error de checksum en paquete recibido
#define STAT_UNKNOWN_CMD        0x08  // Comando desconocido recibido
#define STAT_EXCEED_REG_RANGE   0x10  // Dirección fuera de rango
#define STAT_GARBAGE_IN         0x20  // Detección de paquete basura
#define STAT_MOTOR_ON           0x40  // El motor está activo (torque on)
#define STAT_RESERVED           0x80  // Reservado

// Flags para comandos JOG (byte SET)
// Estructura del byte SET según documentación:
// Bit 0-2: LED (Green=0x01, Blue=0x02, Red=0x04)
// Bit 3: Modo (0=Posición, 1=Velocidad)
// Bit 4: Stop flag
// Bit 5: JogInvalid
// Bit 6: DisableVOR
// Bit 7: Reservado
#define SET_LED_GREEN           0x01  // Bit 0: LED verde
#define SET_LED_BLUE            0x02  // Bit 1: LED azul
#define SET_LED_RED             0x04  // Bit 2: LED rojo
#define SET_MODE_VELOCITY       0x08  // Bit 3: Modo velocidad (1=velocidad, 0=posición)
#define SET_STOP_FLAG           0x10  // Bit 4: Detener movimiento
#define SET_JOG_INVALID         0x20  // Bit 5: Invalidar comando
#define SET_DISABLE_VOR         0x40  // Bit 6: Deshabilitar Velocity Override

// Colores LED (pueden combinarse con OR)
#define LED_OFF                 0x00
#define LED_GREEN               0x01
#define LED_BLUE                0x02
#define LED_RED                 0x04
#define LED_PINK                0x05  // Verde + Rojo
#define LED_CYAN                0x03  // Verde + Azul
#define LED_YELLOW              0x06  // Azul + Rojo
#define LED_WHITE               0x07  // Verde + Azul + Rojo

// Valores por defecto y límites
#define POS_MIN_DEF             11000
#define POS_MAX_DEF             21000

// Timing y timeouts (en milisegundos)
#define HERKULEX_ACK_TIMEOUT_MS     30   // Tiempo máximo para esperar ACK
#define HERKULEX_MIN_CMD_GAP_MS     2    // Gap recomendado entre comandos
#define HERKULEX_RESPONSE_DELAY_MS  5    // Delay después de enviar comando
#define HERKULEX_READ_TIMEOUT_MS    100  // Timeout para lectura de respuestas

// Velocidad por defecto para cálculo de playtime (cuentas por segundo)
#define HERKULEX_DEFAULT_VELOCITY_COUNTS_PER_S  250.0f  // 200-300 recomendado
#define HERKULEX_MIN_PLAYTIME_MS    200   // Tiempo mínimo de movimiento (ms)
#define HERKULEX_MAX_PLAYTIME_MS    2500  // Tiempo máximo de movimiento (ms)
#define HERKULEX_PLAYTIME_UNIT_MS   11.2f // 1 unidad de playtime = 11.2 ms

// Control de torque adaptativo
#define HERKULEX_PWM_SAFE_LIMIT     800   // PWM considerado alto (0-1023)
#define HERKULEX_PWM_CRITICAL_LIMIT 950   // PWM crítico
#define HERKULEX_PWM_MAX_VALUE      1023  // Valor máximo de PWM

// Estimación de torque basada en PWM
// Nota: El manual no proporciona valores exactos, estas son estimaciones basadas en
// que PWM es proporcional a la corriente, y la corriente es proporcional al torque
// DRS-0602: Torque máximo típico ~6.0 Nm (según especificaciones del modelo)
#define HERKULEX_TORQUE_MAX_NM      6.0f  // Torque máximo en Nm (ajustable según modelo)
#define HERKULEX_CURRENT_MAX_MA     2000.0f  // Corriente máxima estimada en mA (ajustable)

// Poses predefinidas
#define POSE_INICIAL   {16507, 18800, 16000, 18000, 16411}
#define POSE_TRABAJO   {16001, 15093, 12381, 18000, 16411}
#define POSE_TRABAJO_2  {16001, 16655, 16302, 18000, 14878}
#define POSE_MATE      {16931, 15576, 14394, 16532, 15938}
#define POSE_MATE_2    {16282, 17427, 16549, 17843, 13832}
#define POSE_SUP       {13365, 16436, 11700, 19535, 14400}
#define POSE_END       {16282, 17387, 16243, 17843, 13832}

/*
motor - posicion cero sleep segun Valentín 
1 - 16384
2 - 20638
3 - 16000
4 - 16526
5 - 16026

pos dos
1 19609
2 18383
3 15127
4 13075
5 14329
Limite motor 1, 14000/19550
*/

#endif
