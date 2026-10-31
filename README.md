# Sonic Decompiled Origins

Un lanzador para Raspberry Pi que reÃºne las **recompilaciones estÃ¡ticas** (ROM â†’ C
nativo) de los Sonic clÃ¡sicos, todas del mismo motor:

| Juego | Proyecto |
|---|---|
| Sonic the Hedgehog (Mega Drive) | [mstan/SonicTheHedgehogRecomp](https://github.com/mstan/SonicTheHedgehogRecomp) |
| Sonic the Hedgehog 2 (Mega Drive) | [mstan/SonicTheHedgehog2Recomp](https://github.com/mstan/SonicTheHedgehog2Recomp) |
| Sonic 3 & Knuckles (Mega Drive) | [mstan/Sonic3AndKnucklesRecomp](https://github.com/mstan/Sonic3AndKnucklesRecomp) |
| Sonic the Hedgehog (Master System) | [mstan/SonicTheHedgehogSMSRecomp](https://github.com/mstan/SonicTheHedgehogSMSRecomp) |

No es un emulador ni los remasters de 2013: el cÃ³digo 68K/Z80 del juego se traduce
a C y se compila de forma nativa. El motor (VDP, Z80, FM/PSG, mandos) tambiÃ©n es
propio.

## Lo que NO incluye

- **Ni ROMs ni binarios.** Cada uno compila en su mÃ¡quina y aporta sus propias
  copias legales de los juegos.
- Nada de Nintendo/Sega con copyright.

## Estructura

```
sonic-decompiled-origins/
  launcher/            lanzador SDL2 (teclado + mando)
  scripts/fetch.sh     clona los 4 proyectos y el motor compartido
  scripts/build-pi.sh  compila los 4 en la Pi
  games/               aquÃ­ quedan los binarios compilados
  roms/                aquÃ­ pones tus ROMs (no se sube al repo)
```

Nombres de ROM esperados (cualquier regiÃ³n, pero el mismo juego):

```
roms/sonic1.bin     Sonic the Hedgehog (Genesis/Mega Drive)
roms/sonic2.bin     Sonic the Hedgehog 2 (Genesis/Mega Drive)
roms/s3k.bin        Sonic 3 & Knuckles (ROM combinada)
roms/sonicsms.sms   Sonic the Hedgehog (Master System)
```

## Compilar en la Raspberry Pi

Requisitos: Raspberry Pi OS (64 bits recomendado), 2 GB de swap si usas una
Zero 2 W, y conexiÃ³n a internet.

```sh
sudo apt update
sudo apt install -y build-essential cmake ninja-build libsdl2-dev git pkg-config

./scripts/fetch.sh      # clona los 4 repos + submÃ³dulos
./scripts/build-pi.sh   # compila cada juego en games/
./launcher/build.sh     # compila el lanzador
./run.sh                # abre el menÃº
```

> En una Pi Zero 2 W la compilaciÃ³n es lenta (varios minutos por juego) y puede
> necesitar swap. Si tienes una Pi 4/5, compila ahÃ­ y copia la carpeta `games/`.

## Lanzador

- Flechas / cruceta: elegir juego.
- Enter / botÃ³n A: lanzar.
- Esc: salir.
- Muestra `(falta)` si no encuentra el binario o la ROM de ese juego.

## Estado

- [x] Estructura del proyecto y scripts.
- [x] Lanzador SDL2.
- [ ] Compilar y probar en la Pi (pendiente de hardware).
- [ ] CarÃ¡tulas y tema visual.
- [ ] Mando: navegaciÃ³n completa (ya soportada por SDL2).

## CrÃ©ditos

Todo el mÃ©rito de las recompilaciones es de **mstan** y del proyecto
[segagenesisrecomp](https://github.com/mstan/segagenesisrecomp).
*Sonic the Hedgehog* es una marca de SEGA. Proyecto fan, sin Ã¡nimo de lucro.

## Estado en Windows (probado)

| Juego | Estado | Notas |
|---|---|---|
| **Sonic 1** | ? funciona | compilado con MSVC + SDK |
| **Sonic SMS** | ? funciona | necesita `--window 3` |
| **Sonic 3 & K** | ?? compila | rechaza la ROM si el CRC32 no es `63522553` |
| **Sonic 2** | ?? no compila | submódulos rotos en el repo original |

### Receta que funcionó (MSVC)

1. Instalar **Visual Studio Build Tools** con el workload C++ y el **Windows SDK**.
2. Configurar con Ninja + `cl` dentro de `VsDevCmd.bat`, añadiendo a mano
   `INCLUDE`, `LIB` y el `bin\x64` del SDK (el SDK no se registra solo).
3. `SDL2_DIR` apuntando al paquete VC que trae el repo de Sonic 1
   (`segagenesisrecomp/runner/external/SDL2/SDL2-2.28.5/cmake`).
4. El lanzador debe:
   - pasar la **ROM en ruta absoluta** (los juegos la resuelven desde su carpeta),
   - lanzar con `CreateProcess` y `lpDirectory` = carpeta del proyecto (si el cwd
     es `games/`, el juego sale al instante),
   - para SMS: `--window 3`.
### Parches que hicieron falta

- **Sonic 2**: el repo apunta a un commit de `recomp-ui` que ya no existe en GitHub;
  se sustituye por el de Sonic 1 y se usa su motor con
  `-DGENESIS_RECOMP_ROOT`. Además hay que crear a mano las etiquetas
  `label_XXXX` que faltan en el C generado (huecos del recompilador).
- **CRC de la ROM**: el motor rechaza ROMs que no son la revisión exacta. Como el
  C se genera **desde nuestra propia ROM**, se cambia ese rechazo por un aviso en
  `segagenesisrecomp/runner/main.c` (bloque `expected_rom_crc32`).
- **Sonic 3 & K**: compila solo la variante combinada
  (`--target Sonic3KRecomp`); las variantes S3 y S&K piden sus propias ROMs.
### Sonic 2: siguiente paso (pendiente)

Compila y arranca, pero la pantalla sale negra. El runtime deja el diagnóstico en:

```
SonicTheHedgehog2Recomp/build-msvc/interior_label_misses.toml
```

Con estas 5 direcciones (destinos de tabla de salto dentro de funciones):

```
0x000C62 (en $000408)
0x00159E (en $001554)
0x001594 (en $001554)
0x0005E4 (en $000408)
0x03A1CC (en $015FCC)
```

`dispatch_misses.toml` está vacío (bien: no faltan funciones completas).
Avance: se añadieron las 5 direcciones como `extra_seeds` en el `[functions]`
del `game.toml` de Sonic 2. Resultado: el miss `$000C62` se resolvió; quedan
4 ($00159E, $001594 en $001554; $0005E4 en $000408; $03A1CC en $015FCC).
Próximo paso: revisar en `code_generator.c` (`seed_pc_idx_dispatch_targets`
/ `probe_pc_idx_targets`) por qué esos 4 seeds no se insertan en el CFG
(probablemente caen justo al otro lado del límite de su función, o el JMP
no es de la forma `(d8,PC,Xn)`). Herramientas: Ghidra para confirmar las
tablas y sus límites.