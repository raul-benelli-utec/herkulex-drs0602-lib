# Ejemplo: Brazo Robótico con HerkuleX DRS-0602

Este ejemplo demuestra el control de un brazo robótico de 5 grados de libertad utilizando servomotores HerkuleX DRS-0602.

---

## Cómo ver el menú del brazo (si te sale el menú de SCAN / SelfCheck)

Si al hacer **Upload and Monitor** en VS Code ves algo como *"1) SCAN IDs RAPIDO, 2) SCAN IDs COMPLETO..."*, estás en el proyecto **self_check**, no en el del brazo.

**Pasos en VS Code:**

1. **File → Open Folder** (o *Abrir carpeta*).
2. Ir a la carpeta del **brazo**, no a la raíz de la librería:
   ```
   .../herkulex-drs0602-lib/examples/proyecto_cobot
   ```
   (dentro de `examples` debe estar la carpeta `proyecto_cobot`; ábrela **esa**).
3. Esperar a que PlatformIO cargue (barra inferior con "megaatmega2560" o similar).
4. Conectar el Arduino por USB.
5. En la barra inferior de PlatformIO: **Upload** (→) y luego **Serial Monitor** (icono de enchufe).

Después de eso deberías ver el menú del brazo:

- `s` - START, `1` - POSE_INICIAL, `2` - POSE_TRABAJO, `3` - POSE_TRABAJO_2  
- `P` - Posiciones, `c` - Limpiar errores  
- `l`, `g`, `a`, `p`, `d`, `r` - Poses de usuario  

---

## Descripción

El ejemplo implementa una estructura organizada con:
- **Motor**: Estructura para control individual de cada servo
- **Brazo**: Estructura para control coordinado de todos los motores
- **Poses**: Definiciones de posiciones predefinidas (inicial, trabajo, standby)
- **PoseManager**: Grabación y reproducción de poses de usuario en RAM (slots 1–20, teach-in)

## Requisitos

### Hardware
- **Arduino Mega 2560** (o compatible)
- **5 servomotores HerkuleX DRS-0602**
- Conexión Serial1 (pines 18/19) a los servos
- Alimentación adecuada para los servos

### Software
- **PlatformIO** (recomendado) o Arduino IDE
- Librería `herkulex-drs0602-lib` (incluida en el proyecto)

## Compilación y Carga

### Con PlatformIO

```bash
cd examples/proyecto_cobot
pio run -t upload
```

### Con Arduino IDE

**No abras `src/main.cpp` directamente** — Arduino IDE no compila subcarpetas `src/` ni enlaza la librería sola.

1. Ejecutá el script de preparación (copia fuentes + instala la librería):
   ```bash
   cd examples/proyecto_cobot
   ./prepare_arduino_ide.sh
   ```
2. En Arduino IDE: **Archivo → Abrir** → `arduino_ide/sketch/proyecto_cobot.ino`
3. **Herramientas → Placa → Arduino Mega or Mega 2560**
4. **Herramientas → Puerto** → tu Mega por USB
5. **Subir** y abrir **Monitor Serie** a **115200** baud

La librería HerkuleX queda en `~/Arduino/libraries/HerkulexDRS0602/`. Si editás archivos en `src/` o `config/`, volvé a correr `./prepare_arduino_ide.sh` antes de subir.

### Calibración de seguridad (sobrecarga)

- Comando **`t`** en el monitor: muestra PWM actual de cada motor.
- Motor **base (ID 1)**: en demo, dispara si el PWM sube **`SAFETY_PWM_DELTA_BASE`** (+55) sobre el valor al inicio del movimiento.
- Ajustá en `config/brazo_config.h`: `SAFETY_PWM_MONITOR`, `SAFETY_PWM_DELTA_BASE`, `SAFETY_PWM_MAX`.
- Para la demo: bloqueá el giro lateral **durante** el movimiento (comando `2` o `3` desde web o serial).
- Si no dispara: bajá `SAFETY_PWM_DELTA_BASE` (ej. 40) o `SAFETY_PWM_MONITOR[0]` (ej. 120).
- Si dispara sin carga: subí esos valores.

## Advertencias Importantes

⚠️ **ANTES DE USAR:**

1. **IDs únicos**: Asegúrate de que cada servo tenga un ID único (1-5). No conectes dos servos con el mismo ID.

2. **No duplicar IDs**: No cambies el ID de un servo a uno que ya existe en la red. Usa el ejemplo `self_check` para verificar y cambiar IDs de forma segura.

3. **Límites de posición**: Los valores `POS_MIN` y `POS_MAX` en `config/brazo_config.h` están en **0** (sin límites). **DEBES calibrarlos en laboratorio** antes de usar el brazo para evitar daños mecánicos.

4. **Alimentación**: Asegúrate de que la fuente de alimentación pueda suministrar suficiente corriente para todos los servos simultáneamente.

## Cómo Probar

1. **Conectar hardware**: Conecta los servos a Serial1 y alimenta el sistema.

2. **Abrir monitor serial**: Configura el monitor a 115200 baudios.

3. **Enviar comandos** (menú por Serial a 115200 baud):
   - `s` → START: check de motores y luego ir a posición inicial
   - `1` → Ir a POSE_INICIAL
   - `2` → Ir a POSE_TRABAJO
   - `3` → Ir a POSE_TRABAJO_2
   - `P` → Mostrar posiciones de todos los motores
   - `c` → Limpiar errores de todos los motores
   - **Poses del usuario (RAM):** `l` listar, `g <n> [name]` grabar en slot, `a [name]` grabar auto, `p <n> <ms>` reproducir, `d <n>` borrar, `r <ms>` recorrer todas

4. **Observar movimiento**: El brazo debería moverse suavemente a la pose solicitada.

## Estructura del Proyecto

```
proyecto_cobot/
├── config/
│   ├── poses.h              # Definiciones de poses
│   └── brazo_config.h       # Configuración del brazo (IDs, límites)
├── src/
│   ├── motor.h / motor.cpp   # Estructura Motor
│   ├── brazo.h / brazo.cpp  # Estructura Brazo
│   └── main.cpp             # Programa principal
├── platformio.ini           # Configuración PlatformIO
├── README.md                # Este archivo
└── main_old.cpp             # Referencia histórica (no modificar)
```

## Notas

- **main_old.cpp**: Este archivo es solo referencia histórica. No debe modificarse ni borrarse.

- **Calibración pendiente**: Los límites de posición (`POS_MIN` / `POS_MAX`) deben calibrarse en laboratorio para cada motor según la mecánica del brazo.

- **Extensibilidad**: El código está diseñado para ser fácilmente extensible. Puedes agregar más poses, funciones de control avanzado, o integración con otros sistemas.

## Solución de Problemas

- **Servos no responden**: Verifica IDs, conexiones y alimentación.
- **Movimientos bruscos**: Ajusta `playtime` en `goPose()` o calibra límites.
- **Errores de comunicación**: Usa `self_check` para diagnosticar problemas.

## Próximos Pasos

- Calibrar límites de posición en laboratorio
- Agregar más poses según necesidades
- Implementar control de trayectorias suaves
- Agregar feedback de sensores

