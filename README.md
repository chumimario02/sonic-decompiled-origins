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
