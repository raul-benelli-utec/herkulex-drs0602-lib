#ifndef HERKULEX_DEFS_H
#define HERKULEX_DEFS_H

// Broadcast ID
#define HERKULEX_BROADCAST_ID 0xFD

// Comandos HerkuleX
#define CMD_EEP_WRITE  0x01
#define CMD_EEP_READ   0x02
#define CMD_RAM_WRITE  0x03
#define CMD_RAM_READ   0x04



//Direcciones de memoria

// ======== Identificación del modelo (ROM) ========
#define ADDR_MODEL_NO1         0x00  // Dígito alto del número de modelo         // RO
#define ADDR_MODEL_NO2         0x01  // Dígito bajo del número de modelo         // RO
#define ADDR_VERSION1          0x02  // Versión mayor del firmware               // RO
#define ADDR_VERSION2          0x03  // Versión menor del firmware               // RO
#define ADDR_BAUD_RATE         0x04  // Velocidad de comunicación serial         // RW

// ======== Configuración básica (RAM) ========
#define ADDR_SERVO_ID          0x06  // ID del servo (0x00–0xFD)                 // RW
#define ADDR_ACK_POLICY        0x07  // Política de respuesta (0–2)              // RW
#define ADDR_ALARM_POLICY      0x08  // Política de alarmas                      // RW
#define ADDR_TORQUE_POLICY     0x09  // Política de torque y errores             // RW

// ======== Límites térmicos y eléctricos ========
#define ADDR_MAX_TEMP          0x0B  // Temperatura máxima [°C]                  // RW
#define ADDR_MIN_VOLTAGE       0x0C  // Voltaje mínimo (×0.1 V)                  // RW
#define ADDR_MAX_VOLTAGE       0x0D  // Voltaje máximo (×0.1 V)                  // RW

// ======== Control de aceleración y zona muerta ========
#define ADDR_ACCEL_RATIO       0x0E  // Relación de aceleración (%)              // RW
#define ADDR_MAX_ACCEL_TIME    0x0F  // Tiempo máx de aceleración (×11.2ms)      // RW
#define ADDR_DEAD_ZONE         0x10  // Zona muerta (solo en control de posición) // RW

// ======== Saturación y PWM ========
#define ADDR_SAT_OFFSET        0x11  // Offset de saturación                     // RW
#define ADDR_SAT_SLOPE_L       0x12  // Pendiente de saturación (LSB)            // RW
#define ADDR_SAT_SLOPE_H       0x13  // Pendiente de saturación (MSB)            // RW
#define ADDR_PWM_OFFSET        0x14  // Compensación PWM                         // RW

// ======== PWM y límites de torque ========
#define ADDR_PWM_MIN           0x15  // PWM mínimo permitido                     // RW
#define ADDR_PWM_MAX_L         0x16  // PWM máximo (LSB)                         // RW
#define ADDR_PWM_MAX_H         0x17  // PWM máximo (MSB)                         // RW
#define ADDR_PWM_OVERLOAD_L    0x18  // Umbral sobrecarga PWM (LSB)              // RW
#define ADDR_PWM_OVERLOAD_H    0x19  // Umbral sobrecarga PWM (MSB)              // RW

// ======== Límites de posición =========
#define ADDR_POS_MIN_L         0x1A  // Posición mínima permitida (LSB)          // RW
#define ADDR_POS_MIN_H         0x1B  // Posición mínima permitida (MSB)          // RW
#define ADDR_POS_MAX_L         0x1C  // Posición máxima permitida (LSB)          // RW
#define ADDR_POS_MAX_H         0x1D  // Posición máxima permitida (MSB)          // RW

// ======== PID de posición ========
#define ADDR_KP_POS_L          0x1E  // Ganancia P de posición (LSB)             // RW
#define ADDR_KP_POS_H          0x1F  // Ganancia P de posición (MSB)             // RW
#define ADDR_KD_POS_L          0x20  // Ganancia D de posición (LSB)             // RW
#define ADDR_KD_POS_H          0x21  // Ganancia D de posición (MSB)             // RW
#define ADDR_KI_POS_L          0x22  // Ganancia I de posición (LSB)             // RW
#define ADDR_KI_POS_H          0x23  // Ganancia I de posición (MSB)             // RW

