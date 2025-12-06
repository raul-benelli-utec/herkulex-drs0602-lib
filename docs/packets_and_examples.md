# 4-1. Communication Protocol

## Introducción

Servo Controller comunica con los servos en la red enviando un **Request Packet** y recibiendo un **ACK Packet** desde el servo.  
En una red multidrop, solo el servo cuyo **pID** coincide responde al comando.

Parámetros de comunicación:

| Parámetro     | Valor |
|---------------|-------|
| Data Bit      | 8     |
| Flow Control  | None  |
| Baud Rate     | 57,600 / 115,200 / 0.2M / 0.25M / 0.4M / 0.5M / 0.666666M / 1Mbaud |

---

# ## Packet Structure

Los paquetes tienen los siguientes campos:

| Campo      | Valor / Rango         | Descripción |
|-----------|------------------------|-------------|
| Header    | `0xFF 0xFF`            | Inicio del paquete |
| Packet Size | `7 ~ 223`            | Bytes desde Header hasta Data |
| pID       | `0 ~ 0xFE`             | ID destino |
| CMD       | `0x01 ~ 0x09`          | Comando |
| CheckSum1 | Ver fórmula            | Detección de error |
| CheckSum2 | Ver fórmula            | Detección de error |
| Data[n]   | Hasta 216 bytes        | Depende del comando |

---

## Header

| Campo | Valor | Bytes |
|--------|--------|--------|
| Header | `0xFF 0xFF` | 2 |

---

## Packet Size

- Tamaño desde el header hasta el último byte de data  
- Máximo recomendado: 223  
- Tamaño mínimo sin data: 7 bytes

---

## pID — Servo ID

| Campo | Rango | Significado |
|--------|--------|--------------|
| pID | `0–253` | ID del servo |
| `0xFE` | especial | Afecta **a todos los servos** |

---

## CMD — Comandos

| CMD (Request) | CMD (ACK) | Nombre |
|---------------|-----------|--------|
| 0x01 | 0x41 | EEP_WRITE |
| 0x02 | 0x42 | EEP_READ |
| 0x03 | 0x43 | RAM_WRITE |
| 0x04 | 0x44 | RAM_READ |
| 0x05 | 0x45 | I_JOG |
| 0x06 | 0x46 | S_JOG |
| 0x07 | 0x47 | STAT |
| 0x08 | 0x48 | ROLLBACK |
| 0x09 | 0x49 | REBOOT |

---

## CheckSum1

```
CheckSum1 = (PacketSize ^ pID ^ CMD ^ Data[0] ^ ... ^ Data[n]) & 0xFE
```

---

## CheckSum2

```
CheckSum2 = (~(PacketSize ^ pID ^ CMD ^ Data[0] ^ ... ^ Data[n])) & 0xFE
```

---

# ## Register Map

Los servos DRS-0602 contienen:

- **EEP Registers (no volátiles)**
- **RAM Registers (volátiles)**

EEP se copia a RAM durante el encendido.

---

# ### Non-Volatile Memory (EEP Register)

Características:

- Mantiene valores sin energía  
- No afecta operación directamente hasta ser copiado a RAM  
- Para aplicar cambios: **REBOOT**

Incluye:

- Dirección  
- Valor por defecto  
- Rango válido  
- Tipo RO / RW  

Notación:

| Notación | Significado |
|----------|-------------|
| `e(Reg_Name)` | Valor en EEP |
| `r(Reg_Name)` | Valor en RAM |

---

# ## PWM, Max PWM y Dead Zone

### PWM

PWM representa la energía de entrada.  
Mayor PWM → mayor torque / velocidad.

### Max PWM

Limita la energía máxima permitida.

### Dead Zone

Cuando el error entre posición actual y objetivo es menor al valor Dead Zone →  
el servo considera alcanzado el objetivo.

Recomendación: **Dead Zone < 10**

---

# ## Volatile Memory (RAM Register)

Los valores de RAM afectan operación en tiempo real.  
Se reinician al valor EEP al encender.

---

# ## Acceleration Ratio (RAM Address 8)

- Disminuir ratio → cambios bruscos, vibración  
- Aumentar ratio → movimientos suaves pero con brusquedad final  

Controladores suelen usar perfil escalonado.

---

# ## Maximum Acceleration Time (RAM Address 9)

- 1 unidad = 11.2 ms  
- Máximo: 254 → 2.844 s  
- Valor 0 → perfil rectangular

---

# ## Torque Control (RAM Address 52)

| Valor | Estado |
|--------|---------|
| 0x40 | Brake On |
| 0x60 | Torque On |
| 0x00 | Torque Free |

Notas:

- I_JOG / S_JOG requieren **Torque On**  
- Brake On impide movimiento  
- Torque Free permite mover la articulación a mano  

---

# ## LED Control

| Bit | LED |
|------|------|
| 0x01 | Verde |
| 0x02 | Azul |
| 0x04 | Rojo |

---

# # EEP_READ — Ejemplo 1 (Request)

