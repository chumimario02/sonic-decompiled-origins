# Sonic Decompiled Origins

Lanzador + builds **nativos** de los Sonic clásicos. No es un emulador: cada juego
se compila para tu máquina (Windows, Linux o Raspberry Pi).

| Juego | Motor | Tipo |
|---|---|---|
| Sonic 1 | RSDKv4 | decompilación (C++) |
| Sonic 2 | RSDKv4 | decompilación (C++) |
| Sonic 3 & Knuckles | segagenesisrecomp | recompilación estática (C) |
| Sonic CD | RSDKv3 | decompilación (C++) |
| Sonic Chaos | smsggrecomp | recompilación estática (C) |
| Sonic Triple Trouble | smsggrecomp | recompilación estática (C) |
| SRB2 (Sonic Robo Blast 2) | motor Doom (open source) | juego fan nativo |

## Lo que NO incluye

- **Ni ROMs ni datos de juego.** No se distribuye ningún asset con copyright.
- Cada usuario aporta sus copias legales:
  - Sonic 1 / 2 / CD → `Data.rsdk` (de tu copia de Sonic Origins / móvil)
  - Sonic 3 & K → `sonic3k.bin` (tu ROM)

## Plataformas

| Plataforma | Estado | Launcher |
|---|---|---|
| Windows x64 | ✅ | `launcher/launcher.c` (SDL2) |
| Linux x86_64 | ✅ | `launcher/launcher_linux.c` |
| Raspberry Pi ARM64 | ✅ | `launcher/launcher_linux.c` (cross) |

> Raspberry Pi: usa **Raspberry Pi OS 64-bit (trixie o superior)**. Los binarios
> ARM64 piden glibc ≤ 2.38 y **no** funcionan en sistemas de 32 bits.

## Estructura

```
sonic-decompiled-origins/
  launcher/
    launcher.c            launcher Windows
    launcher_linux.c      launcher Linux / Raspberry Pi
    build.sh              build Windows (MinGW)
    build-linux.sh        build Linux x86_64
    build-pi.sh           cross-build Raspberry Pi (ARM64)
  pi/
    README.md             guía de instalación en la Pi
    ports/*.sh            entradas para RetroPie → Ports
  scripts/                utilidades de descarga/compilación
  games/                  (ignorado) binarios compilados
  roms/                   (ignorado) ROMs/datos del usuario
```

## Compilar el launcher

### Windows (MinGW + SDL2)
```bash
cd launcher && ./build.sh
```

### Linux x86_64
```bash
sudo apt install gcc pkg-config libsdl2-dev
cd launcher && ./build-linux.sh
```

### Raspberry Pi (ARM64, cross desde PC)
```bash
sudo dpkg --add-architecture arm64
# añade un repo arm64 (ports.ubuntu.com / deb.debian.org) y luego:
sudo apt install gcc-aarch64-linux-gnu libsdl2-dev:arm64 pkg-config
cd launcher && ./build-pi.sh
```

Para compilar en la propia Pi:
```bash
sudo apt install gcc pkg-config libsdl2-dev
cd launcher && ./build-linux.sh
```

## Jugar en la Raspberry Pi

Sigue la guía completa: [`pi/README.md`](pi/README.md).

Resumen:
```bash
# dependencias
sudo apt install -y libsdl2-2.0-0 libglew2.2 libvorbis0a libvorbisfile3 \
                    libogg0 libtheora0 libgl1 libopengl0 zlib1g

# estructura en ~/SonicOrigins (launcher + los 4 juegos + sus datos)
# RetroPie: copia pi/ports/*.sh a ~/RetroPie/roms/ports/
```

## Controles del launcher

| Entrada | Acción |
|---|---|
| ↑/↓ o stick izquierdo | mover |
| Enter / A / Start | jugar |
| Alt+Enter | pantalla completa / ventana |
| Esc / B / Back | salir |

El launcher arranca **a pantalla completa** y adapta el diseño a pantallas
pequeñas (probado en 240×240).

## Créditos

- Recompilación estática: [mstan/segagenesisrecomp](https://github.com/mstan/segagenesisrecomp) y proyectos asociados.
- Decompilaciones RSDKv3/v4: [RSDKModding](https://github.com/RSDKModding).
- Sonic y todos los assets son propiedad de SEGA. Proyecto fan, sin ánimo de lucro.