// ======== Ganancias de compensación/avance ========
#define ADDR_FEEDFORWARD1_L    0x24  // Avance 1ª ganancia (LSB)                 // RW
#define ADDR_FEEDFORWARD1_H    0x25  // Avance 1ª ganancia (MSB)                 // RW
#define ADDR_FEEDFORWARD2_L    0x26  // Avance 2ª ganancia (LSB)                 // RW
#define ADDR_FEEDFORWARD2_H    0x27  // Avance 2ª ganancia (MSB)                 // RW

// ======== PID de velocidad ========
#define ADDR_KP_SPEED_L        0x28  // Ganancia P de velocidad (LSB)            // RW
#define ADDR_KP_SPEED_H        0x29  // Ganancia P de velocidad (MSB)            // RW
#define ADDR_KI_SPEED_L        0x2A  // Ganancia I de velocidad (LSB)            // RW
#define ADDR_KI_SPEED_H        0x2B  // Ganancia I de velocidad (MSB)            // RW

// ======== Calibración =========
#define ADDR_CALIB_DIFF_L       0x34  // Calibración diferencial baja (LSB)
#define ADDR_CALIB_DIFF_H       0x35  // Calibración diferencial alta (MSB)


// ======== Estado de error (solo lectura) =========
#define ADDR_STATUS_ERROR       0x30  // Código de error actual                  // RW
#define ADDR_STATUS_DETAIL      0x31  // Detalle del error                       // RW
#define ADDR_AUX1               0x32  // Auxiliar: reinicio posición absoluta    // RW

// ======== Control torque y LED =========
#define ADDR_TORQUE_CTRL        0x34  // Control de torque                       // RW
#define ADDR_LED_CTRL           0x35  // Control directo del LED (bits G/R/B)   // RW

// ======== Sensores (solo lectura) =========
#define ADDR_VOLTAGE_SENSOR     0x36  // Lectura de voltaje real ×0.1V          // RO
#define ADDR_TEMPERATURE_SENSOR 0x37  // Temperatura actual en [°C]             // RO
#define ADDR_CURRENT_MODE       0x38  // Modo actual (posición o velocidad)     // RO
#define ADDR_TICK               0x39  // Reloj interno del servo (0–255)        // RO

// ======== Posiciones y velocidades (solo lectura) =========
#define ADDR_POS_CALIB_L        0x3A  // Posición calibrada actual (LSB)        // RO
#define ADDR_POS_CALIB_H        0x3B  // Posición calibrada actual (MSB)        // RO
#define ADDR_POS_ABS_L          0x3C  // Posición absoluta sin calibrar (LSB)   // RO
#define ADDR_POS_ABS_H          0x3D  // Posición absoluta sin calibrar (MSB)   // RO
#define ADDR_POS_DIFF_L         0x3E  // Diferencial de posición (LSB)          // RO
#define ADDR_POS_DIFF_H         0x3F  // Diferencial de posición (MSB)          // RO
#define ADDR_PWM_OUT_L          0x40  // PWM actual (LSB)                        // RO
#define ADDR_PWM_OUT_H          0x41  // PWM actual (MSB)                        // RO

// ======== Posición absoluta objetivo (solo lectura) =========
#define ADDR_POS_ABS_OBJ_L      0x44  // Posición absoluta objetivo (LSB)        // RO
#define ADDR_POS_ABS_OBJ_H      0x45  // Posición absoluta objetivo (MSB)        // RO

// ======== Posición de trayectoria deseada =========
#define ADDR_TRAJ_POS_L         0x46  // Posición deseada de trayectoria (LSB)   // RO
#define ADDR_TRAJ_POS_H         0x47  // Posición deseada de trayectoria (MSB)   // RO

// ======== Velocidad deseada =========
#define ADDR_SPEED_GOAL_L       0x48  // Velocidad deseada actual (LSB)          // RO
#define ADDR_SPEED_GOAL_H       0x49  // Velocidad deseada actual (MSB)          // RO

