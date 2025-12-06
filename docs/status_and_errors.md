# Estados, errores y políticas de alarma – HerkuleX DRS-0602

Este documento resume cómo el DRS-0602 maneja estados de error, políticas
de alarma y el uso de los registros EEP/RAM relacionados.

## Concepto de Register Map

El servo organiza su información interna en un mapa de registros: :contentReference[oaicite:19]{index=19}  

- Cada dirección almacena un parámetro (modelo, versión, límites, PID, etc.).  
- Algunos registros son **RO** (solo lectura, por ejemplo, modelo o sensores). :contentReference[oaicite:20]{index=20}  
- Otros son **RW** (lectura y escritura) y permiten configurar el comportamiento. :contentReference[oaicite:21]{index=21}  

## Tipos de errores y protección

Los sensores internos permiten detectar: :contentReference[oaicite:22]{index=22}  

- **Error de temperatura excedida**: cuando se supera el límite configurado.  
- **Error de sobrecarga**: cuando el esfuerzo supera el umbral de sobrecarga
  durante el tiempo definido.  
- **Errores eléctricos / de alimentación**: tensión fuera del rango válido.

Estos estados se reflejan en:

- `Status Error` (RAM) – bits de error.  
- `Status Detail` (RAM) – información adicional.

La política de alarma (Alarm Policy) define qué errores provocan parada,
desactivación de torque o sólo notificación. (Ver tabla de registros ROM/RAM).

## Dead Zone, Max PWM y sobrecarga

El manual define además: :contentReference[oaicite:23]{index=23}  

- **Max PWM**: limita el valor máximo de PWM, reduciendo el par/velocidad
  máxima y optimizando el uso de energía.  
- **Dead Zone**: zona cercana a la posición objetivo donde la fuerza se
  considera cero; aumenta “flexibilidad operativa” y evita vibraciones cuando
  el error es muy pequeño.  
- **Umbral de sobrecarga PWM** y período de detección (ver registros RAM 23 y 37)
  permiten decidir cuándo una fuerza externa prolongada se considera
  sobrecarga.

El efecto combinado se puede ver en los gráficos del manual, pero a nivel de
librería basta con exponer funciones para leer/escribir estos registros y
documentar los rangos válidos.
