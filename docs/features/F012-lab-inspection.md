# F012 — Battle Lab: inspection, determinism and diagnostics
Status: AWAITING_ACCEPTANCE (implemented on a branch stacked on F011, D021)
Dependencies: F011
Owner decision IDs: D003, D021
Branch / commit: `feature/F012-lab-inspection` (based on `feature/F011-lab-scenarios`)

## Objective and scope
BATTLE_LAB.md asks F012 for: inspecting battle state, debug controls, a deterministic seed, concise diagnostics, a safe reset, and host-side logs where feasible.

**Already covered:**
- **Inspection** comes from upstream's in-battle debug menu: press SELECT during a dev-build battle to see stats, stat stages, status, volatiles, hazards, side status, the AI's move scores, Instant Win, and edits to most of these.
- **Safe reset** is F010's Lab: Restart and Lab: Quit.
- **Fixed seed** is F009.

**New in F012:**
- **Every battle is replayable.**
  - With Seed: Random, the Lab picks the battle's seed itself and remembers it.
  - Rules shows `Last seed: N`.
  - **A on the Seed no. row copies the last seed and switches to Fixed**, so the battle you just saw, oddity included, replays exactly with the same inputs.
- **Battle log in mGBA's log window** (mGBA: *Tools → View Logs*, Warn level), dev builds only. At battle start it writes the scenario, format, weather, terrain, AI and seed. At the end of every turn it writes each battler's species, level, HP, max HP and status. A one-line hook in the turn-end code calls the Lab only while a Lab battle runs.
- **Toggles for experimental mechanics:** none exist yet. Each future combat feature (F014 onward) adds its own on/off row to the Lab Rules screen, together with its scenario (BATTLE_LAB.md contract), so there is no empty framework now.

## Explicit exclusions
- Writing logs to a file on the host. mGBA's log window can save its contents, and on-cartridge logging is out of scope.
- A custom stat inspector, since upstream's covers it.

## Acceptance criteria
- [ ] With Seed: Random, after a battle Rules shows `Last seed: N`. A on Seed no. sets Fixed N, and the next battle with the same inputs replays the first turn identically.
- [ ] mGBA's log window shows the start line and one block per turn for a Lab battle, and nothing for campaign battles.
- [ ] Release build: no Lab symbols, and the hook compiles to nothing.

## Technical approach and risks
- **Seed:** in Start battle, the seed is either the fixed seed or a fresh `Random()` value. The Lab stores it and calls `SeedRng(seed)` before building the parties, so the whole run is reproducible.
- **Log:**
  - `BattleLab_LogTurnEnd()` uses `DebugPrintf`, with `%S` for game-charset names.
  - The hook sits in `src/battle_main.c` where `battleTurnCounter` advances, and is guarded by `BattleLab_IsLabBattle()`.
  - In release builds it's a `static inline` no-op, and `DebugPrintf` is compiled out (NDEBUG).
- **Risk: log volume.** Mitigation: a few lines per turn only.

## Automated tests and regression
`test/ascension/battle_lab.c`:
- A random-seed run records a seed.
- "Use last seed" sets Fixed with that value.
- Logging outside a Lab battle does nothing (no crash).

Lab tests from F008–F011 stay green, and the full suite runs in CI.

## Manual playtest
`./scripts/play.sh --lab`, then open mGBA *Tools → View Logs*:
1. Start battle. The log shows `LAB start …` and a block for each turn.
2. After the battle, go to Rules. `Last seed` shows a number. Press A on Seed no., and Seed becomes Fixed with that number.
3. Start battle again and use the same move. The damage and critical hits match the earlier battle.

## Resource budgets and save compatibility
A few hundred bytes of ROM and 2 B of EWRAM, dev builds only. No save change.

## Rollback
Revert the PR.

## Evidence and acceptance
Implemented 2026-11-26 (Linux container):
- `make -j8`: exit 0, no warnings. `make check TESTS=test/ascension/battle_lab.c`: **21/21 PASS**.
- Battle log, captured headlessly with `GBASHOT_GAME_LOG=1` (gbashot now prints the game's debug output, the same text mGBA's log window shows):
  ```
  LAB start LAB-001: trainer 1v1, weather none, terrain none, AI scenario, seed 21534
  LAB turn 1
    you0 Swampert Lv50 HP 175/175 status 0x0
    foe1 Blaziken Lv50 HP 63/155 status 0x0
  ```
- **Found by the log:** a first run showed Swampert at `status 0x4` and Blaziken undamaged. The screen said "Swampert began to nap!": upstream **disobedience**, because the Lab has no badges and LAB-001's Pokémon count as outsiders. This was a latent Lab bug since F007. It's fixed on F009 (Lab battles always obey: one guarded line in `GetAttackerObedienceForAction`) and merged forward. The run above is after the fix.
- Release: `nm pokeemerald-release.elf | grep -ci "BattleLab\|Ascension_C"` = 0. The turn-end hook is a `static inline` no-op, and `DebugPrintf` is compiled out (NDEBUG).
- Not checked headlessly: fixed-seed replay of a full battle. The seed path is unit-tested, and the replay is part of the owner playtest.
