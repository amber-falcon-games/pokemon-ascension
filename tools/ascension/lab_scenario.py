#!/usr/bin/env python3
"""Battle Lab scenario files -> Lab ROM (F011, decision D022).

Scenarios are JSON files in lab/scenarios/. This tool checks them and writes them into the
reserved, dev-only data block of a *copy* of the dev ROM; the Battle Lab reads that block at
start. Nothing goes into any save file, and release ROMs have no block.

  lab_scenario.py check FILE...
  lab_scenario.py inject --rom build/pokemon-ascension.gba --out build/pokemon-ascension-lab.gba \
                         [--open ID | --open builtin | --no-open] FILE...

Names (SPECIES_*, MOVE_*, ITEM_*, NATURE_*) are resolved with the project's own headers by
compiling a tiny probe with arm-none-eabi-gcc (set $ARM_GCC or have it on PATH), so ids always
match the ROM being patched. Format of a scenario file: see lab/scenarios/README.md.
"""
import argparse
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
MARKER = b"ASCENSION-LAB-01"
BLOCK_SIZE = 4096
HEADER_SIZE = 4
VERSION = 1
RECORD_SIZE = 280
MAX_SCENARIOS = (BLOCK_SIZE - len(MARKER) - HEADER_SIZE) // RECORD_SIZE   # 14
PARTY_SIZE = 6
SLOT_ACTIVE = 1

# Must match include/battle_lab.h
FORMATS = ["trainer_single", "trainer_double", "wild_single"]
WEATHERS = ["none", "rain", "sun", "sand", "hail", "snow"]
TERRAINS = ["none", "electric", "grassy", "misty", "psychic"]
AIS = ["scenario", "smart", "random"]
COMBATS = ["classic", "stamina"]   # F017 spike (D023)
ENERGY_SCALES = {1: 0, 1.5: 1, 2: 2, 3: 3}        # max energy multiplier -> 2-bit index
DAMAGE_SCALES = {100: 0, 75: 1, 50: 2, 33: 3}     # damage percent
COST_SCALES = {100: 0, 75: 1, 50: 2, 25: 3}       # energy cost percent


def pick_scale(value, table, default, what):
    if value is None:
        value = default
    if value not in table:
        raise ScenarioError(f"{what} must be one of {', '.join(str(k) for k in table)}")
    return table[value]


def pack_combat(rules, where):
    """The combat byte: bit 0 Classic/Stamina, bits 1-2 energy, 3-4 damage, 5-6 cost."""
    return (pick(rules.get("combat"), COMBATS, f"{where} combat")
            | pick_scale(rules.get("energy"), ENERGY_SCALES, 1, f"{where} energy") << 1
            | pick_scale(rules.get("damage"), DAMAGE_SCALES, 100, f"{where} damage") << 3
            | pick_scale(rules.get("cost"), COST_SCALES, 100, f"{where} cost") << 5)
STATUSES = ["none", "poison", "toxic", "burn", "paralysis", "sleep", "freeze"]
ABILITY_SLOTS = {1: 0, 2: 1, "1": 0, "2": 1, "hidden": 2}


class ScenarioError(Exception):
    pass


def constant(prefix, name):
    """'Swampert' / 'ICE_BEAM' / 'ice beam' -> 'SPECIES_SWAMPERT' style constant name."""
    n = re.sub(r"[^A-Za-z0-9]+", "_", str(name).strip()).strip("_").upper()
    return n if n.startswith(prefix + "_") else prefix + "_" + n


def load_charmap():
    table = {}
    with open(os.path.join(ROOT, "charmap.txt"), encoding="utf-8") as f:
        for line in f:
            m = re.match(r"^'(.)'\s*=\s*([0-9A-Fa-f]{2})\s*$", line)
            if m and m.group(1) not in table:
                table[m.group(1)] = int(m.group(2), 16)
    table[" "] = 0x00
    return table


def encode_text(text, size, charmap, what):
    out = bytearray()
    for ch in text:
        if ch not in charmap:
            raise ScenarioError(f"{what}: character {ch!r} is not in the game font")
        out.append(charmap[ch])
    if len(out) > size - 1:
        raise ScenarioError(f"{what}: {text!r} is longer than {size - 1} characters")
    out.append(0xFF)                                   # EOS
    return bytes(out) + b"\xFF" * (size - len(out))


