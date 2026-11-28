#!/usr/bin/env python3
"""Pokémon Ascension (Dual bars HUD): the player's singles healthbox for Dual-bars battles.

Layout (owner request): the HP numbers sit on the HP bar's row and the energy bar goes on the row
below, where the numbers used to be. To make room the box is 24 px wider (columns inserted between
the moved end cap and the right border) and the HP bar's end cap moves 16 px left with the bar. The game moves
the HP bar 16 px left and the box 16 px left (battle_interface.c). The height is unchanged.

  python3 tools/ascension/dual_healthbox.py   (writes graphics/battle_interface/healthbox_singles_player_wide.png)
"""
import os
from PIL import Image

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
SRC = os.path.join(ROOT, "graphics/battle_interface/healthbox_singles_player.png")
OUT = os.path.join(ROOT, "graphics/battle_interface/healthbox_singles_player_wide.png")
WIDEN_AT, WIDEN = 90, 24          # insert columns right of the moved end cap (corners move with the border)
STRIP_ROWS_FROM, STRIP_WIDEN_AT = 34, 97   # under the box: insert after the EXP strip, which keeps its length
BODY_COL = 60                     # a plain column of the box (top and bottom borders included)
CAP_COLS, CAP_ROWS, CAP_SHIFT = (96, 97), range(17, 23), 16   # the HP bar's ")" end cap


def main():
    src = Image.open(SRC)
    assert src.mode == "P" and src.size == (128, 64)
    w, h = src.size
    sp = src.load()
    rows = []
    for y in range(h):
        row = [sp[x, y] for x in range(w)]
        if y in CAP_ROWS:   # move the end cap left with the bar
            cap = [row[x] for x in CAP_COLS]
            for x in CAP_COLS:
                row[x] = row[BODY_COL]
            for i, x in enumerate(CAP_COLS):
                row[x - CAP_SHIFT] = cap[i]
        at = WIDEN_AT if y < STRIP_ROWS_FROM else STRIP_WIDEN_AT
        src_col = BODY_COL if y < STRIP_ROWS_FROM else STRIP_WIDEN_AT
        row = row[:at] + [row[src_col]] * WIDEN + row[at:]
        assert all(v == 0 for v in row[w:]), f"row {y} overflows"
        rows.append(row[:w])
    out = Image.new("P", (w, h), 0)
    out.putpalette(src.getpalette())
    op = out.load()
    for y in range(h):
        for x in range(w):
            op[x, y] = rows[y][x]
    out.save(OUT)
    print("wrote", OUT)


if __name__ == "__main__":
    main()
