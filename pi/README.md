# Sonic Decompiled Origins — Raspberry Pi

Juegos nativos (no emulados):

| Juego | Motor | Binario |
|---|---|---|
| Sonic 1 | RSDKv4 | `Sonic12v4/linux-out/RSDKv4` |
| Sonic 2 | RSDKv4 | `Sonic12v4/linux-out-s2/RSDKv4` |
| Sonic 3 & Knuckles | Recomp (C) | `Sonic3AndKnucklesRecomp/linux-out/Sonic3KRecomp` |
| Sonic CD | RSDKv3 | `SonicCDv3/linux-out/RSDKv3` |
| Sonic Chaos | Recomp SMS/GG (C) | `SonicChaosRecomp/linux-out/SonicChaosRecomp` |
| Sonic Triple Trouble | Recomp GG (C) | `SonicTripleTroubleSMSRecomp/linux-out/SonicTripleTroubleSMSRecomp` |
| Sonic 2 SMS | Recomp SMS (C) | `Sonic2SMSRecomp/linux-out/Sonic2SMSRecomp` |
| Tails Adventure | Recomp GG (C) | `TailsAdventureRecomp/linux-out/TailsAdventureRecomp` |
| **Sonic 1 Forever** | RSDKv4 (Team Forever) | `SonicForever/s1fs2a` |

> **IMPORTANTE: necesita sistema de 64 bits (ARM64 / aarch64).**
> Raspberry Pi OS 64-bit sí; imágenes RetroPie de 32 bits (armhf) NO sirven
> con estos binarios.
>
> **glibc mínima: 2.38** para los RSDK (y 2.36+ para el launcher).
> Raspberry Pi OS **trixie (Debian 13, glibc 2.41)** o superior ✓
> Bookworm (glibc 2.36) **no** sirve para los RSDK.

## 1. Sistema recomendado

- **Raspberry Pi OS Lite 64-bit** (Bookworm/Trixie) — mejor rendimiento
- O **RetroPie sobre una base de 64 bits**

## 2. Dependencias (runtime)

```bash
sudo apt update
sudo apt install -y libsdl2-2.0-0 libglew2.2 libvorbis0a libvorbisfile3 \
                    libogg0 libtheora0 libgl1 libopengl0 zlib1g
```

> Sonic CD incluye un symlink `lib/libtheoradec.so.2 -> /usr/lib/aarch64-linux-gnu/libtheoradec.so.1`
> (la Pi trae `.so.1`; el launcher añade esa carpeta a `LD_LIBRARY_PATH`).

(Si tu distro usa otro nombre de versión de GLEW, instala `libglew2.2` o el que exista.)

## 3. Copia la carpeta

Coloca todo en `/home/pi/SonicOrigins/` con esta estructura:

```
/home/pi/SonicOrigins/
├── launcher                     # launcher ARM64
├── Sonic12v4/
│   ├── linux-out/               # RSDKv4 + Data.rsdk (Sonic 1) + settings.ini
│   └── linux-out-s2/            # RSDKv4 + Data.rsdk (Sonic 2) + settings.ini
├── SonicCDv3/
│   └── linux-out/               # RSDKv3 + Data.rsdk (CD) + settings.ini
├── Sonic3AndKnucklesRecomp/
│   └── linux-out/               # Sonic3KRecomp + sonic3k.bin + settings.ini + s3k.srm
├── SonicChaosRecomp/linux-out/
├── SonicTripleTroubleSMSRecomp/linux-out/
├── Sonic2SMSRecomp/linux-out/
├── TailsAdventureRecomp/linux-out/
└── SonicForever/
    ├── s1fs2a                   # motor RSDKv4 de Team Forever (ARM64)
    ├── Data.rsdk                # tu copia legal de Sonic 1 (2013)
    ├── Mods/                    # mod de Sonic 1 Forever (lo trae el juego)
    └── settings.ini
```

Marca permisos:

```bash
chmod +x /home/pi/SonicOrigins/launcher
chmod +x /home/pi/SonicOrigins/SonicForever/s1fs2a
```

### Sonic 1 Forever

El motor es el mismo RSDKv4, **el contenido de Forever es la carpeta `Mods/`**.
Para montarlo:

1. Descarga Sonic 1 Forever (versión 1.5.1) de Team Forever.
2. Copia su carpeta `Mods/` a `SonicForever/Mods/`.
3. Copia tu `Data.rsdk` de Sonic 1 (el mismo de `Sonic12v4/linux-out/`).
4. Listo: al elegir "SONIC 1 FOREVER" en el launcher carga el mod.

## 4. RetroPie → Ports

Copia los `.sh` de `pi/ports/` a `/home/pi/RetroPie/roms/ports/` y:

```bash
chmod +x /home/pi/RetroPie/roms/ports/*.sh
```

Aparecerán en **EmulationStation → Ports**:

- Sonic Decompiled Origins (menú)
- Sonic 1 / Sonic 2 / Sonic 3 & Knuckles / Sonic CD / Sonic 1 Forever (directo)

## 5. Ajustes de rendimiento (Zero 2 W)

En cada `settings.ini`:

```ini
UseHQModes=false
FullScreen=true
VSync=false
WindowScale=1
```

- `UseHQModes=false` aligera las 3D de los special stages.
- Si va justo, prueba `FullScreen=false` y ventana pequeña.

## 6. Notas

- **No se incluyen ROMs ni assets**: usa tu copia legal (`Data.rsdk`, `sonic3k.bin`,
  `sonicchaos.gg`, `sonictt.gg`, `Mods/` de Sonic 1 Forever).
- **Sonic Triple Trouble**: el motor se atasca en el SEGA screen del GG. Genera el ROM
  parcheado con `tools/patch_tt.py` (2 bytes) y usa ese `sonictt.gg`:
  ```bash
  python3 tools/patch_tt.py "Triple Trouble (GG).gg" roms/sonictt.gg
  ```
- Sonic CD: solo las cinemáticas (Theora) son pesadas; el gameplay va bien.
- Sonic 3 & K en Zero 2 W: puede ir justo de RAM; cierra otros servicios.
- Sonic 1 Forever en Zero 2 W: si va justo, baja `ScreenWidth` en su `settings.ini`.