def parse_scenario(path):
    with open(path, encoding="utf-8") as f:
        try:
            data = json.load(f)
        except json.JSONDecodeError as e:
            raise ScenarioError(f"{path}: not valid JSON ({e})")
    for key in ("id", "player", "opponent"):
        if key not in data:
            raise ScenarioError(f"{path}: missing \"{key}\"")
    for side in ("player", "opponent"):
        if not isinstance(data[side], list) or not 1 <= len(data[side]) <= PARTY_SIZE:
            raise ScenarioError(f"{path}: \"{side}\" needs 1 to {PARTY_SIZE} Pokémon")
    data["_path"] = path
    return data


def needed_constants(scenarios):
    names = set()
    for sc in scenarios:
        for side in ("player", "opponent"):
            for mon in sc[side]:
                names.add(constant("SPECIES", mon["species"]))
                for mv in mon.get("moves", []):
                    if mv:
                        names.add(constant("MOVE", mv))
                if mon.get("item"):
                    names.add(constant("ITEM", mon["item"]))
                if mon.get("nature"):
                    names.add(constant("NATURE", mon["nature"]))
    return sorted(names)


def resolve_constants(names, gcc):
    """Values of the given constants from the project's headers (compiled, not parsed)."""
    if not names:
        return {}
    src = ('#include "constants/global.h"\n#include "constants/species.h"\n#include "constants/moves.h"\n'
           '#include "constants/items.h"\n#include "constants/pokemon.h"\n'
           "const unsigned int probe[] = {\n" + ",\n".join(names) + "\n};\n")
    with tempfile.TemporaryDirectory() as tmp:
        c, o, b = (os.path.join(tmp, n) for n in ("probe.c", "probe.o", "probe.bin"))
        with open(c, "w") as f:
            f.write(src)
        cmd = [gcc, "-c", "-O0", "-mthumb", "-mcpu=arm7tdmi", "-std=gnu17",
               "-iquote", os.path.join(ROOT, "include"), "-iquote", ROOT,
               "-DMODERN=1", "-DTESTING=0", "-DEMERALD", c, "-o", o]
        try:
            res = subprocess.run(cmd, capture_output=True, text=True)
        except FileNotFoundError:
            raise ScenarioError(f"{gcc} not found: run ./scripts/setup.sh, or set ARM_GCC")
        if res.returncode != 0:
            unknown = sorted(set(re.findall(r"'([A-Z0-9_]+)' undeclared", res.stderr)))
            if unknown:
                raise ScenarioError("unknown name(s): " + ", ".join(unknown))
            raise ScenarioError("could not resolve names:\n" + res.stderr[-2000:])
        objcopy = gcc[: -len("gcc")] + "objcopy" if gcc.endswith("gcc") else "arm-none-eabi-objcopy"
        subprocess.run([objcopy, "-O", "binary", "-j", ".rodata", o, b], check=True)
        with open(b, "rb") as f:
            raw = f.read()
    values = struct.unpack("<%dI" % len(names), raw[: 4 * len(names)])
    return dict(zip(names, values))


def pick(value, options, what):
    v = str(value if value is not None else options[0]).lower()
    if v not in options:
        raise ScenarioError(f"{what}: {value!r} is not one of {', '.join(options)}")
    return options.index(v)


def pack_slot(mon, consts, where):
    if mon is None:
        return struct.pack("<HBB4HHBBBBBB", 0, 50, 0, 0, 0, 0, 0, 0, 0, 0, 31, 100, 0, 0)
    species = consts[constant("SPECIES", mon["species"])]
    level = int(mon.get("level", 50))
    if not 1 <= level <= 100:
        raise ScenarioError(f"{where}: level {level} is not 1-100")
    moves = [consts[constant("MOVE", m)] if m else 0 for m in mon.get("moves", [])]
    if len(moves) > 4:
        raise ScenarioError(f"{where}: more than 4 moves")
    moves += [0] * (4 - len(moves))            # all zero = the Lab fills level-up moves
    item = consts[constant("ITEM", mon["item"])] if mon.get("item") else 0
    ability = mon.get("ability", 1)
    if ability not in ABILITY_SLOTS:
        raise ScenarioError(f"{where}: ability must be 1, 2 or \"hidden\"")
    nature = consts[constant("NATURE", mon["nature"])] if mon.get("nature") else 0
    ivs = int(mon.get("ivs", 31))
    hp = int(mon.get("hp", 100))
    if not 0 <= ivs <= 31 or not 1 <= hp <= 100:
        raise ScenarioError(f"{where}: ivs must be 0-31 and hp 1-100")
    status = pick(mon.get("status", "none"), STATUSES, f"{where} status")
    flags = SLOT_ACTIVE if mon.get("battle", True) else 0
    return struct.pack("<HBB4HHBBBBBB", species, level, flags, *moves, item,
                       ABILITY_SLOTS[ability], nature, ivs, hp, status, 0)


