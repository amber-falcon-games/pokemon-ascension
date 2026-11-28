# F019 — Energy replaces PP: costs, regeneration, recovery (the overhaul in the campaign)
Status: SPEC (building)
Dependencies: F018 (timeline), F017 spike (Dual bars, D025)
Owner decision IDs: D016, D023, D024, D025, D026
Branch / commit: `feature/F019-energy` (stacked on `feature/F018-timeline`)

## Objective and scope
Take the Dual-bars energy model from the Battle Lab into the campaign (D025/D026), so a normal playthrough uses it.

1. **Combat mode outside the Lab.**
   - A config flag `ASCENSION_COMBAT` makes every campaign battle a Dual-bars battle on the timeline: energy, Exhausted, Boost, the HUD (F018), the turn-order strip and beats.
   - It is on in dev builds and off in release until the overhaul ships. Tests turn it on per test, like the move library.
   - Link, Battle Frontier, Safari and recorded battles stay classic.
   - In the Lab, the Rules screen still decides.
2. **Energy persists between battles.**
   - Each Pokémon keeps its energy as a share of its maximum (0–100%, 7 bits). No save layout change: it uses contest fields that the cut contests no longer use (the Smart and Tough ribbon tiers and the Artist ribbon), as F015 did for talent data.
   - A battle starts from the saved share, capped by health as in battle (D024). At the end of a battle, the share is saved back.
3. **Recovery outside battle (D023):**
   - **Walking:** the party regains 1% energy every 16 steps, up to its health cap. Fainted Pokémon don't recover by walking.
   - **Pokémon Center** (and every full heal: Nurse, bed, healing events): full energy.
   - **Items** (proposed): PP items are useless once energy replaces PP, so they become energy items. Ether +25%, Max Ether +50%, Elixir +25% for the party, Max Elixir +50% for the party, Leppa Berry +10% in battle.
   - **Revive and fainting:** a revived Pokémon wakes Exhausted, at 20% energy (D023: "wakes still Exhausted").
4. **PP is hidden in the campaign.** The move menu shows the energy cost instead (as in the Lab), and PP no longer limits moves.
5. **Default rules** (Lab defaults from F017 iteration 4): ×1 energy, 50% damage and 100% costs in trainer battles. Wild battles use 75% damage, because they should be quicker (BATTLE_SYSTEM.md "Pacing"). All tunable in one table.

## Explicit exclusions
- Final balance numbers: the owner's playtest (D026).
- AI use of Boost and timeline-aware planning: F021.
- Doubles HUD art (bars outside the boxes, as now).
- Migration of old saves' contest data: before the release that turns the flag on (as F015).

## Acceptance criteria
- [ ] With `ASCENSION_COMBAT` on, a campaign trainer battle and a wild battle both use Dual bars, the timeline and the HUD, and play to the end.
- [ ] Energy carries over between battles. It recovers by walking, fully at a Pokémon Center, and with PP items as energy items.
- [ ] Release builds and the full upstream suite (flag off) are unchanged.
- [ ] A save and load keeps every party member's energy.

## Technical approach and risks
- **Combat mode:** `BattleLab_StaminaMode()`/`DualMode()` gain a campaign case: `ASCENSION_COMBAT`, not a Lab battle, and not link, Frontier, Safari or recorded. The Lab's rule state is replaced by a campaign rules record.
  - Risk: hooks that assume a Lab battle, such as the Lab energy arrays and AI timing. Mitigation: audit every `sLabInBattleFlag` use.
- **Storage:** `MON_DATA_ASC_ENERGY` (7 bits across the three fields).
  - The battle start fill and end write-back happen where the Lab does them today.
  - Walking recovery hooks the step counter (as happiness/poison do).
  - The heal hooks are `HealPlayerParty` and the item effects.
- **Risk:** contest-ribbon counts in the summary and TV. Contests and TV are cut (D011), and the summary shows no ribbon count in this build. To be checked.

## Automated tests and regression
- Energy share round-trips through boxing and save.
- Battle start/end persistence (a battle test with the flag on).
- Walking recovery, Pokémon Center heal, Ether.
- The full suite with the flag off. Lab tests.

## Manual playtest
Combined overhaul playtest (D026):
1. Start a new game in a dev build, or use a checkpoint.
2. Fight wild Pokémon and trainers. Watch energy carry over, recover while walking, and refill at a Pokémon Center.

## Resource budgets and save compatibility
- No save layout change: 7 bits of reused contest fields, documented in SAVE_COMPATIBILITY.md.
- Small EWRAM and ROM.

## Rollback
Turn `ASCENSION_COMBAT` off, or revert the branch.

## Evidence and acceptance
(to be filled in)
