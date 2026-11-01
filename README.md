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

## Estado en Windows (probado)

| Juego | Estado | Notas |
|---|---|---|
| **Sonic 1** | ? funciona | compilado con MSVC + SDK |
| **Sonic SMS** | ? funciona | necesita `--window 3` |
| **Sonic 3 & K** | ? funciona | rechaza la ROM si el CRC32 no es `63522553` |
| **Sonic 2** | ? funciona | Emerald Hill Act 1 y 2 completos, jefe incluido |
| **Sonic CD** | ? funciona | decomp RSDKv3 nativo (no emulador); necesita `Data.rsdk` |

### Receta que funcion� (MSVC)

1. Instalar **Visual Studio Build Tools** con el workload C++ y el **Windows SDK**.
2. Configurar con Ninja + `cl` dentro de `VsDevCmd.bat`, a�adiendo a mano
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
  `-DGENESIS_RECOMP_ROOT`. Adem�s hay que crear a mano las etiquetas
  `label_XXXX` que faltan en el C generado (huecos del recompilador).
- **CRC de la ROM**: el motor rechaza ROMs que no son la revisi�n exacta. Como el
  C se genera **desde nuestra propia ROM**, se cambia ese rechazo por un aviso en
  `segagenesisrecomp/runner/main.c` (bloque `expected_rom_crc32`).
- **Sonic 3 & K**: compila solo la variante combinada
  (`--target Sonic3KRecomp`); las variantes S3 y S&K piden sus propias ROMs.
### Sonic 2: como se resolvio (resumen)

El motor de recompilacion estatica no ve todos los destinos de salto de las
maquinas de estado de los objetos (patron `move.w Tbl(pc,dN.w),dM` +
`jmp/jsr Tbl(pc,dM.w)`). Cada destino no visto se reportaba en tiempo de
ejecucion como `interior-label miss` y el objeto se quedaba "colgado"
(badniks sin IA, jefes sin secuencia de derrota, colisiones raras).

Tres arreglos, de menos a mas general:

1. **Parche de motor** (`profiles/genesis/code_generator.c`): tras ordenar las
   direcciones de instruccion se descartan las que caen **a mitad** de otra
   instruccion ya decodificada. Son semillas fantasma de un desensamblado de
   otra revision; emitirlas decodificaba los bytes de un operando como opcode
   (p.ej. `$F66A`) y provocaba `[ILLEGAL]` al llegar ahi por flujo lineal.

2. **Auto-descubrimiento** (`[game] jump_table_autodiscovery = true`) para las
   tablas `[[jump_table]]` que el usuario no declara a mano.

3. **Escaneo estatico de tablas** (`scripts/scan_tables.py`, ver abajo): recorre
   el ROM buscando `4E FB` (`jmp`) y `4E BB` (`jsr`) con EA `(d8,PC,Dn.w)`,
   calcula la base de la tabla a partir de la palabra de extension y enumera
   todos los destinos. En Sonic 2 salen **385 tablas / 2530 destinos**.

Con las 1518 etiquetas `Obj*` del desensamblado + los 2530 destinos de tabla,
el `game.toml` acaba con ~3900 entradas en `[functions].extra` y `extra_seeds`.
Resultado: Sonic y Tails jugables, badniks con IA, espiral de Emerald Hill,
puentes, cartel de meta y jefe del Act 2 con su secuencia de derrota.

