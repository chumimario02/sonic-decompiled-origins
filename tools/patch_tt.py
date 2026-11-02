#!/usr/bin/env python3
"""Patch Sonic Triple Trouble to skip the SEGA screen.

The smsggrecomp recompilation of TT gets stuck on the splash screen (its
frame/PCM IRQ path mis-executes in the generated code, while the interpreter
passes it). The game's boot dispatches through a state table; mode 0 is the
SEGA screen. Pointing mode 0 at the next state boots straight into the game.

Supported inputs (auto-detected):
  * Game Gear original  -> table entry 0 at 0x0E1D (0x0E53 -> 0x0F3E)
  * SMS fan conversion   -> table entry 0 at 0x0E25 (0x0E5B -> 0x0F46)

This produces a DERIVED ROM. Keep it local; never redistribute it.

Usage:
    python3 patch_tt.py <input.gg|input.sms> <output>
"""
import sys
import zlib

# (offset, expected 2 bytes, replacement 2 bytes)
SIGNATURES = [
    (0x0E1D, (0x53, 0x0E), (0x3E, 0x0F)),  # GG original
    (0x0E25, (0x5B, 0x0E), (0x46, 0x0F)),  # SMS hack
]


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    src, dst = sys.argv[1], sys.argv[2]
    data = bytearray(open(src, "rb").read())

    for off, expect, newval in SIGNATURES:
        if off + 2 <= len(data) and tuple(data[off:off + 2]) == expect:
            print("detected: %s at 0x%04X" % ("GG original" if off == 0x0E1D else "SMS hack", off))
            print("before: %s  ->  after: %s" % (tuple(data[off:off + 2]), newval))
            data[off], data[off + 1] = newval
            open(dst, "wb").write(data)
            print("wrote %s (crc32=0x%08X)" % (dst, zlib.crc32(data) & 0xFFFFFFFF))
            return 0

    print("error: not a recognized Sonic Triple Trouble ROM (no known state table)")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
