# F011 — Battle Lab: saved scenarios and preset registry
Status: SPEC — OWNER DECISION NEEDED (storage option, below)
Dependencies: F010
Owner decision IDs: D003; D011 (Recorded Battle cut); new decision requested: Lab scenario storage
Branch / commit: `feature/F011-lab-scenarios` (stacked on `feature/F010-lab-battle-loop`, D021)

## Objective and scope
Reusable Battle Lab setups, of two kinds:
- **Presets** live in the repository and ship in every dev build: LAB-001, LAB-009 (six-member trainer battle), LAB-010 (over-levelled wild encounter), and later one per combat feature, as BATTLE_LAB.md requires.
- **Saved scenarios** are made in the Lab and kept on the cartridge, independent of the campaign save.

Lab menu → **Scenario** opens a list of presets (`P` LAB-001 …) and saved slots (`1`–`N`, name or `empty`). In that list:
- A loads the highlighted entry into the teams and rules.
- SELECT on a saved slot offers Save here / Rename / Delete.
- The title shows the loaded scenario's ID or name, plus `(edited)` once anything is changed.

`./scripts/play.sh --scenario LAB-009` opens the Lab with that preset loaded (method below).

## Storage options (owner decision)
| | Option | Capacity | Notes |
|---|---|---|---|
| **A (recommended)** | Use the **Recorded Battle flash sector** (sector 31) in dev builds | 3,968 B, which fits **12 saved scenarios** | Recorded Battle is already cut (D011), so the sector is free. Upstream's `TryWriteSpecialSaveSector` reads and writes it, separately from the campaign save. Dev builds stop recording battles now; release builds are unchanged until F005c removes Recorded Battle everywhere. |
| B | No cartridge storage: presets only, plus an on-screen export code to copy by hand | Unlimited presets, 0 saved | No save risk, but sharing or keeping a setup means writing down a long code. |
| C | Space inside the campaign save | — | Rejected: about 300 B free in SaveBlock1, and a Lab scenario shouldn't live in the campaign. |

**Scenario record (option A):** about 300 B each.
- Header: magic `LAB1`, format version, checksum.
- Name: 10 characters.
- Both sides: 6 slots × 20 B.
- Rules: 8 B.

The sector carries a version byte. On a format change, older versions are migrated where possible and otherwise shown as `old format`, never silently misread. The format and migrations are documented in SAVE_COMPATIBILITY.md.

**Presets:** written as data (`src/data/battle_lab_scenarios.h`: teams, details and rules per ID), so a new combat feature adds its LAB-xxx scenario in the same PR. The registry `docs/BATTLE_LAB_SCENARIOS.md` lists every preset.

**`--scenario ID` method:** `play.sh` writes a small "open scenario ID" request into the Lab sector of the dev save file before starting mGBA. On the title screen, the Lab opens with that preset, and the request is cleared once used. This needs option A. Under option B, `--scenario` would only print which preset to pick in the menu.

## Explicit exclusions
- Sharing scenarios between machines other than by copying the `.sav` file.
- Importing from other tools.

## Acceptance criteria
- [ ] Presets LAB-001, LAB-009 and LAB-010 load from the Scenario menu and play as their registry entries describe.
- [ ] (A) A saved scenario survives Exit, closing mGBA and rebuilding the ROM. 12 slots work: Save, Rename, Delete. The campaign save is byte-identical before and after (checked by `.sav` diff, sectors 0–29).
- [ ] (A) `./scripts/play.sh --scenario LAB-009` opens the Lab with LAB-009 loaded.
- [ ] Format version: a record from a different format version is reported, not loaded.
- [ ] Release build: no Lab symbols, and Recorded Battle behaviour is unchanged in release.

## Technical approach and risks
- `src/battle_lab_scenarios.c` holds the record encode/decode, checksum and the preset table.
- The Lab Scenario screen reuses the F008 list UI.
- Upstream touch (option A): `ASCENSION_BATTLE_LAB` builds compile out the Recorded Battle *save* call (`recorded_battle.c`), so the sector isn't overwritten. This is dev-only; release keeps upstream behaviour until F005c.
- **Risk: corrupting the campaign save.** Mitigation: only upstream's special-sector API is used, which never touches sectors 0–29, and the acceptance `.sav` diff checks it.

## Automated tests and regression
- Encode/decode round trip for every field.
- Checksum and version rejection.
- Presets build the documented parties.
- Lab tests from F008–F010 stay green, and the full suite runs in CI.

## Manual playtest
After implementation, with steps in the PR: load each preset, save two scenarios, exit and restart mGBA, load them, then rename and delete one. Finally, `play.sh --scenario LAB-009`.

## Resource budgets and save compatibility
- Option A uses sector 31 in dev builds only: no campaign layout change, and release unchanged.
- ROM: a few KB for code and presets.
- EWRAM: a 3,968 B buffer, only while the Scenario screen is open (allocated, then freed).

## Rollback
Revert the PR. The sector simply holds stale Lab data, which the restored Recorded Battle code treats as an invalid recording, as upstream does for an empty sector.

## Evidence and acceptance
Pending owner decision (A or B), then implementation.
