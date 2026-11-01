"""Scan a 68k ROM for every `jmp $tbl(pc,Dn.w)` dispatch table and enumerate all
of its targets. Sonic's state machines look like:

    moveq   #0,d0
    move.b  $24(a0),d0
    move.w  Tbl(pc,d0.w),d1
    jmp     Tbl(pc,d1.w)        ; 4E FB <disp16>  -> Tbl = (pc+2) + disp
Tbl:
    dc.w    Handler0-Tbl
    dc.w    Handler1-Tbl
    ...

The recompiler's static walker only follows the first few, so deep states
(boss defeat sequences, boss weapons, ...) end up as runtime "interior label
misses".  Enumerating them from the ROM up front fixes every object at once.
"""
import sys
from capstone import Cs, CS_ARCH_M68K, CS_MODE_BIG_ENDIAN, CS_MODE_M68K_000

ROM_PATH = sys.argv[1]
OUT_PATH = sys.argv[2] if len(sys.argv) > 2 else None

rom = bytearray(open(ROM_PATH, "rb").read())
rom_size = len(rom)

md = Cs(CS_ARCH_M68K, CS_MODE_BIG_ENDIAN | CS_MODE_M68K_000)
md.detail = False


def dec(addr):
    for ins in md.disasm(bytes(rom[addr:addr + 16]), addr):
        return ins
    return None


def word(addr):
    if addr < 0 or addr + 1 >= rom_size:
        return None
    return (rom[addr] << 8) | rom[addr + 1]


targets = set()
tables = 0
skipped = 0

# `jmp/jsr $tbl(pc,Dn.w)`: 4E FB / 4E BB <ext16>
# ext bits: (ext>>12)&7 = Dn, bit11 = size (0 => .W), bit8 = 0 => brief format,
# disp8 = signed low byte.  (full-format extension words are skipped)
for a in range(rom_size - 6):
    if rom[a] == 0x4E and rom[a + 1] in (0xFB, 0xBB):
        ext = (rom[a + 2] << 8) | rom[a + 3]
        if ext & 0x0100:            # full extension word, not a simple table
            skipped += 1
            continue
        if ext & 0x0800:            # .L index, not the word-offset table idiom
            skipped += 1
            continue
        disp = ext & 0xFF
        if disp & 0x80:
            disp -= 0x100
        base = a + 2 + disp          # table address (pc base is instr+2)
        if not (0 <= base < rom_size - 4):
            skipped += 1
            continue
        first = word(base)
        if first is None:
            skipped += 1
            continue
        # entry 0 normally points just past the table -> table length = first/2
        n = first // 2
        if not (1 <= n <= 96):
            # fall back: walk entries while they stay in ROM
            n = 0
            for i in range(64):
                v = word(base + i * 2)
                if v is None:
                    break
                t = base + v
                if not (0 <= t < rom_size) or v == 0:
                    break
                n = i + 1
            if n == 0:
                skipped += 1
                continue
        tables += 1
        for i in range(n):
            v = word(base + i * 2)
            if v is None:
                break
            t = base + v
            if not (0 <= t < rom_size - 2):
                continue
            ins = dec(t)
            if ins is None or ins.size > 10:
                continue
            targets.add(t)

sys.stderr.write("tables=%d  targets=%d  skipped=%d\n" % (tables, len(targets), skipped))

out = ["# auto-scanned JMP (pc,Dn.w) dispatch targets",
       "interior_targets = ["]
for t in sorted(targets):
    out.append("    0x%06X," % t)
out.append("]")
text = "\n".join(out) + "\n"
if OUT_PATH:
    open(OUT_PATH, "w").write(text)
else:
    sys.stdout.write(text)
