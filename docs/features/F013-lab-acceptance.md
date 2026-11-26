# F013 — Battle Lab acceptance and 0.2.0 foundation release
Status: IN_PROGRESS (acceptance matrix complete; 0.2.0 release needs the owner's Mac)
Dependencies: F007–F012 (all merged or stacked), F007b
Owner decision IDs: D003, D017 (deliberate releases), D021, D022
Branch / commit: `feature/F013-lab-acceptance` (based on `feature/F012-lab-inspection`)

## Objective and scope
Close the Battle Lab foundation, the gate before any combat redesign (D003), and ship it as milestone **0.2.0**. BATTLE_LAB.md requires F013 to prove the following with automated checks and owner acceptance: direct boot, custom setup, play, reset, persistence, isolation and release exclusion. It also requires the memory and ROM overhead to be measured.

## Acceptance matrix
| Requirement (BATTLE_LAB.md) | Delivered by | Automated evidence | Manual evidence |
|---|---|---|---|
| Direct boot into the Lab, no campaign save needed | F007, F011 | CI builds the Lab ROM from `lab/scenarios/*.json` on every PR. Header and auto-open are covered by `test_lab_scenario.py` and `battle_lab.c` | Owner: `./scripts/play.sh --lab` opens straight into the Lab (2026-11-26) |
| Custom setup: both teams, details, rules | F008, F009 | `battle_lab.c`: party building, every Pokémon detail, rules (AI flags, weather and terrain via starting statuses, doubles validation) | Owner playtest of the stack (2026-11-26); headless 2v2 with rain and Electric Terrain |
| Play a real battle in the production engine | F007–F010 | The full upstream test suite in CI (4 shards) stays green | Owner played LAB-001, LAB-009 and LAB-010 |
| Restart, edit and relaunch, exit | F010 | Result mapping, and restart/quit refused outside Lab battles | Headless: Restart replays within about 150 frames; Quit returns with edits |
| Persistence (named, reusable scenarios) | F011 (D022: repository files, no save memory) | `test_lab_scenario.py`: byte layout, errors, injection, presets | Owner: 3 scenarios listed and playable |
| Determinism and diagnostics | F009, F012 | Fixed seed repeats the RNG; party building is seeded; no per-frame RNG in Lab battles | mGBA log window: start, per-turn and end blocks |
| Isolation (no campaign mutation) | F007 (fresh in-memory game, Exit soft-resets), F011 (separate Lab ROM and `.sav`) | Lab code never calls a save function. The Lab ROM is a copy and the input ROM is never modified (tested) | `./scripts/play.sh` → Continue loads the campaign save unchanged |
| Release exclusion | All | Nightly: no `BattleLab`, `sLabTrainers` or `Ascension_C` symbols and no `ASCENSION-LAB-01` block in the release ROM. PR CI checks the dev side | — |
| Overhead measured | All | See below | — |

## Overhead (dev builds only; release: 0)
| Feature | ROM | EWRAM |
|---|---|---|
| F007 Lab entry | +2,320 B | +8 B |
| F007b checkpoints | +2,628 B (scripts) | 0 |
| F008 team builder | +4,810 B | +56 B |
| F009 details and rules | +5,361 B | +200 B |
| F010 battle loop | +648 B | +4 B |
| F011 scenario files | +6,126 B (including the 4,096 B block) | +564 B |
| F012 diagnostics | about +1 KB | +4 B |
| **Total** | **about 23 KB of ROM** (of about 6.8 MB free) | **about 0.8 KB** (of 256 KB) |

No save memory is used by any of it (D022). In release builds all of the above is absent.

## 0.2.0 release (owner, on the Mac)
The player-facing build is functionally identical to 0.1.0. 0.2.0 marks the stable foundation that the combat work starts from, and its patch is the new last-known-good build.
1. `git checkout main && git pull`, once the stack and F013 are merged.
2. `./scripts/release.sh`: it suggests **0.2.0**, builds, and asks for a smoke test (New Game → first wild battle → save → Continue). Then it creates and checks the patch and publishes it. The release notes are the `0.2.0` section of `CHANGELOG.md`.

## Acceptance criteria
- [x] Every row of the matrix has automated evidence, owner evidence, or both.
- [x] Overhead measured.
- [ ] CI builds the Lab ROM on every PR, and the nightly checks the release ROM for the scenario block (added in this PR).
- [ ] 0.2.0 published by the owner with `release.sh`.

## Rollback
Docs and CI only, apart from the release. Revert the PR, and a published release stays as a tag.

## Evidence and acceptance
- Owner: "checked and working, 3 scenarios", 2026-11-26. This was the combined playtest of the Lab stack; the owner then asked to merge it down.
- Merged to `main` in order with merge commits: #11 (F008), #12 (F009), #13 (F010), #14 (F011). #15 (F012) merges when CI is green.
