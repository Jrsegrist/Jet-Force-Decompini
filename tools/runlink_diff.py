#!/usr/bin/env python3
"""asm-differ support for runlink module functions (func_mNNN_XXXXX).

diff_settings.py calls apply() for every diff. When the function to diff
is a module function, it is looked up in build/runlink/modules.map (written
by tools/runlink_build.py) and asm-differ is pointed at:

  default  the built module images (build/runlink/modules.bin) against the
           module images in the ROM, at that function's offsets. These are
           the final bytes, so a function that diffs clean here matches.
  -o       the module's object (build/src/modules/mNNN.o when the module is
           C) against the module's assembly object (build/asm/modules/mNNN.o).
           Shows symbols, but references to a module's own data are spelt
           differently (`.data+0x20` from IDO, `rl_mNNN_XXXXX+0x20` in the
           assembly), so a few lines can differ in name only.

Run `make` first (or pass -m). Example:
  python3 tools/asm-differ/diff.py -mw func_m001_00120
"""
import os
import re
import struct

MAP = "build/runlink/modules.map"
BASEROM = "baserom.us.z64"
MODTAB, MODULES = 0x1ED2780, 0x1ED3B20  # US 1.0, see tools/runlink_split.py
FUNC_RE = re.compile(r"func_m(\d{3})_[0-9A-Fa-f]{5}$")


def find(name, mapfile=MAP):
    """(module, image offset, object, function offset, size) or None."""
    if not os.path.exists(mapfile):
        return None
    cur = None
    for line in open(mapfile):
        if line.startswith("#") or not line.strip():
            continue
        parts = line.split()
        if not line.startswith(" "):
            cur = (int(parts[0][1:]), int(parts[1], 16), parts[5])
        elif parts[0] == name and cur:
            return cur + (int(parts[1], 16), int(parts[2], 16))
    return None


def apply(config, args):
    start = getattr(args, "start", None)
    if not start or not FUNC_RE.match(start):
        return
    hit = find(start)
    if hit is None:
        return
    me, img, obj, foff, size = hit
    if getattr(args, "diff_obj", False):
        if not getattr(args, "file", None):
            args.file = obj
        if not getattr(args, "ref_file", None):
            args.ref_file = "build/asm/modules/m%03d.o" % me
        return
    rom = open(BASEROM, "rb").read(MODTAB + 32 * me)
    rom_img = struct.unpack_from(">I", rom, MODTAB + 32 * (me - 1) + 4)[0]
    config["myimg"] = os.path.join(os.path.dirname(MAP), "modules.bin")
    config["baseimg"] = BASEROM
    args.start = "0x%X" % (img + foff)
    if getattr(args, "end", None) is None:
        args.end = "0x%X" % (img + foff + size)
    args.base_shift = "0x%X" % (MODULES + rom_img - img)
