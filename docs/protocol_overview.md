# Protocolo de comunicación – HerkuleX DRS-0602

Este documento resume la estructura del paquete y las reglas básicas del
protocolo UART utilizado por los servos HerkuleX DRS-0602.

## Parámetros de comunicación

- Interfaz: UART TTL, full-duplex, multidrop.
- Bits de datos: 8
- Control de flujo: ninguno
- Velocidades soportadas (baudrate): 57 600, 115 200, 200 000, 250 000,
  400 000, 500 000, 666 666 y 1 000 000 baudios (1 Mbps). :contentReference[oaicite:0]{index=0}

## Estructura general del paquete

Cada paquete (Request o ACK) tiene el siguiente formato: :contentReference[oaicite:1]{index=1}

| Campo       | Bytes | Descripción                                                                 |
|-------------|-------|------------------------------------------------------------------------------|
| Header      | 2     | Siempre `0xFF 0xFF`. Marca el inicio del paquete.                           |
| PacketSize  | 1     | Tamaño total del paquete desde el primer `0xFF` hasta el último byte de Data. |
| pID         | 1     | ID de destino del servo (0–253) o valor especial `0xFE` (broadcast). :contentReference[oaicite:2]{index=2} |
| CMD         | 1     | Código de comando a ejecutar (EEP_READ, RAM_WRITE, I_JOG, etc.).            |
| CheckSum1   | 1     | Primer checksum, calculado por XOR.                                         |
| CheckSum2   | 1     | Segundo checksum, complementario del primero.                               |
| Data\[n\]   | 0–216 | Datos opcionales del comando. El tamaño depende del CMD.                    |

- Tamaño mínimo de paquete sin datos: **7 bytes**. :contentReference[oaicite:3]{index=3}  
- Tamaño máximo recomendado: **≤ 223 bytes** (si es mayor puede no ser reconocido). :contentReference[oaicite:4]{index=4}  

> Nota: en el contexto del paquete se usa el término **pID** para evitar confusión
> con el ID del servo a nivel de documentación. :contentReference[oaicite:5]{index=5}

## Cálculo de checksums

Sea:

```text
X = PacketSize ^ pID ^ CMD ^ Data[0] ^ Data[1] ^ ... ^ Data[n]
