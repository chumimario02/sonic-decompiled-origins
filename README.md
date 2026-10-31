# Sonic Decompiled Origins

Un lanzador para Raspberry Pi que reúne las **recompilaciones estáticas** (ROM → C
nativo) de los Sonic clásicos, todas del mismo motor:

| Juego | Proyecto |
|---|---|
| Sonic the Hedgehog (Mega Drive) | [mstan/SonicTheHedgehogRecomp](https://github.com/mstan/SonicTheHedgehogRecomp) |
| Sonic the Hedgehog 2 (Mega Drive) | [mstan/SonicTheHedgehog2Recomp](https://github.com/mstan/SonicTheHedgehog2Recomp) |
| Sonic 3 & Knuckles (Mega Drive) | [mstan/Sonic3AndKnucklesRecomp](https://github.com/mstan/Sonic3AndKnucklesRecomp) |
| Sonic the Hedgehog (Master System) | [mstan/SonicTheHedgehogSMSRecomp](https://github.com/mstan/SonicTheHedgehogSMSRecomp) |

No es un emulador ni los remasters de 2013: el código 68K/Z80 del juego se traduce
a C y se compila de forma nativa. El motor (VDP, Z80, FM/PSG, mandos) también es
propio.

## Lo que NO incluye

- **Ni ROMs ni binarios.** Cada uno compila en su máquina y aporta sus propias
  copias legales de los juegos.
- Nada de Nintendo/Sega con copyright.

## Estructura

```
sonic-decompiled-origins/
  launcher/            lanzador SDL2 (teclado + mando)
  scripts/fetch.sh     clona los 4 proyectos y el motor compartido
  scripts/build-pi.sh  compila los 4 en la Pi
  games/               aquí quedan los binarios compilados
  roms/                aquí pones tus ROMs (no se sube al repo)
```

Nombres de ROM esperados (cualquier región, pero el mismo juego):

```
roms/sonic1.bin     Sonic the Hedgehog (Genesis/Mega Drive)
roms/sonic2.bin     Sonic the Hedgehog 2 (Genesis/Mega Drive)
roms/s3k.bin        Sonic 3 & Knuckles (ROM combinada)
roms/sonicsms.sms   Sonic the Hedgehog (Master System)
```

## Compilar en la Raspberry Pi

Requisitos: Raspberry Pi OS (64 bits recomendado), 2 GB de swap si usas una
Zero 2 W, y conexión a internet.

```sh
sudo apt update
sudo apt install -y build-essential cmake ninja-build libsdl2-dev git pkg-config

./scripts/fetch.sh      # clona los 4 repos + submódulos
./scripts/build-pi.sh   # compila cada juego en games/
./launcher/build.sh     # compila el lanzador
./run.sh                # abre el menú
```

> En una Pi Zero 2 W la compilación es lenta (varios minutos por juego) y puede
> necesitar swap. Si tienes una Pi 4/5, compila ahí y copia la carpeta `games/`.

## Lanzador

- Flechas / cruceta: elegir juego.
- Enter / botón A: lanzar.
- Esc: salir.
- Muestra `(falta)` si no encuentra el binario o la ROM de ese juego.

## Estado

- [x] Estructura del proyecto y scripts.
- [x] Lanzador SDL2.
- [ ] Compilar y probar en la Pi (pendiente de hardware).
- [ ] Carátulas y tema visual.
- [ ] Mando: navegación completa (ya soportada por SDL2).

## Créditos

Todo el mérito de las recompilaciones es de **mstan** y del proyecto
[segagenesisrecomp](https://github.com/mstan/segagenesisrecomp).
*Sonic the Hedgehog* es una marca de SEGA. Proyecto fan, sin ánimo de lucro.
