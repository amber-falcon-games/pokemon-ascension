# F007 — Battle Lab entry, shell menu and LAB-001 baseline battle
Status: AWAITING_ACCEPTANCE (implemented; owner playtest needed)
Dependencies: F006 tooling (merged); the owner's 0.1.0 Mac verification can run in parallel
Owner decision IDs: D003 (Lab mandatory before combat changes)
Branch / commit: `claude/sharp-euler-yfhti2`

## Objective and scope
A developer-only way to go straight from boot into the Battle Lab and fight a real battle, with no campaign save involved.
- **Entry:** in non-release builds, the title screen accepts **L + SELECT** to open the Battle Lab (beside upstream's SELECT quickstart). `./scripts/play.sh --lab` builds and opens the dev ROM and prints the key combination. *Change from the first draft:* no separate boot-to-Lab build flag. Upstream's Makefile doesn't rebuild objects when preprocessor flags change, so a flag-switched build risks stale objects; L + SELECT is one key press.
- **Lab shell:** a GBA menu with the tabs Player Team | Opponent Team | Rules | Scenario | Start Battle, as in BATTLE_LAB.md. In F007 only Start Battle and Exit are active; the other tabs show "Coming in F008/F009" so the structure is visible from the first build.
- **LAB-001 baseline:** a fixed 1v1 trainer battle (level 50 vs level 50; species chosen from upstream test conventions and documented in the scenario registry) using the real battle engine: the Lab sets both parties from `src/data/battle_lab_trainers.party`, points upstream's debug-battle trainer lookup at the scenario opponent (`gDebugAiTrainerOverride`, a three-line hook in `src/debug.c`), and enters `CB2_InitBattle` directly with a Lab callback that returns to the Lab menu instead of the overworld.
- **Isolation:** the Lab sets up save blocks in memory as a fresh game (`NewGameInitData`) and never calls a save function. The battle's XP, items and money changes are thrown away on return. A campaign save on the cartridge is untouched (test: save size and contents are unchanged after a Lab session).
- **Release exclusion:** all Lab code is behind `#if !defined(RELEASE)` (or upstream's `DISABLED_ON_RELEASE`). `make release` contains no Lab symbols (verified with `nm`).
- `docs/BATTLE_LAB_SCENARIOS.md`: scenario registry, starting with LAB-001.

## Explicit exclusions
Team builders (F008), conditions/moves/items/AI editing (F009), edit-and-relaunch (F010), saved scenarios (F011), inspection tools (F012). No gameplay changes to campaign battles.

## Acceptance criteria
- [ ] Owner: in a dev build, L+SELECT on the title screen opens the Lab (`./scripts/play.sh --lab` prints the keys).
- [ ] Start Battle runs LAB-001 in the real engine; winning, losing or running returns to the Lab menu; Start Battle can be repeated.
- [ ] Exit returns to the title screen. No save is written (checked by comparing the `.sav` file before and after).
- [x] `make release`: no Lab symbols at all (`arm-none-eabi-nm pokeemerald-release.elf | grep -i BattleLab` is empty; the dev ELF has `BattleLab_Open`, `CB2_BattleLab_Init` and the rest). The check also runs in the nightly build.
- [x] Automated: `test/ascension/battle_lab.c`, LAB-001 parties (species, level, first move, party size): 1/1 PASS. Full suite: in PR CI.
- [x] Resource delta (dev build vs F005b): ROM +2,320 B (26,744,176 → 26,746,496), EWRAM +8 B, IWRAM 0. Release build: 0.
- [ ] Owner playtest of the flow on the Mac.

## Technical approach and risks
- Reuse upstream's debug-battle mode (`gIsDebugBattle`), `CreateNPCTrainerPartyFromTrainer` and the `.party` text format (new file `src/data/battle_lab_trainers.party`, generated to a header by the existing build rule), so LAB-001 is data, not code. Upstream changes are three small hooks: title-screen L+SELECT, the trainer-override pointer in `debug.c`/`debug.h`, and one `AUTO_GEN_TARGETS` line.
- Risk: starting a battle without an overworld loaded. Mitigation: the end callback returns to the Lab menu's own callback; the battle start transition is chosen so it does not require map state. Fallback: warp to a tiny Lab map (a single room) and start from there.
- Risk: title-screen input conflicts with upstream's quickstart. Mitigation: L+SELECT is checked before plain SELECT.

## Manual playtest (owner)
`./scripts/play.sh --lab` → Lab menu → Start Battle → fight LAB-001 → back in the Lab → Start Battle again → Exit. Then a normal `./scripts/play.sh`: Continue still loads your campaign save as before.

## Resource budgets and save compatibility
No save layout change. The Lab never writes the save. Expected ROM growth: a few KB.

## Rollback
Revert the PR; release builds are unaffected either way.

## Evidence and acceptance
Built and tested in the Linux container 2026-11-26 (`make -j4` exit 0; test above; release `nm` check). Not verifiable here: the on-screen flow (no way to drive the emulator UI headlessly), so the owner playtest is the acceptance test for entry, battle, return and exit.