Scripts del bucle (en `%TEMP%\opencode\`, no versionados):

| Script | Para que |
|---|---|
| `add_misses.ps1 -ErrLog <log>` | vuelca los `interior-label miss` del log al `game.toml` |
| `build-s2.ps1` | build en dos pasadas + parche `label_XXXX` + enlazado |
| `scan_tables.py` | escaneo estatico de tablas `(d8,PC,Dn.w)` |

Nota: el build regenera el C en cada cambio del `game.toml`, y eso borra las
etiquetas `label_XXXX` insertadas a mano; por eso el build es de dos pasadas.

---

### Sonic CD (decomp RSDKv3, aparte de los recompilados)

Sonic CD **no** va por recompilacion estatica: el Mega CD tiene un segundo
68000 y el chip ASIC de escalado, y el motor de mstan no lo soporta. Se usa el
decomp nativo en C++ de [RSDKv3-Decompilation](https://github.com/RSDKModding/RSDKv3-Decompilation),
que ademas es el camino bueno para Android y Raspberry Pi (probado en ARM por
PortMaster).

Necesita `Data.rsdk` (assets del Sonic CD de 2011: Steam, Sonic Origins o el
APK de Android renombrando `Data.rsdk.xmf` -> `Data.rsdk`). **El `.chd` de
Mega CD NO sirve**: es otro formato de datos.

Receta en Windows con MSYS2/MinGW (mas ligero que vcpkg):

```bash
# dependencias
pacman -S --needed mingw-w64-x86_64-glew mingw-w64-x86_64-libogg \
  mingw-w64-x86_64-libvorbis mingw-w64-x86_64-libtheora \
  mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja

# configurar (cmake-shims/ aporta los .cmake de Vorbis y Theora,
# que MSYS2 no trae para las libs de sonido)
cmake -B build-mingw -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
  -DCMAKE_PREFIX_PATH="$PWD/cmake-shims;C:/msys64/mingw64"

cmake --build build-mingw
```

Dos parches necesarios (ya aplicados en el clon local):

1. `platforms/windows.cmake`: `/subsystem:windows` (MSVC) -> `-mwindows` en GCC.
2. `RSDKv3/fcaseopen.c`: incluir `<direct.h>` en `_WIN32` para que `chdir` este
   declarado en MinGW (GCC 14+ lo trata como error).

Empaquetado en `games/soniccd/`: `SonicCD.exe` + `Data.rsdk` + `SDL2.dll`,
`glew32.dll`, `libogg-0`, `libvorbis-0`, `libvorbisfile-3`, `libtheora-1`,
`libtheoradec-2`, `libtheoraenc-2`, `libwinpthread-1`, `libgcc_s_seh-1`,
`libstdc++-6`.

### Sonic 2: el bug de las tablas (por que se veia "sucio")

Sintoma: logos y titulo con basura, sprites corruptos, y crashes
`[ILLEGAL] ... opcode=FFA8` en la Special Stage.

Causa: las tablas de estados de Sonic 2 son `dc.w Handler-Tbl`, y las entradas
de relleno valen `$0000`. Como `Tbl + 0 == Tbl`, el enumerador de tablas
registraba **la propia tabla** como destino de codigo. El recompilador emitia
los bytes de la tabla como instrucciones y, si un objeto caia ahi, caminaba la
tabla hasta dar con una palabra de datos como opcode.

Dos arreglos:

1. **Datos**: 349 + 283 direcciones que caian dentro de una tabla se quitaron
   de `game.toml` y de `sonic2.disasm_labels.toml`, y las 349 del toml se
   anadieron a `[functions].blacklist`. (El fichero de etiquetas se fusiona por
   separado: limpiar solo el `game.toml` no bastaba.)
2. **Motor** (`code_generator.c`): los dos enumeradores de tablas de offsets
   ahora ignoran las entradas con offset `0` (`if (off == 0) continue;`), asi
   que el fallo no puede reaparecer en ningun juego.

Efecto secundario del arreglo: el codigo fantasma escribia basura en la VDP.
Al quitarlo, el logo de SEGA, la pantalla de titulo y los sprites se ven limpios.

### Level Select sin pasar por Options

El flag vive en RAM en `$FFFFD0` (el operando `.w` `$FFD0` se extiende con
signo: `$FFFFD0`, no `$00FFD0`). El motor tiene un poke de RAM:

```
GENESIS_POKE=FFFFD0=0100 GENESIS_POKE_FRAMES=3000
```

El launcher lo aplica automaticamente al lanzar SONIC 2. En el titulo hay que
mantener **A** (tecla Z) y pulsar **Start** (Enter).

### Diagnosticar cuelgues (no crashes)

`GENESIS_PC_LOG=N` imprime cada N*10000 instrucciones el anillo de ultimas
funciones ejecutadas (`crash_report_recent_blocks`). Muestrear desde
`glue_check_vblank` (no desde el servicio de VBlank) es lo que hace que se vea
codigo del juego y no el handler de VBlank. Un bucle de espera aparece como un
punado de direcciones repitiendose.

### El juego ya no muere al pisar datos

`m68k_illegal_trap` hacia `abort()`. Como el codigo generado emite la llamada
inmediatamente antes de un `return`, basta con registrarla y volver: la funcion
apestosa se desenrolla y el juego sigue (con glitches visuales como mucho).
`GENESIS_TRAP_ABORT=1` restaura el aborto duro para depurar.

Ojo: la primera limpieza de tablas se revirtio porque el filtro "destino dentro
de una tabla" usaba `longitud = primera_entrada/2`, y eso sobreestima cuando la
primera entrada apunta lejos. Borro rutinas reales (agua, boosters, plataformas).
El arreglo bueno es el del motor: ignorar solo las entradas con offset 0.
