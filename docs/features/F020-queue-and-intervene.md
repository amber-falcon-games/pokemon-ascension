# F020 — Queue, cancellation and manual intervention
Status: BUILT; awaiting the combined owner playtest (D026)
Dependencies: F019
Owner decision IDs: D028 (2026-11-28), D023, D026
Branch / commit: `feature/F020-queue` (stacked on `feature/F019-energy`)

## Objective and scope
On the timeline a fast Pokémon often acts several times in a row. Let the player line up the next action, so the battle doesn't stop for a menu each time, while keeping control: "dynamic queued commands with manual intervention, not autonomous battling" (GAME_DESIGN.md).

1. **Queue up to 2 (D028-1).** In the move menu, SELECT marks the highlighted move as the follow-up.
   - The cost box reads "Next". SELECT again on it unqueues it.
   - A then chooses the action for now, as usual.
   - At your Pokémon's next turn the follow-up runs without the menu: "Ninjask follows up with X-Scissor!" The target is the one you chose for the first action.
2. **Automatic cancel (D028-2):** the menu comes back instead when:
   - the target fainted or left the field
   - your Pokémon became Exhausted
   - it lost 25% or more of its health since you queued
   - it got a new status condition, or became confused
   - it switched out, or the move can no longer be used (Disable, Taunt, too little energy…)
3. **B clears the queue (D028-3)** while actions resolve. No time jump: you choose at your next turn. Reacting out of turn is F022.
4. **Free (D028-4):** energy is paid only when an action runs.
5. **Where:** battles with energy (Lab Stamina/Dual, campaign with `ASCENSION_COMBAT`). It is your Pokémon only; an in-game partner's are not queued.

## Explicit exclusions
- The AI queueing: the AI decides every action (F021).
- Queueing switches or items. Out-of-turn reactions: F022.

## Acceptance criteria
- [ ] SELECT queues and unqueues a follow-up, shown as "Next" in the cost box.
- [ ] The follow-up runs at the next turn with the "follows up" message, then the menu returns.
- [ ] Every D028 cancel rule sends you back to the menu. B clears the queue.
- [ ] Classic battles and release builds are unchanged. The full suite is green.

## Technical approach and risks
- `battle_lab.c` keeps one queued move per battler. It is armed when the current action is confirmed (`STATE_WAIT_ACTION_CASE_CHOSEN`), with snapshots of health, status, confusion and target.
- At the battler's next action selection (`HandleTurnActionSelectionState`), a valid follow-up fills in the move, slot and target the way upstream's Encore path does, and skips the controller.
- The rules are a pure function, `BattleLab_QueueStillValid`, unit-tested.
- Risk: B is also used to speed up battle text, so tapping B through messages clears the queue. This follows D028-3; if it feels too eager in the playtest, a hold-B or L+B alternative is easy.

## Automated tests and regression
- `test/ascension/queue.c`: the D028 rules (nothing changed, a small hit, a big hit, new or old status, confusion, Exhausted, target fainted or left, switched, unusable move, self-targeted moves).
- The full suite. Battle tests can't press SELECT, so the queue path is checked headless.

## Manual playtest
`./scripts/play.sh --scenario LAB-011`, then:
1. In the move menu, press SELECT on X-Scissor (the box reads "Next 17").
2. Press A.
3. Watch Ninjask follow up on its next turn without the menu.
4. Try B during the battle text to clear a queued move.

## Resource budgets and save compatibility
About 60 bytes EWRAM. No save change.

## Rollback
Revert the branch. The queue is only reached in energy battles.

## Evidence (2026-11-28)
- Headless LAB-011:
  - SELECT then A: the cost box shows "Next 17" and X-Scissor is used.
  - Next turn: "Ninjask follows up with X-Scissor!" without the menu. The log shows the energy paid for both. Then the menu returns.
- Tests: `queue.c` 2/2.
