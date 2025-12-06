# Comandos del protocolo – HerkuleX DRS-0602

Este archivo documenta los comandos del protocolo de comunicación del servo HerkuleX DRS-0602, incluyendo los comandos Request (enviados del controlador al servo) y los comandos ACK (respuestas del servo al controlador).

## Comandos Request (Controlador → Servo)

| Nombre | Opcode | Tipo | Descripción |
|--------|--------|------|-------------|
| EEP_write | 0x01 | Request | Escritura de la longitud de valores en la dirección del registro EEP. Longitud de datos: Dirección y longitud de cada 1 byte + Longitud de byte |
| EEP_read | 0x02 | Request | Lectura de la longitud de valores desde la dirección del registro EEP. Longitud de datos: 2. Puede no responder según la política r(ACK) |
| RAM_write | 0x03 | Request | Escritura de valores de longitud en la dirección del registro RAM. Longitud de los datos: Dirección y longitud de cada uno (1 byte + byte de longitud) |
| RAM_read | 0x04 | Request | Lectura de valores de longitud desde la dirección del registro RAM. Longitud de los datos: 2. Puede que no responda según la política r(ACK) |
| I_jog | 0x05 | Request | Puede enviar un comando JOG a un máximo de 43 servos Herkulex. Envía el comando a varios servos Herkulex simultáneamente. Asigna posición/tiempo a todos los servos Herkulex individualmente. Asigna tiempo de reproducción a los servos Herkulex individualmente. LJOG requiere 5 bytes de datos por cada servo Herkulex. Se necesitan 50 bytes de datos opcionales para enviar el comando a 10 servos Herkulex. Consulta la página 40 para el formato LJOG |
| S_jog | 0x06 | Request | Permite enviar el comando JOG a un máximo de 53 servos Herkulex. Envía el comando a varios servos Herkulex simultáneamente. Todos los servos Herkulex tienen el mismo tiempo de reproducción y llegan a la posición objetivo al mismo tiempo. S JOG requiere 1 byte de tiempo de reproducción y 4 bytes para cada servo Herkulex. Se necesitan 41 bytes de longitud de datos opcional para enviar comandos a 10 servos Herkulex. Consulte la página 41 para ver el formato S_JOG_TAG |
| Stat | 0x07 | Request | Solicitar el estado del servo Herkulex (error de estado, datos de estado). Enviar siempre respuesta al paquete STAT, independientemente de la política ACK |
| RollBack | 0x08 | Request | Cambiar todos los valores del registro EEP a los valores predeterminados de fábrica. Aplicar los cambios después del reinicio. e(ID, velocidad en baudios, diferencia de calibración) puede eximirse de la inicialización predeterminada de fábrica mediante omisión de ID, omisión de baudios y omisión de calibración. Según la configuración de omisión de calibración, e(diferencia de calibración) puede eximirse |
| Reboot | 0x09 | Request | Reiniciar el servo Herkulex |

## Comandos ACK (Servo → Controlador)

| Nombre | Opcode | Tipo | Descripción |
|--------|--------|------|-------------|
| EEP_write | 0x41 | ACK | Paquete de respuesta a CMD(0x01). El valor predeterminado es no responder. Se puede responder modificando la configuración de la política r(ACK) |
| EEP_read | 0x42 | ACK | Longitud de respuesta: número de valores de la dirección del registro EEP. Es posible que no se responda según la configuración de la política r(ACK) |
| RAM_write | 0x43 | ACK | Paquete de respuesta a CMD(0x03). El valor predeterminado es no responder. Se puede responder modificando la configuración de r(Política ACK) |
| RAM_read | 0x44 | ACK | Longitud de respuesta: número de valores de la dirección del registro RAM. Puede que no se responda según la configuración de r(Política ACK) |
| I_jog | 0x45 | ACK | Paquete de respuesta a CMD(0x05). El valor predeterminado es no responder. Se puede responder modificando la configuración de r(Política ACK) |
| S_jog | 0x46 | ACK | Paquete de respuesta a CMD(0x06). El valor predeterminado es no responder. Se puede responder modificando la configuración de r(Política ACK) |
| Stat | 0x47 | ACK | Respuesta r(Error de estado, Detalle de estado). Responder siempre independientemente de r(Política ACK) |
| RollBack | 0x48 | ACK | Paquete de respuesta a CMD(0x08). El valor predeterminado es no responder. Se puede responder modificando la configuración de r(Política ACK) |
| Reboot | 0x49 | ACK | Paquete de respuesta a CMD(0x09). El valor predeterminado es no responder. Se puede responder modificando la configuración de r(Política ACK) |
