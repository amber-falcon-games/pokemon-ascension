#!/usr/bin/env python3
"""Pokémon Ascension (Dual bars HUD): makes the player's singles healthbox 8 pixels taller so the
energy bar sits inside it, under the HP numbers. Plain body rows are inserted at y=29, just above
the bottom-corner rounding, so every row from there moves down exactly 8 px: the EXP strip moves
from tile row 4 to tile row 5 (the game's EXP fill offsets add 8 tiles).
The opponent uses upstream's healthbox_singles_opponent_large.png, which is already 8 px taller.

  python3 tools/ascension/tall_healthbox.py   (writes graphics/battle_interface/healthbox_singles_player_tall.png)
"""
import os
from PIL import Image

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
SRC = os.path.join(ROOT, "graphics/battle_interface/healthbox_singles_player.png")
OUT = os.path.join(ROOT, "graphics/battle_interface/healthbox_singles_player_tall.png")
INSERT_AT, ROWS, BODY_ROW = 29, 8, 28   # BODY_ROW: a plain body row


def main():
    src = Image.open(SRC)
    assert src.mode == "P" and src.size == (128, 64)
    w, h = src.size
    out = Image.new("P", (w, h), 0)
    out.putpalette(src.getpalette())
    sp, op = src.load(), out.load()
    for y in range(h):
        if y < INSERT_AT:
            sy = y
        elif y < INSERT_AT + ROWS:
            sy = None
        else:
            sy = y - ROWS
        for x in range(w):
            if sy is None:
                op[x, y] = sp[x, BODY_ROW]
            elif sy < h:
                op[x, y] = sp[x, sy]
    # nothing may be lost off the bottom
    assert all(sp[x, y] == 0 for y in range(h - ROWS, h) for x in range(w))
    out.save(OUT)
    print("wrote", OUT)


if __name__ == "__main__":
    main()