def pack_record(sc, consts, charmap):
    where = sc["_path"]
    rules = sc.get("rules", {})
    seed = rules.get("seed")
    if seed is not None and not 0 <= int(seed) <= 65535:
        raise ScenarioError(f"{where}: seed must be 0-65535 or null")
    rec = encode_text(sc["id"], 8, charmap, f"{where} id")
    rec += encode_text(sc.get("name", ""), 24, charmap, f"{where} name")
    rec += struct.pack("<BBBBBBH",
                       pick(rules.get("format"), FORMATS, f"{where} format"),
                       pick(rules.get("weather"), WEATHERS, f"{where} weather"),
                       pick(rules.get("terrain"), TERRAINS, f"{where} terrain"),
                       pick(rules.get("ai"), AIS, f"{where} ai"),
                       1 if seed is not None else 0,
                       pack_combat(rules, where), int(seed or 0))
    for side in ("player", "opponent"):
        mons = sc[side] + [None] * (PARTY_SIZE - len(sc[side]))
        for i, mon in enumerate(mons):
            rec += pack_slot(mon, consts, f"{where} {side} {i + 1}")
    assert len(rec) == RECORD_SIZE, len(rec)
    return rec


def build_block(scenarios, open_index, gcc):
    if len(scenarios) > MAX_SCENARIOS:
        raise ScenarioError(f"at most {MAX_SCENARIOS} scenarios fit (got {len(scenarios)})")
    ids = [sc["id"] for sc in scenarios]
    if len(set(ids)) != len(ids):
        raise ScenarioError("duplicate scenario ids: " + ", ".join(sorted(i for i in ids if ids.count(i) > 1)))
    consts = resolve_constants(needed_constants(scenarios), gcc)
    charmap = load_charmap()
    data = bytes([VERSION, len(scenarios), open_index, 0])
    for sc in scenarios:
        data += pack_record(sc, consts, charmap)
    return data


def inject(rom_in, rom_out, block_data):
    with open(rom_in, "rb") as f:
        rom = bytearray(f.read())
    at = rom.find(MARKER)
    if at < 0 or rom.find(MARKER, at + 1) >= 0:
        raise ScenarioError(f"{rom_in}: Battle Lab block not found exactly once (is this a dev build?)")
    start = at + len(MARKER)
    if len(block_data) > BLOCK_SIZE - len(MARKER):
        raise ScenarioError("scenarios don't fit in the Lab block")
    rom[start:start + len(block_data)] = block_data
    rom[start + len(block_data):at + BLOCK_SIZE] = b"\xFF" * (BLOCK_SIZE - len(MARKER) - len(block_data))
    tmp = rom_out + ".tmp"
    with open(tmp, "wb") as f:
        f.write(rom)
    os.replace(tmp, rom_out)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    c = sub.add_parser("check", help="validate scenario files")
    c.add_argument("files", nargs="+")
    i = sub.add_parser("inject", help="write scenarios into a Lab ROM copy")
    i.add_argument("--rom", required=True)
    i.add_argument("--out", required=True)
    g = i.add_mutually_exclusive_group()
    g.add_argument("--open", default="builtin", help="scenario id to open at boot, or 'builtin' (LAB-001)")
    g.add_argument("--no-open", action="store_true", help="boot to the title screen as usual")
    i.add_argument("files", nargs="*")
    args = ap.parse_args(argv)
    gcc = os.environ.get("ARM_GCC", "arm-none-eabi-gcc")
    try:
        scenarios = sorted((parse_scenario(p) for p in args.files), key=lambda s: s["id"])
        if args.cmd == "check":
            build_block(scenarios, 0, gcc)
            print(f"ok: {len(scenarios)} scenario(s)")
            return 0
        if args.no_open:
            open_index = 0
        elif args.open.lower() in ("builtin", "lab-001") and not any(s["id"] == args.open for s in scenarios):
            open_index = 1                                  # scenario 0 = built-in LAB-001
        else:
            ids = [s["id"] for s in scenarios]
            if args.open not in ids:
                raise ScenarioError(f"no scenario {args.open!r}; available: LAB-001 (built in), " + ", ".join(ids))
            open_index = 2 + ids.index(args.open)          # block scenario k = Lab scenario k+1
        inject(args.rom, args.out, build_block(scenarios, open_index, gcc))
        print(f"wrote {args.out} with {len(scenarios)} scenario(s)")
        return 0
    except ScenarioError as e:
        print(f"error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
