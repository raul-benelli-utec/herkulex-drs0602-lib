# Compilación con PlatformIO

Esta guía explica cómo compilar y subir los ejemplos a Arduino Mega 2560 usando PlatformIO en VSCode.

## Requisitos Previos

1. **VSCode** instalado
2. **PlatformIO IDE** (extensión para VSCode)
3. **Arduino Mega 2560** conectado por USB

## Estructura de Proyectos

Los ejemplos están organizados como proyectos independientes dentro de `examples/`:

```
examples/
├── self_check/          # Scanner y tester de servos
│   ├── self_check.ino
│   └── platformio.ini
└── proyecto_cobot/      # Control de brazo robótico
    ├── src/
    ├── config/
    └── platformio.ini
```

## Cómo Compilar y Subir

### Opción 1: Desde VSCode (Recomendado)

1. **Abrir el proyecto**:
   - Abre VSCode
   - File → Open Folder
   - Selecciona la carpeta del ejemplo que quieres compilar:
     - `examples/self_check/` para el self-check
     - `examples/proyecto_cobot/` para el brazo robótico

2. **Compilar**:
   - Presiona `Ctrl+Alt+B` (o `Cmd+Alt+B` en Mac)
   - O haz clic en el ícono ✓ en la barra inferior de PlatformIO
   - O usa el menú: PlatformIO → Build

3. **Subir al Arduino**:
   - Presiona `Ctrl+Alt+U` (o `Cmd+Alt+U` en Mac)
   - O haz clic en el ícono → en la barra inferior
   - O usa el menú: PlatformIO → Upload

4. **Abrir monitor serial**:
   - Presiona `Ctrl+Alt+S` (o `Cmd+Alt+S` en Mac)
   - O haz clic en el ícono 🔌 en la barra inferior
   - O usa el menú: PlatformIO → Serial Monitor

### Opción 2: Desde Terminal

1. **Navegar al proyecto**:
   ```bash
   cd examples/self_check
   # o
   cd examples/proyecto_cobot
   ```

2. **Compilar**:
   ```bash
   pio run
   ```

3. **Subir**:
   ```bash
   pio run -t upload
   ```

4. **Monitor serial**:
   ```bash
   pio device monitor
   ```

## Configuración

### Arduino Mega 2560

Ambos proyectos están configurados para **Arduino Mega 2560**:

```ini
[env:megaatmega2560]
platform = atmelavr
board = megaatmega2560
framework = arduino
monitor_speed = 115200
```

### Include Paths

Los `platformio.ini` están configurados con los paths correctos para encontrar la librería:

- **self_check**: `-I../../src` (apunta a `src/herkulex_utils.h`)
- **proyecto_cobot**: 
  - `-I../../src` (librería)
  - `-Iconfig` (archivos de configuración)
  - `-Isrc` (código fuente)

## Solución de Problemas

### Error: "No se encuentra herkulex_utils.h"

**Causa**: Los include paths no están configurados correctamente.

**Solución**: 
- Verifica que el `platformio.ini` tenga los paths correctos
- Asegúrate de estar compilando desde la carpeta del ejemplo (no desde la raíz)

### Error: "Board not found"

**Causa**: La plataforma atmelavr no está instalada.

**Solución**:
```bash
pio platform install atmelavr
```

### Error: "Port not found"

**Causa**: El Arduino no está conectado o el puerto está ocupado.

**Solución**:
- Verifica que el Arduino esté conectado por USB
- Cierra otros programas que puedan usar el puerto (Arduino IDE, otros monitores serial)
- Verifica el puerto en PlatformIO: `pio device list`

### El código compila pero no se sube

**Causa**: Problemas con el bootloader o el puerto.

**Solución**:
- Presiona el botón RESET del Arduino justo antes de subir
- Verifica que el puerto sea correcto en `platformio.ini` (opcional):
  ```ini
  upload_port = /dev/ttyUSB0  # Linux
  upload_port = COM3          # Windows
  ```

## Verificación

Después de subir el código:

1. **Abre el monitor serial** (115200 baudios)
2. **Para self_check**: Deberías ver el menú principal
3. **Para proyecto_cobot**: Deberías ver el mensaje de inicio y el menú

## Notas

- **No compiles desde la raíz del proyecto**: Cada ejemplo debe compilarse desde su propia carpeta
- **Serial1**: Los servos se comunican por Serial1 (pines 18/19 en Mega)
- **Serial**: La comunicación con PC es por Serial (USB)

## Próximos Pasos

- Para probar servos individuales: usa `self_check`
- Para controlar el brazo completo: usa `proyecto_cobot`

