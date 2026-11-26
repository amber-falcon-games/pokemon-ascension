# F011 — Battle Lab: scenario files and presets
Status: AWAITING_ACCEPTANCE (implemented on a branch stacked on F010, D021; owner playtest after F010)
Dependencies: F010
Owner decision IDs: D003, D021, **D022** (scenarios stay out of save memory: repository files injected into a Lab ROM copy)
Branch / commit: `feature/F011-lab-scenarios` (based on `feature/F010-lab-battle-loop`)

## Objective and scope
Reusable Battle Lab setups that never touch save memory (D022, the owner's design):
- **Scenario files** live in the repository: `lab/scenarios/<ID>.json`. Each holds the teams (species, level, moves, item, ability, nature, IVs, HP %, status, in battle), the rules (format, weather, terrain, AI, seed), an ID and a name. The format is in `lab/scenarios/README.md`.
- **`./scripts/play.sh --lab`** builds the dev ROM and copies it to `build/pokemon-ascension-lab.gba`. It writes every scenario file into a reserved data block in that copy and opens it in mGBA, which goes **straight into the Battle Lab**. **`--scenario LAB-009`** does the same but opens with that scenario.
  - Hold B while the Lab ROM boots to reach the title screen instead.
  - The Lab ROM has its own `.sav`, so the campaign save is never used.
- **Scenario menu** in the Lab lists the built-in LAB-001 plus every injected file. A loads one, and the title shows its ID. `!` and `(edited)` mean "different from the loaded scenario", and START on a team screen resets to it.
- **Presets shipped now:**
  - LAB-001: built in, baseline 1v1.
  - LAB-009: six-member trainer battle, 6v6, Smart AI.
  - LAB-010: over-levelled wild encounter, Marshtomp 25 and Swellow 24 vs a wild Tyranitar 70.

  Each new combat feature adds its LAB-xxx file in the same PR (BATTLE_LAB.md contract).

## Explicit exclusions
- Saving a setup made in the Lab back to a file. That would need an emulator script reading memory and is a later, optional step. Until then a setup is written as a file, which takes about a minute with the README table.
- Any cartridge or save storage (rejected in D022).

## Acceptance criteria
- [ ] `./scripts/play.sh --lab` opens the Lab directly. Scenario lists LAB-001, LAB-009 and LAB-010, and each one loads and plays as its file describes.
- [ ] `./scripts/play.sh --scenario LAB-010` opens with LAB-010 loaded (wild Tyranitar 70).
- [ ] A bad file (unknown species or move, level 101, unknown weather, an ID too long, a duplicate ID, or more than 14 files) stops `play.sh` with a message naming the file and the problem. No ROM is launched.
- [ ] The campaign ROM and save are untouched: `build/pokemon-ascension.gba` and `.sav` don't change after a Lab session.
- [ ] Release build: no Lab symbols and no reserved block.

## Technical approach and risks
- **Game:** `gBattleLabScenarioBlock`, 4,096 B of ROM in dev builds only. It starts with the 16-byte ASCII marker `ASCENSION-LAB-01` and is 0xFF everywhere else.
  - The header holds the version, the count and the scenario to auto-open.
  - Then come up to 14 fixed-size 280 B records, using the same layout as `struct LabSlot` and `struct LabRules`.
  - The Lab reads the block through a `volatile` pointer, because it is patched after the build. It never trusts the data: invalid species become empty slots, and invalid moves, items, abilities and ranges are reset to defaults. If a file lists no moves, the Pokémon gets its level-up moves.
- **Tool:** `tools/ascension/lab_scenario.py check|inject`.
  - Names such as `SWAMPERT` and `ICE_BEAM` are resolved by compiling a tiny probe against the project's own headers with the pinned ARM GCC, so the IDs always match the ROM being patched.
  - The marker must appear exactly once.
  - Writing is atomic (a temp file, then rename), and the input ROM is never modified.
- **Title screen:** a one-condition hook. The Lab ROM opens the Lab automatically when its block asks for it; in release builds the check is a `static inline FALSE`.
- **Risks:**
  - The record layout is shared by C and Python. Mitigations: a `STATIC_ASSERT` on the 280 B record size, and a Python test of the byte layout.
  - After an Exit, the Lab ROM reopens the Lab. That's by design; hold B at boot for the title screen.

## Automated tests and regression
- **`test/ascension/test_lab_scenario.py`** (CI docs job): byte layout of a full record, errors (unknown name, level, weather, long ID, duplicates, too many files), injection into a marked ROM with the rest left untouched, a missing block refused, and the repository presets parsing (file name = ID).
- **`test/ascension/battle_lab.c`** (now 20 tests): a plain dev ROM has only the built-in scenario and doesn't auto-open; selecting a scenario resets teams and rules; `!` and `(edited)` compare against the loaded scenario.
- Full suite in CI.

## Manual playtest
1. `./scripts/play.sh --lab`. mGBA opens straight into the Lab (title `BATTLE LAB · LAB-001`).
2. Open Scenario and pick LAB-009, then Start battle: a 6v6 against a Smart AI.
3. Back in the Lab, open Scenario and pick LAB-010, then Start battle: a wild Tyranitar Lv70.
4. Close mGBA and run `./scripts/play.sh --scenario LAB-010`. It opens on LAB-010.
5. Copy `lab/scenarios/LAB-010.json` to `LAB-011.json`, change `"id"` and a species, and run `./scripts/play.sh --scenario LAB-011`. Your new scenario appears.
6. Run `./scripts/play.sh` and choose Continue. Your campaign save is as before.

## Resource budgets and save compatibility
- Dev ROM vs F010: +6,126 B (the 4,096 B block plus about 2 KB of code); EWRAM +564 B (the loaded scenario and a scratch record). Release: 0.
- **No save memory at all.** The campaign save layout and data are unchanged.

## Rollback
Revert the PR. Scenario files stay in the repository, unused.

## Evidence and acceptance
Implemented 2026-11-26 (Linux container):
- `make -j8`: exit 0, no warnings.
- Tests:
  - `make check TESTS=test/ascension/battle_lab.c`: **20/20 PASS**.
  - `python3 -I test/ascension/test_lab_scenario.py`: **4/4 OK**.
- Real name resolution: `lab_scenario.py check lab/scenarios/*.json` gives `ok: 2 scenario(s)`. A bad file gives `error: unknown name(s): MOVE_NOT_A_MOVE, SPECIES_PIKACHUU`.
- `./scripts/play.sh --scenario LAB-009` built the ROM and wrote `build/pokemon-ascension-lab.gba` (header `01 02 02 00`: version 1, 2 scenarios, open LAB-009). mGBA isn't installed in the container, so the launch itself was not run there.
- Headless screenshots of the Lab ROM:
  - It boots straight into `BATTLE LAB · LAB-009` with You: 6 and Foe: 6.
  - Scenario lists LAB-001, LAB-009 (now) and LAB-010.
  - Loading LAB-010 shows Marshtomp 25 and Swellow 24 vs a wild Tyranitar 70, with no `!` on the unedited slots.
- Release: `nm pokeemerald-release.elf | grep -ci "BattleLab\\|Ascension_C"` = 0, and the `ASCENSION-LAB-01` marker appears 0 times in `pokeemerald-release.gba`.