### Solicitud:  
Servo ID **253**, leer **4 bytes** desde dirección **0x1E** (Position Kp/Kd).

### Tabla del paquete Request:

| Campo | Valor |
|--------|--------|
| Header | 0xFF 0xFF |
| PacketSize | 0x09 |
| pID | 0xFD |
| CMD | 0x02 |
| CheckSum1 | 0xEC |
| CheckSum2 | 0x12 |
| Data[0] | 0x1E |
| Data[1] | 0x04 |

---

# # EEP_READ — ACK de Ejemplo 1

### Valores recibidos:

- Kp = 440 (`0x01B8`)  
- Kd = 8000 (`0x1F40`)  
- Últimos 2 bytes siempre: Status Error + Status Detail

### Tabla ACK:

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x0F |
| pID        | 0xFD |
| CMD        | 0x42 |
| CheckSum1  | 0x4C |
| CheckSum2  | 0xB2 |
| Data[0]    | 0x1E |
| Data[1]    | 0x04 |
| Data[2]    | 0xB8 |
| Data[3]    | 0x01 |
| Data[4]    | 0x40 |
| Data[5]    | 0x1F |
| Data[6]    | 0x00 (Status Error) |
| Data[7]    | 0x00 (Status Detail) |

---

# # EEP_WRITE — Ejemplo

Servo ID **253**, escribir:

- Kp = 200 (0x00C8)  
- Kd = 1000 (0x03E8)  

Dirección base: **0x1E**, longitud: 4 bytes.

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x0D |
| pID        | 0xFD |
| CMD        | 0x01 |
| CheckSum1  | 0xC8 |
| CheckSum2  | 0x36 |
| Data[0]    | 0x1E |
| Data[1]    | 0x04 |
| Data[2]    | 0xC8 |
| Data[3]    | 0x00 |
| Data[4]    | 0xE8 |
| Data[5]    | 0x03 |

> Después de escribir en EEP, es necesario enviar **REBOOT**.

---

# # RAM_WRITE

## Ejemplo 1 — Encender LED verde (Address 0x35)

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x0A |
| pID        | 0xFD |
| CMD        | 0x03 |
| CheckSum1  | 0xC0 |
| CheckSum2  | 0x3E |
| Data[0]    | 0x35 |
| Data[1]    | 0x01 |
| Data[2]    | 0x01 |

---

## Ejemplo 2 — Limpiar Status Error y Detail (Address 0x30)

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x0B |
| pID        | 0xFD |
| CMD        | 0x03 |
| CheckSum1  | 0xC6 |
| CheckSum2  | 0x38 |
| Data[0]    | 0x30 |
| Data[1]    | 0x02 |
| Data[2]    | 0x00 |
| Data[3]    | 0x00 |

---

## Ejemplo 3 — Torque ON (Address 0x34)

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x0A |
| pID        | 0xFD |
| CMD        | 0x03 |
| CheckSum1  | 0xA0 |
| CheckSum2  | 0x5E |
| Data[0]    | 0x34 |
| Data[1]    | 0x01 |
| Data[2]    | 0x60 |

---

# # RAM_READ

Ejemplo: Leer LED Control (Address 0x35)

## Request

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x09 |
| pID        | 0xFD |
| CMD        | 0x04 |
| CheckSum1  | 0xC4 |
| CheckSum2  | 0x3A |
| Data[0]    | 0x35 |
| Data[1]    | 0x01 |

## ACK

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x0C |
| pID        | 0xFD |
| CMD        | 0x44 |
| CheckSum1  | 0xC2 |
| CheckSum2  | 0x3C |
| Data[0]    | 0x35 |
| Data[1]    | 0x01 |
| Data[2]    | 0x01 (LED verde ON) |
| Data[3]    | 0x00 (Status Error) |
| Data[4]    | 0x42 (Torque ON + Inposition) |
# # I_JOG

I_JOG permite controlar posición o velocidad según el modo.  
El paquete incluye:

- Datos de movimiento (iJogData)
- LED
- Modo (posición o velocidad)
- Tiempo de ejecución (playtime)
- ID destino

Formato general:

| Campo      | Descripción |
|------------|-------------|
| JOG(LSB)   | Parte baja del dato de posición/velocidad |
| JOG(MSB)   | Parte alta |
| SET        | Bits de control (LED, modo, stop, etc.) |
| ID         | Servo destino |
| playtime   | Tiempo de ejecución en múltiplos de 11.2ms |

---

# ## I_JOG — Ejemplo 1  
### Position control, goal = 512, LED verde, playtime = 60 (672ms)

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x0C |
| pID        | 0xFD |
| CMD        | 0x05 |
| CheckSum1  | 0x32 |
| CheckSum2  | 0xCC |
| JOG(LSB)   | 0x00 |
| JOG(MSB)   | 0x02 |
| SET        | 0x04 (Pos control + LED verde) |
| ID         | 0xFD |
| playtime   | 0x3C |

---

