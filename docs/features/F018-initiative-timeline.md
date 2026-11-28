# F018 — Initiative timeline: who acts next, and when
Status: BUILT (engine fixes and turn-order strip, Lab combat modes); awaiting the combined owner playtest (D026)
Dependencies: F016; F017 spike (go, D026)
Owner decision IDs: D016, D023 (timeline), D024 (opening round), D025, D026
Branch / commit: `feature/F018-timeline` (stacked on `feature/F016-move-info`)

## Objective and scope
Turn the F017 spike's timeline into a dependable part of the overhaul, and make the order visible. In the owner's playtest: "Turn order isn't visible... a turn-order strip should show who acts next."

1. **Clock and order (unchanged from the spike).**
   - Each battler has a time for its next action. A round lets the earliest battler(s) act.
   - Acting moves the battler's time on by 1000 × weight ÷ Speed. Weight is 100 for status moves, switches and items; damaging moves add power ÷ 3. A Speed Boost (Lab) makes the step ×0.6.
   - Opening round (D024): everyone acts, in classic priority and Speed order.
2. **End-of-turn effects on beats (fixes a spike finding).**
   - Weather, Leftovers, poison and burn, Speed Boost, screens, Taunt and other timers no longer tick after every one-action round. They run once per **beat**: when the clock crosses a multiple of 1,000 (about one plain action at Speed 100).
   - Steps that keep the battle going always run: turn order, replacements after fainting, form changes, Eject Pack and trainer slides.
   - Where several beats pass in one round, the effects run once (proposed; tune at the playtest).
3. **Recharge waits for the battler's own action (fixes a spike shortcut).** After Hyper Beam or Giga Impact, the recharge turn is the battler's next action. Rounds it sits out no longer use it up.
4. **Turn-order strip.**
   - At the top of the screen while choosing an action: "Next: Ninjask → Ninjask → Slaking → Ninjask".
   - The next four actions are predicted from the current clock, assuming an ordinary attack each (weight 115). A heavier or boosted move changes the real order, so it is a forecast.
   - In a mirror match the opponent's entries read "Foe Swampert".
5. **Where it applies.** The Lab's Stamina and Dual-bars modes, as in the spike. The campaign switches to the overhaul together with F019, which productises energy, so the two land in the campaign at once.

## Explicit exclusions
- Energy rules, costs and regen numbers: F019. The AI using Boost and planning on the timeline: F021. Queueing and cancelling: F020.
- Animated icons in the strip (proposed later if the owner wants it; text first).

## Acceptance criteria
- [ ] Speed Boost, Leftovers, poison and weather tick once per beat, not once per action.
- [ ] A recharge is spent on the battler's next own action, however many rounds pass.
- [ ] The strip shows the next four actors and matches the order the battle then follows when everyone uses ordinary attacks.
- [ ] Classic battles and release builds are unchanged. The full suite is green.

## Technical approach and risks
- **Beats:** `BattleLab_EndTurnSkips(state)` is checked in `DoEndTurnEffects`. A beat is set when a round's time enters a new multiple of 1,000. A whitelist keeps the always-run steps.
  - Risk: a timed effect hidden in an always-run step. Mitigation: the steps on the whitelist are bookkeeping only.
- **Recharge:** `TurnValuesCleanUp` decrements `rechargeTimer` only for battlers that acted (`BattleLab_SatOutLastRound`).
- **Strip:** window `B_WIN_ASC_TIMELINE` (24×2 tiles at bg0 rows 20–21, tiles 0x2D0–0x2FF, otherwise unused), drawn when the action menu opens. It is visible only in the action-menu view.

## Automated tests and regression
- The full upstream suite (timeline off in tests). Lab tests.
- Headless Lab runs with the battle log (`GBASHOT_GAME_LOG=1`).

## Manual playtest
Combined overhaul playtest (D026): `./scripts/play.sh --scenario LAB-011`.
- Check that the strip matches who actually acts.
- Ninjask's Speed Boost should now build up gradually.
- In LAB-011, Slaking's Giga Impact leaves it recharging on its next turn.

## Resource budgets and save compatibility
- About 6 bytes EWRAM and a little ROM (dev and Lab code). No save change.

## Rollback
Revert the branch. Classic battles never reach this code.

## Evidence (2026-11-28)
- **LAB-011, battle log:**
  - Ninjask's action step shrank only at the beats (700 → 466 after the beat at 1,166, then 350): Speed Boost fired once per beat.
  - The spike doubled its Speed in two rounds.
- **Recharge check** (a throwaway LAB-090 in the scratchpad: Snorlax with only Hyper Beam vs Ninjask):
  - Snorlax pays for Hyper Beam at 0, acts again at 3,000 without paying (recharge), then pays at 5,000.
  - Before the fix, the rounds it sat out used the recharge up.
- **Strip:** headless screenshots of LAB-011 show "Next: Ninjask → Ninjask → Slaking → Ninjask", updated every turn.
