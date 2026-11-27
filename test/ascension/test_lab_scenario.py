"""Tests for tools/ascension/lab_scenario.py (F011). Run: python3 test/ascension/test_lab_scenario.py

Name resolution normally compiles a probe with arm-none-eabi-gcc; these tests stub it so they run
anywhere (CI docs job). The real resolution is exercised by ./scripts/play.sh --lab.
"""
import json
import os
import struct
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "tools", "ascension"))
import lab_scenario as ls  # noqa: E402

FAKE = {"SPECIES_SWAMPERT": 260, "SPECIES_BLAZIKEN": 257, "MOVE_WATERFALL": 127, "MOVE_ICE_BEAM": 58,
        "ITEM_LEFTOVERS": 472, "NATURE_ADAMANT": 3}


def fake_resolve(names, gcc):
    missing = [n for n in names if n not in FAKE]
    if missing:
        raise ls.ScenarioError("unknown name(s): " + ", ".join(missing))
    return {n: FAKE[n] for n in names}


ls.resolve_constants = fake_resolve


def scenario(**extra):
    sc = {"id": "LAB-099", "name": "Test", "_path": "test.json",
          "player": [{"species": "SWAMPERT", "level": 40, "moves": ["WATERFALL", "ICE_BEAM"],
                      "item": "LEFTOVERS", "nature": "ADAMANT", "ivs": 20, "hp": 50, "status": "burn",
                      "ability": "hidden"}],
          "opponent": [{"species": "BLAZIKEN"}]}
    sc.update(extra)
    return sc


class LabScenarioTool(unittest.TestCase):
    def test_record_layout(self):
        block = ls.build_block([scenario(rules={"format": "trainer_double", "weather": "rain", "seed": 1234})], 2, "gcc")
        self.assertEqual(block[:4], bytes([1, 1, 2, 0]))            # version, count, open, reserved
        rec = block[4:]
        self.assertEqual(len(rec), ls.RECORD_SIZE)
        charmap = ls.load_charmap()
        self.assertEqual(rec[0], charmap["L"])
        self.assertIn(0xFF, rec[:8])                                 # EOS-terminated id
        fmt, weather, terrain, ai, fixed, _, seed = struct.unpack("<BBBBBBH", rec[32:40])
        self.assertEqual((fmt, weather, terrain, ai, fixed, seed), (1, 1, 0, 0, 1, 1234))
        slot = struct.unpack("<HBB4HHBBBBBB", rec[40:60])
        self.assertEqual(slot[:3], (260, 40, 1))                     # species, level, active
        self.assertEqual(slot[3:7], (127, 58, 0, 0))                 # moves
        self.assertEqual(slot[7:13], (472, 2, 3, 20, 50, 3))         # item, hidden ability, nature, ivs, hp, burn
        opp = struct.unpack("<HBB4HHBBBBBB", rec[40 + 6 * 20:60 + 6 * 20])
        self.assertEqual(opp[:3], (257, 50, 1))                      # defaults: level 50, active
        self.assertEqual(opp[3:7], (0, 0, 0, 0))                     # no moves: level-up moves in game

    def test_errors(self):
        with self.assertRaises(ls.ScenarioError):
            ls.build_block([scenario(player=[{"species": "NOT_REAL"}])], 0, "gcc")
        with self.assertRaises(ls.ScenarioError):
            ls.build_block([scenario(player=[{"species": "SWAMPERT", "level": 101}])], 0, "gcc")
        with self.assertRaises(ls.ScenarioError):
            ls.build_block([scenario(rules={"weather": "fog"})], 0, "gcc")
        with self.assertRaises(ls.ScenarioError):
            ls.build_block([scenario(id="TOOLONGID")], 0, "gcc")
        with self.assertRaises(ls.ScenarioError):
            ls.build_block([scenario(), scenario()], 0, "gcc")       # duplicate ids
        with self.assertRaises(ls.ScenarioError):
            ls.build_block([scenario(id=f"L{i}") for i in range(ls.MAX_SCENARIOS + 1)], 0, "gcc")

    def test_inject_into_marked_rom(self):
        with tempfile.TemporaryDirectory() as tmp:
            rom_in, rom_out = os.path.join(tmp, "in.gba"), os.path.join(tmp, "out.gba")
            body = b"\x00" * 1000 + ls.MARKER + b"\xFF" * (ls.BLOCK_SIZE - len(ls.MARKER)) + b"\x11" * 500
            with open(rom_in, "wb") as f:
                f.write(body)
            data = ls.build_block([scenario()], 2, "gcc")
            ls.inject(rom_in, rom_out, data)
            out = open(rom_out, "rb").read()
            self.assertEqual(len(out), len(body))
            at = out.find(ls.MARKER)
            self.assertEqual(out[at + 16:at + 16 + len(data)], data)
            self.assertEqual(out[at + 16 + len(data)], 0xFF)
            self.assertEqual(out[-500:], b"\x11" * 500)              # rest of the ROM untouched
            self.assertEqual(open(rom_in, "rb").read(), body)         # input never modified
            with open(rom_in, "wb") as f:
                f.write(b"\x00" * 100)
            with self.assertRaises(ls.ScenarioError):
                ls.inject(rom_in, rom_out, data)                      # release ROM: no block

    def test_repository_presets_parse(self):
        folder = os.path.join(HERE, "..", "..", "lab", "scenarios")
        files = sorted(f for f in os.listdir(folder) if f.endswith(".json"))
        self.assertTrue(files)
        for name in files:
            sc = ls.parse_scenario(os.path.join(folder, name))
            self.assertEqual(sc["id"] + ".json", name)                # file name = id
            json.dumps(sc)

    def test_combat_byte_packs_stamina_tuning(self):
        # bits 0-1 combat, 2-3 energy, 4-5 damage, 6-7 cost (struct LabRules bitfields)
        self.assertEqual(ls.pack_combat({}, "t"), 0)
        self.assertEqual(ls.pack_combat({"combat": "stamina"}, "t"), 1)
        self.assertEqual(ls.pack_combat({"combat": "dual"}, "t"), 2)
        self.assertEqual(ls.pack_combat({"combat": "stamina", "energy": 2, "damage": 75, "cost": 50}, "t"),
                         1 | 2 << 2 | 1 << 4 | 2 << 6)
        self.assertEqual(ls.pack_combat({"energy": 3, "damage": 33, "cost": 25}, "t"), 3 << 2 | 3 << 4 | 3 << 6)
        with self.assertRaises(ls.ScenarioError):
            ls.pack_combat({"energy": 4}, "t")
        with self.assertRaises(ls.ScenarioError):
            ls.pack_combat({"damage": 60}, "t")


if __name__ == "__main__":
    unittest.main()
