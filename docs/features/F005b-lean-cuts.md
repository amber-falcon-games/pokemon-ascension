# F005b — Lean cuts: unused link/e-Reader save data, duplicate Pokédex flags, 10 PC boxes
Status: DONE (pending owner smoke test in the 0.1.0 playtest)
Dependencies: F005 (scripts); decision D011
Owner decision IDs: D011
Branch / commit: `claude/sharp-euler-yfhti2`

## Objective and scope
Apply the parts of D011 that are safe as configuration changes, before the first release, so the 0.1.0 save layout already reflects them.
- Upstream switches in `include/config/save.h` set to `TRUE` (data removed from the save and code paths compiled out):
  - `FREE_UNION_ROOM_CHAT`, `FREE_LINK_BATTLE_RECORDS`, `FREE_POKEMON_JUMP`, `FREE_RECORD_MIXING_HALL_RECORDS`: link-cable / wireless features (cut).
  - `FREE_ENIGMA_BERRY`, `FREE_BATTLE_TOWER_E_READER`, `FREE_MYSTERY_EVENT_BUFFERS`: e-Reader and Mystery Event data (cut).
  - `FREE_EXTRA_SEEN_FLAGS_SAVEBLOCK1/2`: redundant Pokédex copies (no feature lost).
- `TOTAL_BOXES_COUNT` 14 → 10 (`include/pokemon_storage_system.h`).

## Explicit exclusions (moved to F005c, with reasons)
- Trainer Hill and Mystery Gift: their switches free the data but the building and the menu stay reachable in single-player. They need their entry points removed in maps/scripts at the same time.
- Match Call: kept until rematches have a replacement (VS Seeker needs an in-world source; F032).
- Secret Bases, TV, Contests/ribbons, Mail, flavour NPCs, link minigame venues, Recorded Battle sector: map/script edits (F005c).

## Acceptance criteria
- [x] Build passes: EWRAM 214,028 B (81.65%, was 86.37%), IWRAM 28,364 B, ROM 26,744,176 B (79.70%).
- [x] Full test suite: 5,426 passed / 5,876; the only 3 failures were upstream's save-layout tripwires (`test/save.c`), which fire by design on a layout change. Updated them to the new sizes as upstream's comment instructs; `make check TESTS=test/save.c`: 4/4 passed. Net: no new failures.
- [x] Save-block sizes measured before and after (DWARF): SaveBlock2 3,884 → 2,544 B; SaveBlock1 15,568 → 14,160 B; PokemonStorage 34,144 → 24,504 B; 12,388 B freed in total.
- [ ] Owner smoke test on Mac (part of the 0.1.0 playtest): new game, open PC (10 boxes, box 10 wraps to box 1), save, reload.

## Technical approach and risks
Config-only changes supported by upstream. Risk: a hidden reference to freed data in a reachable path. Mitigation: full test suite plus owner smoke test; link/e-Reader paths are unreachable without link hardware.

## Manual playtest
After `git pull`: `./scripts/play.sh`, start a new game (or use quickstart: press SELECT on the title screen), reach a Pokémon Center PC, open "Someone's PC", move a Pokémon, cycle boxes with L/R past box 10. Save in-game, close mGBA, reopen, Continue.

## Resource budgets and save compatibility
Changes the save layout. No released saves exist (pre-0.1.0), so no migration. Any save made on an earlier dev build is incompatible: start a new game.

## Rollback
Revert the commit.

## Evidence and acceptance
Linux container, 2026-11-26: `make -j4` exit 0 (258 s); `make -j4 check` 5,426 passed + 3 expected tripwire failures; after updating `test/save.c`, `make check TESTS=test/save.c` 4/4 passed.
