# F009 — Battle Lab: Pokémon details and battle rules
Status: AWAITING_ACCEPTANCE (implemented on a branch stacked on F008, D021; owner playtest needed after F008)
Dependencies: F008 (team builders)
Owner decision IDs: D003 (Lab before combat changes), D021 (stacked branches)
Branch / commit: `feature/F009-lab-conditions` (based on `feature/F008-lab-team-builder`)

## Objective and scope
Finish the Lab's battle setup so a scenario can be described completely with upstream mechanics: what each Pokémon carries, and the conditions the battle starts in.

### Pokémon details (per slot, both sides)
The slot editor becomes a scrolling list: **Species, Level, Battle, Move 1–4, Ability, Item, Nature, IVs, HP %, Status, Reset slot, Clear slot**.
- **Moves 1–4.** Any move, or `- - -` for none. Left/Right steps ±1, L/R steps ±25, and SELECT jumps to the first move with a given letter, as for species. A debug Lab can give a Pokémon a move it couldn't learn; the editor shows a small `!` when the move isn't in the species' learnset, as an informational note only.
- **Ability.** Cycles through the species' own abilities (1, 2 and hidden), skipping empty entries.
- **Item.** Cycles through holdable items in item order, with SELECT for a letter jump. `- - -` means none.
- **Nature.** Cycles through the 25 natures and shows the boosted and lowered stats, e.g. `Adamant +Atk -SpA`.
- **IVs.** One value, 0–31, applied to all six stats.
- **HP %.** 1–100, changed in steps of 1 or 10. The Pokémon starts the battle with that share of its maximum HP.
- **Status.** None, Poison, Bad poison, Burn, Paralysis, Sleep or Freeze. Sleep lasts the engine's normal duration.

Choosing a new species fills Moves with that species' level-up moves at the current level and resets Ability to slot 1. Scenario slots start with the scenario's own values: LAB-001's Swampert keeps its moves, 31 IVs and the rest. The Pokémon that goes into battle is built from these values, so changing one field never loses the others. EVs and stat stages are not included, because upstream sets EVs per trainer and stat stages only exist inside a battle. They can be added later if a scenario needs them.

