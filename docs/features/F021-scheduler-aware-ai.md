# F021 — Scheduler-aware enemy AI and full battle regression
Status: BUILT; awaiting the combined owner playtest (D026)
Dependencies: F020
Owner decision IDs: D023 (timeline), D026 (build the whole overhaul, then a combined playtest)
Branch / commit: `feature/F021-ai` (stacked on `feature/F020-queue`)

## Objective and scope
Opponents play by the overhaul's rules as well as the player does. Then run every Lab scenario end to end on the finished overhaul.

1. **The AI uses Boost (R).** It pays the same boosted cost as the player. It boosts only if, after paying, it still has half its maximum energy (a reserve, so it does not exhaust itself).
   - **Power (x1.3):** when the extra power turns a hit that would not knock its target out into one that would. The estimate is upstream's median damage roll.
   - **Speed (step x0.6):** when the shorter step lets it act again before its target's next action, which it would not otherwise do. The estimate comes from the timeline clock.
   - Status moves are never boosted.
2. **AI decision time.** The cost of each decision is measured in the Lab log ("LAB ai: X decided in N ticks"). The `sActiveCombat` cache (F019) removed the overhaul's own overhead from the AI's hot path.
3. **Full battle regression.** Every Lab scenario is played to the end headless, on the Lab ROM, with the battle log.

## Explicit exclusions
- The AI queueing follow-ups (F020): it decides every action anyway, so a queue would add nothing.
- New AI scoring for energy costs (preferring cheap moves when low). Proposed for after the playtest, if opponents exhaust themselves too often.
- Reactive combat: F022–F026.

## Acceptance criteria
- [ ] The AI boosts by the rules above, and never below its reserve.
- [ ] Classic battles and release builds are unchanged. The full suite is green.
- [ ] Every Lab scenario plays to the end without a softlock.

## Technical approach and risks
- `BattleLab_AiBoostChoice(const struct AscAiBoostCheck *)` is a pure function, unit-tested. `BattleLab_AiChooseBoost(battler)` fills it from the AI's simulated damage and the timeline clock. It is called in `HandleTurnActionSelectionState` once the AI's move and target are set.
- Only AI battlers on the opposing side, in energy battles, targeting a player-side Pokémon.
- Risk: the AI's damage estimate ignores crits and random rolls, so a Power Boost sometimes still misses the knock-out. This is acceptable: the player's own estimate is no better.

## Automated tests and regression
- `test/ascension/ai_boost.c`: the rules (reserve, knock-out threshold, Speed overtaking, status moves).
- The full suite.
- Headless regression: every Lab scenario on the Lab ROM, mashing A with an occasional DOWN so forced switches pick a healthy Pokémon.

## Manual playtest
Combined overhaul playtest (D026): `./scripts/play.sh --scenario LAB-014`.
- Watch the opponent's moves: "boosted" moves cost it more energy.
- `./scripts/play.sh --scenario LAB-011`: Slaking and Ninjask, timeline and Speed Boost.

## Resource budgets and save compatibility
No EWRAM beyond the existing boost flags. A little ROM. No save change.

## Rollback
Revert the branch. Classic battles never reach this code.

## Evidence (2026-11-28)
- `test/ascension/ai_boost.c`: 2/2 pass.
- **Headless regression**, Lab ROM, `GBASHOT_GAME_LOG=1`, mashing A for 700–1,600 presses:

  | Scenario | Result |
  | --- | --- |
  | LAB-011 | 10 turns; 1 AI Boost |
  | LAB-012 | 13 turns |
  | LAB-014 | 21 turns; 7 AI Boosts; ended "Lost after 4 turn(s)" |
  | LAB-010 (Classic) | played to the end: "Lost after 3 turn(s)" |
  | LAB-013 | 19 turns |
  | LAB-009 (Classic) | stopped in the party menu |

  - Every stop was the input script, not the game. Mashing A after a faint keeps choosing the fainted or active first slot ("has no energy left to battle", "already in battle").
  - No softlock, crash or assertion attributable to the overhaul was seen.
- **AI-vs-AI** (upstream's `IsAiVsAiBattle` forced on in a throwaway build, never committed) is not a usable harness here.
  - Classic LAB-009 and LAB-010 hit upstream's switch-in assertion (`battle_script_commands.c:3201`) with it too, so it is the harness, not the overhaul.
  - Not used as evidence.
- **AI decision time**, from the battle log (4,389 ticks = 1 frame):
  - Dual-bars singles: mean of about 0.9 frames; worst 2.1 frames (LAB-012).
  - The Classic 6v6 opener (LAB-009) takes 13 frames, which is upstream's switch analysis. It is unchanged by the overhaul.
- The full suite (`make -j check` at the commit below the base 46ddea0e96 plus this change): exit 0, 5,490 passed, 19 known-failing (upstream), 0 failed.