// ======== Comandos al servo (Request Packets) ========
#define CMD_EEP_WRITE    0x01  // Escribir en memoria EEP (requiere reinicio para aplicar cambios)
#define CMD_EEP_READ     0x02  // Leer valores desde EEP
#define CMD_RAM_WRITE    0x03  // Escribir en memoria RAM (inmediato)
#define CMD_RAM_READ     0x04  // Leer desde RAM
#define CMD_I_JOG        0x05  // Movimiento individual (tiempo distinto por servo)
#define CMD_S_JOG        0x06  // Movimiento sincronizado (tiempo común)
#define CMD_STAT         0x07  // Solicita estado del servo (siempre responde)
#define CMD_ROLLBACK     0x08  // Restaura valores de fábrica en EEP
#define CMD_REBOOT       0x09  // Reinicia el servo

// ======== Respuestas del servo (ACK Packets) ========
#define ACK_EEP_WRITE    0x41  // Respuesta a escritura en EEP
#define ACK_EEP_READ     0x42  // Respuesta a lectura desde EEP
#define ACK_RAM_WRITE    0x43  // Respuesta a escritura en RAM
#define ACK_RAM_READ     0x44  // Respuesta a lectura desde RAM
#define ACK_I_JOG        0x45  // Respuesta a comando I_JOG
#define ACK_S_JOG        0x46  // Respuesta a comando S_JOG
#define ACK_STAT         0x47  // Respuesta a STAT (siempre se envía)
#define ACK_ROLLBACK     0x48  // Respuesta a restaurar valores de fábrica
#define ACK_REBOOT       0x49  // Respuesta a reinicio del servo

// ======== Status Error Flags (registro 0x30) ========
#define ERR_INPUT_VOLTAGE       0x01  // Voltaje fuera del rango permitido
#define ERR_POS_LIMIT           0x02  // Límite de posición superado
#define ERR_OVERHEAT            0x04  // Temperatura excedida
#define ERR_OVERLOAD            0x08  // Sobrecarga detectada
#define ERR_DRIVER_FAULT        0x10  // Fallo del controlador (no aplicable en DRS-0602)
#define ERR_EEPROM_CORRUPT      0x20  // Error en memoria EEP (checksum)
#define ERR_RESERVED1           0x40  // Reservado
#define ERR_RESERVED2           0x80  // Reservado

// ======== Status Detail Flags (registro 0x31) ========
#define STAT_MOVING             0x01  // El servo está en movimiento
#define STAT_IN_POSITION        0x02  // El servo alcanzó la posición objetivo
#define STAT_CHECKSUM_ERROR     0x04  // Error de checksum en paquete recibido
#define STAT_UNKNOWN_CMD        0x08  // Comando desconocido recibido
#define STAT_EXCEED_REG_RANGE   0x10  // Dirección fuera de rango
#define STAT_GARBAGE_IN         0x20  // Detección de paquete basura
#define STAT_MOTOR_ON           0x40  // El motor está activo (torque on)
#define STAT_RESERVED           0x80  // Reservado

#define SET_STOP_FLAG      0x01
#define SET_MODE_VELOCITY  0x02  // 1 = velocity, 0 = position
#define SET_LED_GREEN      0x04
#define SET_LED_BLUE       0x08
#define SET_LED_RED        0x10
#define SET_JOG_INVALID    0x20
#define SET_DISABLE_VOR    0x40


// Colores LED (pueden combinarse con OR si querés)
#define LED_OFF   0x00
#define LED_GREEN 0x01
#define LED_RED   0x02
#define LED_BLUE  0x04
#define LED_PINK  0x08
#define LED_WHITE 0x0F

// Varibles 
#define POS_MIN_DEF     11000
#define POS_MAX_DEF     21000
#define PLAYTIME_MIN    1000     // ~112 ms
#define PLAYTIME_MAX    4000    // ~2.24 s

// Poses
#define POSE_INICIAL   {16507, 18800, 16000, 18000, 16411}
//18784
#define POSE_TRABAJO   {16001, 15093, 12381, 18000, 16411}
//16892
#define POSE_TRABAJO_2  {16001, 16655, 16302, 18000, 14878}
//16223
#define POSE_MATE  {16931, 15576, 14394, 16532, 15938}//19524
#define POSE_MATE_2  {16282, 17427, 16549, 17843, 13832}//13365
//15026
#define POSE_SUP  {13365, 16436, 11700, 19535, 14400}//19479
#define POSE_END  {16282, 17387, 16243, 17843, 13832}//19479

//16433
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