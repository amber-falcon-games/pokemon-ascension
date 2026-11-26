# F010 — Battle Lab: play, restart, edit and exit loop
Status: AWAITING_ACCEPTANCE (implemented on a branch stacked on F009, D021; owner playtest after F009)
Dependencies: F009
Owner decision IDs: D003, D021
Branch / commit: `feature/F010-lab-battle-loop` (based on `feature/F009-lab-conditions`)

## Objective and scope
Make iterating on a scenario fast: play it, see how it ended, restart it unchanged or tweak it and go again, all without leaving the Lab.

Already in place from F007–F009: Start battle plays the current setup, every battle returns to the Lab, edits persist, and the Lab never writes the save. F010 adds:
- **Result line.** After a battle, the Lab's title bar shows how it ended, and after how many turns for wins and losses: `… · Won 4t`, `· Lost 7t`, `· Ran`, `· Quit`. It stays until the next battle. It's in the title bar because the main panel's six lines are full.
- **Restart during a battle.** Upstream's in-battle debug menu (SELECT during a dev-build battle) gains **Lab: Restart**. It ends the battle and immediately starts the same setup again: same teams, same rules and the same fixed seed if one is set. Nothing from the abandoned battle carries over, because both parties are rebuilt from the Lab slots.
- **Quit during a battle.** The same menu gains **Lab: Quit**, which ends the battle and returns to the Lab menu so you can edit and relaunch.
- Both entries do nothing outside Lab battles: they play the "can't" sound, so campaign debug battles are unaffected.

The in-battle debug menu itself (stats, stat stages, status, hazards, AI scores, Instant Win) is upstream's; extending it is F012.

## Explicit exclusions
- Changing the setup without leaving the battle.
- Battle replays and recordings.
- Saving scenarios: F011.

## Acceptance criteria
- [ ] After winning, losing, running or quitting, the title bar shows the right result, with the turn count for wins and losses.
- [ ] Lab: Restart starts an identical battle with full HP, the original moves and items, and the same weather and terrain. With a fixed seed, the first turn replays identically.
- [ ] Lab: Quit returns to the Lab menu with every edit kept.
- [ ] In a non-Lab debug battle (overworld debug menu), both entries do nothing.
- [ ] Release build: no Lab symbols. The battle debug menu is already excluded from release builds upstream.

## Technical approach and risks
Upstream changes, all small and marked `Pokémon Ascension`:
- **`battle_main.c`:** `BattleDebug_EndBattle(outcome)`, which generalises upstream's `BattleDebug_WonBattle`. It sets the outcome and the matching end-of-battle step, and marks the turn's actions finished so the battle ends without waiting for the player's choice.
- **`battle_debug.c`:** two list entries plus their handlers, which call the Lab.

Lab side:
- Flags for "Lab battle active", "quit requested" and "restart requested".
- The last outcome and turn count (`gBattleResults.battleTurnCounter`) are stored on return.
- On a restart, the Lab skips its menu and runs Start battle again after re-initialising.

Risks:
- **Ending a battle from the debug menu mid-turn.** This uses the same path as upstream's Instant Win, with outcome `B_OUTCOME_PLAYER_TELEPORTED`, which goes straight to `HandleEndTurn_FinishBattle` with no message. Found during implementation: a "drew" outcome prints "You have no more Pokémon that can fight!", and without the finished-actions mark the battle waits for the player's next action.

## Automated tests and regression
`test/ascension/battle_lab.c`:
- Result text for each outcome.
- The restart and quit flags are only honoured while a Lab battle is active.
- F008 and F009 tests stay green. Full suite in CI.

## Manual playtest
`./scripts/play.sh --lab`, then L + SELECT:
1. Start battle, win or lose, and check the result line.
2. Start battle, use one move, then press SELECT → **Lab: Restart**. The battle starts again from the beginning, at full HP.
3. SELECT → **Lab: Quit**. You're back in the Lab with your edits; the result reads `Quit`.
4. Rules → Seed: Fixed. Restart twice and use the same move each time; the damage is identical.

## Resource budgets and save compatibility
A few bytes of EWRAM and under 1 KB of ROM, dev builds only. No save change.

## Rollback
Revert the PR.

## Evidence and acceptance
Implemented 2026-11-26 (Linux container):
- `make -j8`: exit 0, no warnings. `make check TESTS=test/ascension/battle_lab.c`: **15/15 PASS** (13 from F008/F009, plus result mapping and restart/quit refused outside Lab battles).
- Headless screenshots:
  - In a LAB-001 battle, SELECT opens upstream's debug menu with **Lab: Restart** and **Lab: Quit** after Instant Win.
  - Restart: within about 150 frames the same battle starts again from the beginning ("You are challenged by…"), at full HP.
  - Quit: back in the Lab menu, with the title ending `· Quit`.
- Release: `nm pokeemerald-release.elf | grep -ci "BattleLab\|Ascension_C"` = 0. The battle-debug hook is compiled only when `ASCENSION_BATTLE_LAB` is on.
- Dev cost vs F009: ROM +648 B, EWRAM +4 B. Release: 0.
- Upstream touch points: `src/battle_main.c` (+9 lines), `include/battle_main.h` (+1), `src/battle_debug.c` (+2 list entries and a 15-line handler), all marked `Pokémon Ascension (F010)`.
