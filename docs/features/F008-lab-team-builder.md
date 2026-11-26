# F008 — Battle Lab team builders (player and opponent)
Status: AWAITING_ACCEPTANCE (spec approved by the owner 2026-11-26; implemented; owner playtest needed)
Dependencies: F007 (Battle Lab entry, accepted 2026-11-26)
Owner decision IDs: D003 (Lab before combat changes)
Branch / commit: `feature/F008-lab-team-builder`

## Objective and scope
Set up both sides of a Lab battle from inside the Lab: up to 6 Pokémon per side, any species or form, any level, and which members take part. This replaces the fixed LAB-001 teams as the only option, while LAB-001 stays the default you start from.

- **Where:** the Lab menu's *Player team* and *Opponent team* entries (placeholders in F007) open a team screen for that side.
- **Team screen:** six rows, one per slot, each showing the slot number, species name, level and markers:
  `1 Swampert  Lv50` · `2 Pikachu!  Lv12` · `3 off Gengar Lv40` · `4 - - -`.
  `!` means the slot differs from the scenario (a debug override). `off` (greyed) means it stays out of the battle. `- - -` is an empty slot. The GBA font has no `*`, `#` or `—`, so `!` and `No.` are used instead.
  Up/Down moves between slots, A edits the slot, B goes back to the Lab menu, and START returns both teams to the scenario defaults after a Yes/No check.
- **Slot editor:** a small panel with *Species*, *Level*, *In battle*, *Reset slot* and *Clear slot*.
  - **Species:** Left/Right steps ±1 and L/R jumps ±25 through every species the game has enabled, forms included, in National Dex order. Holding a button repeats. SELECT opens a letter picker (A–Z) that jumps to the first species starting with that letter. The panel shows the name and dex number, plus `form N` when the entry is an alternate form of the same species.
  - **Level:** Left/Right ±1, L/R ±10, limited to 1–100.
  - **In battle:** Yes/No. Members set to No stay on the team screen but aren't sent into battle.
  - **Reset slot** restores the scenario's Pokémon for that slot. **Clear slot** empties it.
- **Party building at Start battle:** for each side, the active slots in slot order become the party, so the first active slot leads.
  - An unchanged slot is built exactly as the scenario defines it (moves, IVs, item), so LAB-001 plays as before.
  - A changed slot is built fresh: that species at that level with its level-up moves, 31 IVs and no item. Choosing moves, abilities and items is F009.
- **Validation (Start battle):** each side needs at least one active Pokémon. Otherwise the Lab stays on the menu and names the side that's empty. Species and level can't go out of range because the controls clamp them.
- **Lifetime:** edits last for the whole Lab visit, through any number of battles, and are lost on Exit. Saved scenarios are F011.
- **Header:** the Lab title shows `LAB-001 (edited)` once anything differs from the scenario.

## Explicit exclusions
- Moves, abilities, held items, natures, IVs/EVs, HP and status: F009.
- Battle rules such as doubles, wild battles, weather and AI: F009.
- Edit-and-relaunch from inside a battle: F010. Saving or loading teams: F011.
- Pokémon icons or sprites on the team screen: text only for now, though icons can be added later if you want them.
- Any change to campaign battles or release builds.

## Acceptance criteria
- [ ] On both team screens, LAB-001 is pre-filled, with Swampert on the player's side and Blaziken on the opponent's.
- [ ] Any species and form can be picked with the stepping, jump and letter controls. Level can be set from 1 to 100. Slots can be switched off, cleared and reset.
- [ ] Changed slots show `!`, and the title shows `(edited)`. START resets everything.
- [ ] Start battle runs a real battle with exactly the active members, in slot order, at the chosen levels, on both sides. A 6v6 battle works (LAB-009 setup).
- [ ] An unchanged LAB-001 still plays exactly as it does in F007.
- [ ] A side with no active members blocks Start battle with a clear message.
- [ ] Edits survive returning from a battle. Exit still soft-resets, and the campaign save is untouched.
- [ ] Release build: no Lab symbols (`nm` check, unchanged).
- [ ] Owner playtest on the Mac.

## Technical approach and risks
- **Model (pure C, testable):** `struct LabSlot { u16 species; u8 level; u8 flags; }` with active and modified bits, and `struct LabTeam { struct LabSlot slot[PARTY_SIZE]; }` for each side: 2 × 6 × 4 B = 48 B of EWRAM, dev builds only. Functions:
  - `LabTeam_InitFromScenario`
  - `LabSlot_StepSpecies(dir, amount)`, which skips disabled species with upstream's `IsSpeciesEnabled`
  - `LabSlot_JumpToLetter`, `LabSlot_SetLevel`, `LabTeam_ActiveCount`
  - `LabTeam_BuildParty(side, party)`
  These are declared in `battle_lab.h` so tests can call them.
