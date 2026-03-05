# Guía: Crear repo de tesis y vincular la librería

Pasos para tener dos repos: la librería (publicable) y la tesis (con la librería incluida).

---

## Paso 0: Subir la librería a GitHub (si aún no está)

1. **En GitHub (web):** Crear un nuevo repositorio
   - Ve a https://github.com/new
   - Nombre: `herkulex-drs0602-lib` (o el que prefieras)
   - Descripción: "Librería Arduino para servomotores HerkuleX DRS-0602"
   - **No** marques "Add README" (ya tienes uno)
   - Clic en **Create repository**

2. **En GitHub Desktop** (o terminal):
   - Abre el repo `herkulex-drs0602-lib`
   - **Repository → Repository Settings** (o Ctrl+,)
   - En "Primary remote repository", pega la URL que te dio GitHub (ej: `https://github.com/TU_USUARIO/herkulex-drs0602-lib.git`)
   - O en terminal:
     ```bash
     cd /ruta/a/herkulex-drs0602-lib
     git remote add origin https://github.com/TU_USUARIO/herkulex-drs0602-lib.git
     ```

3. **Hacer commit de los cambios pendientes** (si los hay):
   - En GitHub Desktop: revisa los archivos, escribe mensaje, **Commit to main**

4. **Publicar / Push:**
   - En GitHub Desktop: **Publish repository** (si es la primera vez) o **Push origin**
   - En terminal: `git push -u origin main`

---

## Paso 1: Crear el repo de la tesis en GitHub

1. Ve a https://github.com/new
2. Nombre: `tesis-cobot-herkulex` (o el que prefieras)
3. Descripción: "Tesis - Control de brazo robótico con HerkuleX DRS-0602"
4. **Sí** marca "Add a README file" (para que no esté vacío)
5. Clic en **Create repository**

---

## Paso 2: Clonar el repo de la tesis en tu máquina

**Opción A - GitHub Desktop:**
1. **File → Clone repository**
2. Pestaña **GitHub.com**
3. Busca `tesis-cobot-herkulex` y selecciónalo
4. Local path: `/home/raul/Documentos/Desarrollo_soluciones/Proyectos/`
5. **Clone**

**Opción B - Terminal:**
```bash
cd /home/raul/Documentos/Desarrollo_soluciones/Proyectos/
git clone https://github.com/TU_USUARIO/tesis-cobot-herkulex.git
cd tesis-cobot-herkulex
```

---

## Paso 3: Añadir la librería como submodule

Un **submodule** es un repo dentro de otro. La tesis tendrá una carpeta que apunta a tu librería.

**En terminal** (GitHub Desktop no gestiona submodules bien, hay que usar terminal o VS Code):

```bash
cd /home/raul/Documentos/Desarrollo_soluciones/Proyectos/tesis-cobot-herkulex

# Añadir la librería como submodule (usa TU URL de GitHub)
git submodule add https://github.com/TU_USUARIO/herkulex-drs0602-lib.git codigo/herkulex-drs0602-lib
```

Esto crea la carpeta `codigo/herkulex-drs0602-lib` con el contenido de la librería.

**Commit y push:**
```bash
git add .
git commit -m "Añadir librería herkulex-drs0602-lib como submodule"
git push
```

---

## Paso 4: Estructura inicial del repo de tesis

Puedes crear estas carpetas en el repo de la tesis:

```
tesis-cobot-herkulex/
├── codigo/
│   └── herkulex-drs0602-lib/   ← submodule (se añade en Paso 3)
├── docs/                       ← capítulos, borradores
├── templates/                  ← plantillas de entrega
├── normativa/                  ← guías de la universidad
└── README.md
```

Crea las carpetas vacías con un `.gitkeep` para que Git las trackee:

```bash
cd /home/raul/Documentos/Desarrollo_soluciones/Proyectos/tesis-cobot-herkulex
mkdir -p docs templates normativa
touch docs/.gitkeep templates/.gitkeep normativa/.gitkeep
git add .
git commit -m "Estructura inicial: docs, templates, normativa"
git push
```

---

## Resumen de comandos (todo en uno)

```bash
# 1. En herkulex-drs0602-lib - añadir remote y push (cambia TU_USUARIO)
cd /home/raul/Documentos/Desarrollo_soluciones/Proyectos/herkulex-drs0602-lib
git remote add origin https://github.com/TU_USUARIO/herkulex-drs0602-lib.git
git add .
git commit -m "Preparar para publicación"
git push -u origin main

# 2. Clonar tesis (o si ya lo clonaste, saltar)
cd /home/raul/Documentos/Desarrollo_soluciones/Proyectos/
git clone https://github.com/TU_USUARIO/tesis-cobot-herkulex.git
cd tesis-cobot-herkulex

# 3. Añadir submodule
git submodule add https://github.com/TU_USUARIO/herkulex-drs0602-lib.git codigo/herkulex-drs0602-lib

# 4. Estructura
mkdir -p docs templates normativa
touch docs/.gitkeep templates/.gitkeep normativa/.gitkeep
git add .
git commit -m "Añadir librería como submodule y estructura de carpetas"
git push
```

---

## Notas importantes

- **Submodule:** Cuando clones el repo de la tesis en otra máquina, haz `git submodule update --init` para descargar la librería.
- **Actualizar la librería en la tesis:** Entra en `codigo/herkulex-drs0602-lib`, haz `git pull`, vuelve al repo tesis, commit y push.
- **GitHub Desktop:** Puedes usarlo para commits y push normales. Los submodules se gestionan mejor desde terminal o VS Code.
