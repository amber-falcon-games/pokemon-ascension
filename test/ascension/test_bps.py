"""Tests for tools/ascension/bps.py. Run: python3 test/ascension/test_bps.py"""
import os
import random
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "tools", "ascension"))
import bps  # noqa: E402


class BpsRoundTrip(unittest.TestCase):
    def roundtrip(self, source, target):
        patch = bps.create_patch(source, target, b"test")
        self.assertEqual(bps.apply_patch(source, patch), target)
        return patch

    def test_identical(self):
        data = bytes(random.Random(1).randrange(256) for _ in range(5000))
        patch = self.roundtrip(data, data)
        self.assertLess(len(patch), 64)

    def test_edits_moves_runs_and_growth(self):
        rng = random.Random(2)
        src = bytes(rng.randrange(256) for _ in range(200_000))
        tgt = bytearray(src)
        tgt[1000:1010] = b"0123456789"                 # in-place edit
        tgt[50_000:50_000] = src[120_000:130_000]       # moved block (SourceCopy)
        tgt += b"\xff" * 100_000                         # padding run (TargetCopy)
        tgt += bytes(rng.randrange(256) for _ in range(3000))  # new data (TargetRead)
        patch = self.roundtrip(src, bytes(tgt))
        self.assertLess(len(patch), 10_000)

    def test_shrink_and_empty(self):
        src = bytes(range(256)) * 100
        self.roundtrip(src, src[:1000])
        self.roundtrip(src, b"")
        self.roundtrip(b"", b"new")

    def test_rejects_wrong_source(self):
        src = bytes(range(256)) * 10
        patch = bps.create_patch(src, src[::-1])
        with self.assertRaises(bps.PatchError):
            bps.apply_patch(src[:-1] + b"x", patch)

    def test_rejects_corrupt_patch(self):
        src = bytes(range(256)) * 10
        patch = bytearray(bps.create_patch(src, src[::-1]))
        patch[10] ^= 0xFF
        with self.assertRaises(bps.PatchError):
            bps.apply_patch(src, bytes(patch))

    def test_varint_roundtrip(self):
        for v in [0, 1, 127, 128, 129, 16383, 16384, 2**32, 2**40 + 7]:
            out = bytearray()
            bps.put_varint(out, v)
            self.assertEqual(bps.get_varint(out, 0), (v, len(out)))


if __name__ == "__main__":
    unittest.main()