- **Building parties:** first build the scenario party into a scratch party with upstream's `CreateNPCTrainerPartyFromTrainer`. Then, for each active slot, copy the scenario Pokémon if the slot is unchanged, or call `CreateMon` (fixed 31 IVs, which also gives the level-up moveset) if it's changed.
- **Engine compatibility:** upstream doesn't rebuild the enemy party during debug battles (`src/battle_main.c:563`, which checks `gIsDebugBattle`). So the party prepared by the Lab is the party the battle uses, with no new upstream hooks. The trainer override still supplies the opponent's name, class and AI.
- **UI:** extends the existing Lab screen with a team window, a slot-editor window and a letter-picker window, built from the same upstream window and menu helpers. The layout is prototyped at 240×160 and checked with gbashot screenshots before owner review.
- **Risk: the species list is large (about 1,500 entries including forms).** Mitigation: ±25 jumps, held-button repeat and the letter picker. A text search is too clumsy on a GBA pad.
- **Risk: some forms can't exist in battle on their own (Mega/Gigantamax/battle-only forms).** Mitigation: hide forms upstream marks as battle-only (`isMegaEvolution`, `isGigantamax`, `isPrimalReversion`, `isUltraBurst`, `isTeraForm` and similar flags). The full list is decided during implementation and recorded in Evidence.
- **Risk: the trainer's declared party size differs from the real party.** Mitigation: the battle counts the real party (`CalculateEnemyPartyCount`). A 6v6 test battle confirms the ball display and the switch logic.

## Automated tests and regression
`test/ascension/battle_lab.c` (extended):
- Defaults equal LAB-001, and an unchanged build gives the same parties as F007's test.
- Species stepping skips disabled species and wraps at both ends; the letter jump lands on the first matching name; level is clamped to 1–100.
- Building parties: inactive and empty slots are skipped; order follows slot order; species and level are as chosen; a changed slot gets level-up moves; an unchanged slot keeps the scenario moves.
- Validation: a side with no active members is reported.
- Full suite in PR CI. Release `nm` check as in F007.

## Manual playtest
`./scripts/play.sh --lab`, then **L + SELECT** on the title screen:
1. Open *Opponent team*. Slot 1 shows BLAZIKEN Lv50.
2. Slot 2 → Species: press SELECT, pick G, step to GENGAR. Level: L/R to 40. Back with B, and the row shows `Gengar! Lv40`.
3. Fill the player's side up to 3 Pokémon, and set one of them to *In battle: No*.
4. Start battle: the opponent leads with Blaziken and has 2 Pokémon; you have 2 active. Win or lose, you return to the Lab and your edits are still there.
5. Clear every opponent slot, then try Start battle: you get a message and no battle.
6. Press START on a team screen and confirm: everything returns to LAB-001. Exit → title. Continue still loads your campaign save.

Report anything that's awkward to use, especially finding species.

## Resource budgets and save compatibility
48 B of EWRAM plus a few KB of ROM, dev builds only. Release builds: 0. No save layout change; the Lab never saves.

## Rollback
Revert the PR. The Lab falls back to fixed LAB-001.

## Evidence and acceptance
Implemented 2026-11-26 (Linux container):
- `make -j8`: exit 0, no warnings. `make check TESTS=test/ascension/battle_lab.c`: **7/7 PASS**. The tests cover LAB-001 parties, defaults and the edited flag, dex-order stepping with wrap and the battle-only-form filter, the letter jump, level clamping, building parties from active slots in order (unchanged slots keep scenario moves, changed slots get level-up moves), and empty-side detection with reset and clear.
- Battle-only forms hidden: `isMegaEvolution`, `isPrimalReversion`, `isUltraBurst`, `isGigantamax`, `isTeraForm`. Eggs and disabled species are also excluded.
- Headless screenshots (gbashot):
  - Lab menu → Opponent team (Blaziken) → slot editor (No.0257, level, Battle, Reset, Clear) → letter grid.
  - H jumps to Haunter; level 49; slot 2 is stepped to Ivysaur; the title reads "(edited)".
  - Main panel shows "Foe: 2". A real battle starts with the opponent's two Pokémon.
- Release: `arm-none-eabi-nm pokeemerald-release.elf | grep -ci "BattleLab\|Ascension_C"` = 0.
- Dev cost vs main: ROM text +4,810 B, EWRAM +56 B. Release: 0.
- Not checked headlessly: a full 6v6 played to the end (covered by the party-building test, plus the owner playtest).