### Battle rules (Rules screen)
The Lab menu's **Rules** entry opens a list. Left/Right changes a value, B goes back, and changed values are marked `!`.
- **Format:** Trainer single (default), Trainer double or Wild single. Doubles need 2 active Pokémon on each side. Wild uses the opponent's first active slot.
- **Weather:** None, Rain, Sun, Sandstorm or Snow (Hail under older-generation config), lasting the whole battle.
- **Terrain:** None, Electric, Grassy, Misty or Psychic, lasting the whole battle. Upstream's trainer starting-status feature provides this.
- **Opponent AI:**
  - Scenario (default; for LAB-001 that's Basic Trainer)
  - Smart (upstream's "smart trainer" flag set: check bad moves, try to faint, viability, HP awareness, smart switching)
  - Random, which picks any usable move
- **Seed:** Random (default), or Fixed with a 0–65535 value set in steps of 1 or 100. A fixed seed is applied to the battle RNG just before battle start, so the same setup and the same inputs replay the same battle.

## Explicit exclusions
- Multi battles, partner trainers, Dynamax, Tera and Mega controls, and stat stages.
- Saving setups: F011. In-battle tools: F010 and F012.

## Acceptance criteria
- [ ] Every Pokémon detail above can be set on both sides, and the battle uses it: the moves, ability and item shown in battle and the summary match, HP starts at the chosen percentage, and the status icon shows.
- [ ] Unchanged LAB-001 still plays exactly as before. Editing one field of a scenario Pokémon keeps its other scenario values.
- [ ] Rules: each format starts correctly, and doubles is refused with a message if either side has fewer than 2 active Pokémon. Weather and terrain are active from turn 1, and each AI choice changes the opponent's behaviour.
- [ ] Fixed seed: running the same setup twice with the same inputs gives the same first-turn result (damage roll and critical hit). This is checked in an automated test and in the playtest.
- [ ] Release build: no Lab symbols.

## Technical approach and risks
- **Model:** `struct LabSlot` grows to hold `moves[4]`, `item`, `abilitySlot`, `nature`, `ivs`, `hpPercent` and `status` (about 20 B per slot, 240 B in all). A `struct LabRules` adds about 8 B. EWRAM, dev builds only. Reset copies the scenario's `TrainerMon` values (moves, held item, ability, nature, IVs) into the slot.
- **Build:** for a changed slot, `CreateMonWithIVs`, then `SetMonData` for moves, PP, item, ability number, nature (via the expansion's nature setter or a matching personality), HP and status. Moves are written with full PP.
- **Battle start:**
  - AI goes through upstream's `gDebugAIFlags` (debug battles). Doubles add `BATTLE_TYPE_DOUBLE`.
  - Wild battles clear the trainer override and battle flags so the battle is a plain wild encounter against the built party.
  - Weather and terrain are both set through upstream's `gStartingStatuses`, which the battle keeps from before it starts (`battle_main.c`, `UNPACK_STARTING_STATUS_TO_BATTLE`). That works for wild battles too, so no trainer copy is needed. The Lab clears it again on return.
  - A fixed seed calls `SeedRng`.
- **Risks:**
  - The longer editor list must stay usable at 240×160. It scrolls with the cursor, six rows at a time.
  - Some move and ability combinations are illegal in normal play. That is intended in a debug Lab, and the `!` notes make it visible.
  - Weather plumbing outside the overworld needs to be confirmed; see the fallback above.

## Automated tests and regression
`test/ascension/battle_lab.c`, extended:
- Defaults copy LAB-001's moves, item, nature and IVs.
- A changed slot builds with exactly the chosen moves, item, ability, nature, IVs, HP and status.
- Picking a new species refreshes its moves.
- Rules produce the right trainer copy: AI flags, terrain starting status and doubles battle type.
- A fixed seed gives the same RNG sequence twice.
- F008 tests stay green, and the full suite runs in CI.

## Manual playtest
`./scripts/play.sh --lab`, then **L + SELECT** on the title screen:
1. Player team → Swampert → set Item to Leftovers and Status to Burn. Start battle: the summary shows Leftovers and a burn.
2. Opponent team → Blaziken → set Move 1 to Flamethrower and HP % to 50. In battle, Blaziken starts at half HP and uses Flamethrower.
3. Rules → Weather: Rain, Terrain: Electric. Start battle: rain and terrain are shown on turn 1.
4. Rules → Format: Trainer double. With 1 Pokémon per side you get a message; add a second to each side and a double battle starts.
5. Rules → Seed: Fixed 1234. Use the same move twice in two runs; the damage is identical.

## Resource budgets and save compatibility
About 250 B of EWRAM and a few KB of ROM, dev builds only. Release builds: 0. No save layout change.

## Rollback
Revert the PR. The Lab falls back to F008.

## Evidence and acceptance
Implemented 2026-11-26 (Linux container):
- `make -j8`: exit 0, no warnings. `make check TESTS=test/ascension/battle_lab.c`: **13/13 PASS**: 7 F008 tests plus 6 new ones (scenario defaults, a changed slot built with every detail, new species refreshing its moves, move and item lists, rules for AI/weather/terrain/doubles, fixed-seed RNG repeat).
- Nature is written as upstream's *hidden nature*, which stat calculation (`pokemon.c`, `CalculateMonStats`) and the summary use. The personality nature stays random.
- Headless screenshots (gbashot):
  - The editor list scrolls. The letter jump puts Night Shade in Move 1, marked `!` as not in Blaziken's learnset. Ability changes to Speed Boost. Item, HP 50% and Burn are set.
  - Rules screen shows Rain, Electric and Smart, each marked `!`.
  - Doubles with 1v1 teams are refused ("Doubles: you need 2 in battle"). With 2v2 teams a real double battle starts, showing "An electric current is running across the battlefield!" then "It started to rain!".
- Release: `nm pokeemerald-release.elf | grep -ci "BattleLab\|Ascension_C"` = 0.
- Dev cost vs F008: ROM +5,361 B, EWRAM +200 B. Release: 0.
- Not checked headlessly: fixed-seed replay in a real battle (the RNG repeat is unit-tested), and the wild format and AI behaviour differences. These are part of the owner playtest.
- The GBA font has no `←`, `→` or `±`, so the help text uses the game's arrow glyphs and `+-`.
