# Save compatibility

No engine or save schema yet. F001/F006 establish upstream layout, capacity and baseline. Before any persistent change record old/new version, affected data, compatibility, migration/rollback and tests. Never silently overwrite incompatible saves. Preserve backups and distinguish emulator save states from game saves. Use isolated debug/Lab saves; Lab battles must not mutate campaign. Version scenario schemas separately. If compatibility cannot be retained, request approval and clearly document restart requirements before merging. No save binaries committed.


## Measured upstream headroom (expansion/1.17.2, F001 audit)
SaveBlock2 84 B free of 3,968; SaveBlock1 304 B free of 15,872; PokemonStorage 1,568 B free of 35,712; SaveBlock3 ≈1,620 B free of 1,624 (default config). All 32 flash sectors are allocated (two 14-sector slots, Hall of Fame, Trainer Hill, Recorded Battle). Consequence: persistent per-Pokémon additions (e.g. extra stored moves) are effectively impossible for boxed Pokémon; prefer data-derived state. Lab scenario storage must reuse a debug-only sector or live outside the save; to be designed in F011.
