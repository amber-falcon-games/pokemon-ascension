# Save compatibility

No engine or save schema yet. F001/F006 establish upstream layout, capacity and baseline. Before any persistent change record old/new version, affected data, compatibility, migration/rollback and tests. Never silently overwrite incompatible saves. Preserve backups and distinguish emulator save states from game saves. Use isolated debug/Lab saves; Lab battles must not mutate campaign. Version scenario schemas separately. If compatibility cannot be retained, request approval and clearly document restart requirements before merging. No save binaries committed.


## Measured upstream headroom (expansion/1.17.2, F001 audit)
SaveBlock2 84 B free of 3,968; SaveBlock1 304 B free of 15,872; PokemonStorage 1,568 B free of 35,712; SaveBlock3 ≈1,620 B free of 1,624 (default config). All 32 flash sectors are allocated (two 14-sector slots, Hall of Fame, Trainer Hill, Recorded Battle). Consequence: persistent per-Pokémon additions (e.g. extra stored moves) are effectively impossible for boxed Pokémon; prefer data-derived state. Lab scenario storage must reuse a debug-only sector or live outside the save; to be designed in F011.

## Options for more save space (reference for F015; none decided)
The 128 KB flash chip is the largest standard GBA save type and is already fully allocated, so space can only be reclaimed, not added. Work RAM (EWRAM 256 KB, IWRAM 32 KB) is fixed hardware; ROM (32 MB, ≈6.8 MB free) is cheap, so anything derivable from game data should live in ROM, not the save.
- S1 — Upstream `include/config/save.h` toggles: remove unused link/e-Reader/Mystery Gift/record-mixing data. Up to 2,516 B in SaveBlock1 and 1,274 B in SaveBlock2. Low risk, mostly features this project does not need.
- S2 — Repurpose the Trainer Hill and/or Recorded Battle flash sectors (3,968 B each) if those features are dropped; also a candidate for Battle Lab scenarios (F011).
- S3 — Fewer PC boxes: each box of 30 Pokémon costs 2,400 B (80 B per stored Pokémon; 14 boxes now). Gameplay decision.
- S4 — Spare bits already inside each stored Pokémon: roughly 10–11 unused bits (e.g. `unused_04:5`, `unused_06:3`, `unused_13:1`, `unused_1E:1` in `include/pokemon.h`). Too few for a library, enough for small flags.
- S5 — Drop the second (backup) save slot: frees ≈55 KB but removes protection against corrupted saves. Not recommended.

Move storage today is already index-based: each of the four moves is an 11-bit index into the global move table (`include/pokemon.h`, `struct PokemonSubstruct1`). Storing more global indices costs 11 bits per move per Pokémon (a 20-move library ≈ 27 B × 420 boxed Pokémon ≈ 11.5 KB). Cheaper encodings: a bitmask over the species' own learnable-move list (1 bit per learnable move, roughly 8–16 B per Pokémon), or deriving level-up moves from level with zero storage and storing only TM/tutor extras. These choices belong to F015.

## Measured footprint of optional features (expansion/1.17.2, from DWARF debug info, 2026-11-26)
Save blocks are mirrored in EWRAM, so every byte saved also frees work RAM. A 14-sector save slot holds 55,552 B in total; block boundaries can be rebalanced across sectors, so freed bytes in any block form a common pool.
| Feature | Save bytes | Where |
|---|---|---|
| Secret Bases (+ decoration inventory ≈170 B) | 3,200 | SaveBlock1 |
| Battle Frontier (all facilities, records) | 2,272 | SaveBlock2 |
| Mystery Gift / Mystery Event script / e-Reader / external events | 876 + 1,004 + 52 + 57 | SaveBlock1 |
| Record mixing: Hall records, trainer name records, gift | 1,032 + 240 + 16 | SaveBlock2/1 |
| TV shows, PokéNews, Gabby & Ty | 900 + 64 + 12 | SaveBlock1 |
| Contests: winners, Pokéblocks, link results | 416 + 320 + 40 | SaveBlock1/2 |
| Mail | 576 | SaveBlock1 |
| Fusion storage (DNA Splicers etc.) | 400 | PokemonStorage |
| Frontier Apprentice | 316 | SaveBlock2 |
| Daycare | 288 | SaveBlock1 |
| Union Room chat phrases | 212 | SaveBlock1 |
| Match Call / trainer rematches | 102 | SaveBlock1 |
| Link battle records | 88 | SaveBlock1 |
| Mauville Old Man, Lilycove Lady, Dewford Trends, Walda phrase | 64 + 64 + 48 + 24 | SaveBlock1 |
| Roaming legendary | 28 | SaveBlock1 |
| Link minigames (Berry Crush, Pokémon Jump, Berry Picking) | 48 | SaveBlock2 |
| Unused Pokédex seen flags (no feature lost) | 52 + 108 | SaveBlock1/2 |
| Hall of Fame | 7,936 (2 sectors) | Flash sectors 28–29 |
| Trainer Hill | 3,968 (1 sector) + 28 | Flash sector 30 |
| Recorded Battle | 3,968 (1 sector) | Flash sector 31 |
| One PC box (30 Pokémon) | 2,400 | PokemonStorage |

Per-Pokémon bits (multiply by 426 stored Pokémon; reusing them needs no change to save size):
- 22 bits currently unused (`unused_02:6`, `unused_04:3`, `unused_0A:2`, `unused_04:5`, `unused_06:3`, `unused_0B:1`, `unused_13:1`, `unused_1E:1`; no code references outside `include/pokemon.h`).
- Contest condition stats (cool, beauty, cute, smart, tough, sheen): 48 bits, freed if contests are cut.
- Contest rank ribbons 15 bits; other ribbons 12 bits (several never obtainable); `isShadow` 1 bit.