# ## I_JOG — Ejemplo 2  
### Infinite turn, goal speed = 320, LED azul, playtime = 60

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x0C |
| pID        | 0xFD |
| CMD        | 0x05 |
| CheckSum1  | 0x7E |
| CheckSum2  | 0x80 |
| JOG(LSB)   | 0x40 |
| JOG(MSB)   | 0x01 |
| SET        | 0x0A (Vel control + LED azul) |
| ID         | 0x0A |
| playtime   | 0x3C |

---

# ## Bitfield de I_JOG (estructura del manual)

```
typedef struct
{
    int iJogData : 15;
    unsigned int uiReserved1 : 1;
    unsigned int uiStop : 1;
    unsigned int uiMode : 1; //0 : Position control
    unsigned int uiLED : 3;  //Green, Blue, Red
    unsigned int uiJogInvalid : 1;
    unsigned int unDisableVOR : 1;
    unsigned int uiReserved2 : 1;
    unsigned int ucID : 8;
    unsigned char ucJogTime_ms;
} IJOG_TAG;
```

Notas:

- `unDisableVOR = 1` → desactiva "Velocity Override"
- Estructura depende del compilador (alineación de bytes)

---

# # S_JOG

S_JOG es similar a I_JOG, pero:

- `playtime` aparece antes del dato JOG  
- El ID aparece al final  
- Modo = 1 indica control de velocidad  

Usado para movimientos sincronizados entre múltiples servos.

---

# ## S_JOG — Ejemplo 1  
### Position control, goal = 512, LED rojo, playtime = 60

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x0C |
| pID        | 0xFD |
| CMD        | 0x06 |
| CheckSum1  | 0x30 |
| CheckSum2  | 0xCE |
| playtime   | 0x3C |
| JOG(LSB)   | 0x00 |
| JOG(MSB)   | 0x02 |
| SET        | 0x04 (Pos control + LED rojo) |
| ID         | 0xFD |

---

# ## S_JOG — Ejemplo 2  
### Infinite turn, goal speed = 704, LED azul, playtime = 60

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x0C |
| pID        | 0xFD |
| CMD        | 0x06 |
| CheckSum1  | 0xFE |
| CheckSum2  | 0x00 |
| playtime   | 0x3C |
| JOG(LSB)   | 0x40 |
| JOG(MSB)   | 0x01 |
| SET        | 0x0A |
| ID         | 0x0A |

---

# ## Bitfield de S_JOG

```
typedef struct
{
    int iJogData : 15;
    unsigned int uiReserved1 : 1;
    unsigned int uiStop : 1;
    unsigned int uiMode : 1;  //1 : Speed Control
    unsigned int uiLED : 3;   //Green, Blue, Red
    unsigned int uiJogInvalid : 1;
    unsigned int unDisableVOR : 1;
    unsigned int uiReserved2 : 1;
    unsigned int ucID : 8;
} SJOG_TAG;
```

---

# # STAT

STAT permite consultar:

- Status Error  
- Status Detail  

Ejemplo: Solicitar estado del Servo ID 253.

## Request STAT

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x07 |
| pID        | 0xFD |
| CMD        | 0x07 |
| CheckSum1  | 0xFC |
| CheckSum2  | 0x02 |

## ACK STAT

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x09 |
| pID        | 0xFD |
| CMD        | 0x47 |
| CheckSum1  | 0xF2 |
| CheckSum2  | 0x0C |
| Data[0]    | 0x00 (Status Error) |
| Data[1]    | 0x40 (Torque On) |

---

# # ROLLBACK

ROLLBACK restaura valores de fábrica:

- Restaura **EEP Register**
- No cambia ID ni Baud Rate si se usan Skip Options

## Opciones

| Opción | Descripción |
|--------|-------------|
| 0x01 | ID Skip |
| 0x10 | Cali Skip |
| 0x01 (2nd byte) | Baud Skip |

---

## ROLLBACK — Request

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x09 |
| pID        | 0xFD |
| CMD        | 0x08 |
| CheckSum1  | 0xFC |
| CheckSum2  | 0x02 |
| Data[0]    | Skip Option 1 |
| Data[1]    | Skip Option 2 |

---

## ROLLBACK — ACK

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x09 |
| pID        | 0xFD |
| CMD        | 0x48 |
| CheckSum1  | 0xBC |
| CheckSum2  | 0x42 |
| Data[0]    | 0x01 |
| Data[1]    | 0x01 |

---

# # REBOOT

Reinicia el servo.  
Importante después de escribir en **EEP Register**.

## REBOOT — Request

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x07 |
| pID        | 0xFD |
| CMD        | 0x09 |
| CheckSum1  | 0xF2 |
| CheckSum2  | 0x0C |

## REBOOT — ACK

| Campo      | Valor |
|------------|--------|
| Header     | 0xFF 0xFF |
| PacketSize | 0x09 |
| pID        | 0xFD |
| CMD        | 0x49 |
| CheckSum1  | 0xBC |
| CheckSum2  | 0x42 |
| Data[0]    | 0x00 |
| Data[1]    | 0x00 |
