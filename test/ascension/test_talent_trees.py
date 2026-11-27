"""Tests for tools/ascension/talent_trees.py (F015, D027)."""
import json
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "tools", "ascension"))
import talent_trees as tt  # noqa: E402


class TalentTrees(unittest.TestCase):
    def test_repository_trees_pass_the_checker(self):
        self.assertTrue(tt.check(tt.JSON_OUT))

    def test_checker_catches_bad_edits(self):
        data = json.load(open(tt.JSON_OUT))
        tree = data["species"]["SPECIES_PIKACHU"]["branches"]
        tree[0]["nodes"][1]["level"] = 0                       # level out of range / order
        tree[1]["nodes"][0]["moves"] = ["MOVE_NOT_A_MOVE"]      # unknown move
        data["species"]["SPECIES_PIKACHU"]["branches"] = tree * 3   # too many branches
        with tempfile.NamedTemporaryFile("w", suffix=".json", delete=False) as f:
            json.dump(data, f)
        try:
            self.assertFalse(tt.check(f.name))
        finally:
            os.unlink(f.name)

    def test_generated_trees_respect_limits(self):
        data = json.load(open(tt.JSON_OUT))
        for sp, tree in data["species"].items():
            self.assertLessEqual(len(tree["branches"]), tt.MAX_BRANCHES, sp)
            for b in tree["branches"]:
                levels = [n["level"] for n in b["nodes"]]
                self.assertEqual(levels, sorted(levels), sp)

    def test_evolved_relearn_moves_unlock_at_evolution(self):
        data = json.load(open(tt.JSON_OUT))
        dragon = [b for b in data["species"]["SPECIES_CHARIZARD"]["branches"] if b["name"] == "Dragon Physical"][0]
        self.assertEqual(dragon["nodes"][0]["level"], 36)   # Dragon Claw comes with evolving at 36




class CompiledHeader(unittest.TestCase):
    def test_committed_header_matches_the_json(self):
        with tempfile.TemporaryDirectory() as d:
            out = os.path.join(d, "talent_trees.h")
            tt.compile_header(tt.JSON_OUT, out)
            self.assertEqual(open(out).read(), open(tt.H_OUT).read(),
                             "run: python3 tools/ascension/talent_trees.py compile")


if __name__ == "__main__":
    unittest.main()
