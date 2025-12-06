# Modos de control y perfil de movimiento – HerkuleX DRS-0602

Este documento describe los modos de operación del DRS-0602 y los
parámetros de movimiento más relevantes.

## Rango de operación y modos básicos

El servo admite:   

- **Control de posición**: rango típico hasta 900° mecánicos.  
- **Control de velocidad / giro continuo**: rotación ilimitada controlando velocidad.  
- Al encender en modo posición, la posición inicial reconocida debe estar dentro
  de ~0–320° para que el cero se inicialice correctamente.

La resolución típica es de ~0,028° por unidad interna, lo que permite control
suave y preciso. :contentReference[oaicite:11]{index=11}  

## Perfil de velocidad y aceleración

Dos parámetros de RAM controlan el perfil de movimiento: :contentReference[oaicite:12]{index=12}  

- **Acceleration Ratio** (RAM addr 8): porcentaje del tiempo total de movimiento
  dedicado a acelerar y desacelerar.
  - Valores bajos → cambios de velocidad más bruscos, más vibración.
  - Valores altos → aceleración más suave pero con cambios marcados en la zona central.
- **Maximum Acceleration Time** (RAM addr 9):  
  - Unidad: 11,2 ms por incremento.  
  - Máximo ≈ 2,844 s con valor 254.  
  - Si es 0, el perfil de velocidad es rectangular.

En la práctica, muchos controladores usan un perfil “en escalera” (ladder) para
obtener movimientos suaves y eficientes. :contentReference[oaicite:13]{index=13}  

## Control de torque (RAM addr 52)

El registro de **Torque Control** define el estado del par en el servo: :contentReference[oaicite:14]{index=14}  

- `0x60` → Torque ON (par activo, el servo sigue comandos I_JOG / S_JOG).  
- `0x40` → Brake ON (freno eléctrico, mantiene posición, no ejecuta JOG).  
- `0x00` → Torque Free (eje libre, se puede mover fácilmente a mano).

Notas importantes:

- El modo de control efectivo depende de `Current Control Mode` (RAM addr 56). :contentReference[oaicite:15]{index=15}  
- Al encender, el modo de control actual se inicializa en **Position Control (0)**. :contentReference[oaicite:16]{index=16}  
- Comandos `I_JOG` / `S_JOG` sólo funcionan con torque en `Torque ON`.

## Control de LED (RAM addr 53)

El registro de LED permite encender/apagar los colores del LED integrado:   

- Bit 0 (`0x01`): Verde  
- Bit 1 (`0x02`): Azul  
- Bit 2 (`0x04`): Rojo  

Cuando hay una alarma activa y la política de LED lo indica, el valor de este
registro puede ser ignorado en favor del parpadeo de error.

## Protección y feedback

El servo incorpora protecciones por: :contentReference[oaicite:18]{index=18}  

- Temperatura excesiva  
- Sobrecarga mecánica  
- Tensión fuera de rango  

Y puede reportar:

- Temperatura actual  
- Voltaje de alimentación  
- Estados de error y detalle de error (registros de status)

Estos valores están mapeados en los registros de RAM y ya están listados en
`docs/registers_drs0602.md`.
