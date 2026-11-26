# F017 — Feasibility spike: timeline, energy, more than 4 moves, AI time
Status: IN_PROGRESS (model D agreed, D023; spike being built)
Dependencies: F013 (Battle Lab accepted, 0.2.0)
Owner decision IDs: D015 (more than 4 moves, talent tree), D016 (energy replaces PP; order from energy, recovery and Speed); new decisions requested below
Branch / commit: `feature/F017-energy-spike`

## Objective
Before committing to F018–F021, prove on the real GBA engine that the core combat redesign works. The spike builds rough prototypes and **measures** them; it is not the finished feature:
1. **Timeline turn order.** Battlers act one at a time when their time comes, instead of in classic rounds.
2. **Energy instead of PP**, tied to turn order (D016).
3. **More than 4 moves** per Pokémon in battle (D015), with the AI still choosing well.
4. **AI time.** The opponent must still decide within a frame budget when choosing from more moves on a timeline.
5. **Platform checkpoint.** At the end there is a go/no-go: stay on the GBA engine, or reconsider the platform. The evidence decides, and the owner makes the call.

Everything sits behind a **Battle Lab only** Rules toggle (`Turn order: Classic / Timeline`). The campaign and release builds are untouched. Classic stays the default until F018–F019 are approved.

## Model: D, the stamina timeline (owner decision D023)
The owner's design replaces options A–C (2026-11-26).

**Rules:**
- **One energy bar replaces HP and PP.** Max energy is the Pokémon's max HP (species, level, evolution), and damage drains it as today.
- **Moves cost energy** as a share of the user's max:
  - damaging moves: about power ÷ 6 % (Tackle 40 → 7%, Flamethrower 90 → 15%, Hyper Beam 150 → 25%)
  - status moves: 5%
  - Protect-type and recovery moves: free

  Your own move never takes you below 1. These are starting numbers, tuned in the Lab.
- **Exhausted** applies at ≤ 25% energy, from any cause: Speed ×0.75, move power ×0.85, accuracy ×0.9. Moves with power ≥ 100 are locked, and the Pokémon is easier to hit (opponents' accuracy +10%).
- **Regeneration:** +2% of max energy at the end of each turn, but not on a turn the Pokémon protected.
- **Protect:** upstream's consecutive-use rule (1/3, then 1/9, …).
- **Timeline turn order:** faster Pokémon act more often (spike step 3).
- **Boost:** R on a move opens Speed / Power / Both. Speed costs +50% (acts sooner), Power costs +50% (×1.3 power), Both costs +100%. The cost is previewed on the energy bar.

**Spike build order** (each step is playable in the Lab):
1. energy costs on one bar
2. Exhausted
3. timeline
4. Boost with the cost preview
5. scenarios

Everything is behind the Lab Rules row **Combat: Classic / Stamina**.

Out of scope for the spike: carrying energy over between battles, recovery while walking, and fainted Pokémon waking up. Those are campaign work in F019, and the save layout needs no change for them, since the HP field becomes energy.

## What the spike measures (the go/no-go evidence)
| Question | How | Go if |
|---|---|---|
| Can the upstream battle loop run 1-action rounds without breaking abilities, items, weather and end-of-turn effects? | Prototype in a Lab toggle, plus the full upstream battle test suite with the toggle off, plus timeline Lab scenarios | Suite green; LAB-001, LAB-009 and the new scenarios play to the end |
| More than 4 moves in battle | Party Pokémon carry an extended list (e.g. 8) **in RAM during battle only**. The menu is a scrolling list, and boxed Pokémon keep 4 (save limits, SAVE_COMPATIBILITY.md) | List works in the Lab; EWRAM cost measured |
| AI time with 8 moves on a timeline | Count frames per AI decision (mGBA cycle counter in the battle log) | Under 2 frames on average, no visible stall |
| Memory and ROM | Size deltas | Well within the budgets (about 6.8 MB ROM free; about 45 KB EWRAM free in dev builds) |

Platform checkpoint: if any row is a clear "no", the spike reports why and the options (narrow the design, or another platform), and **you decide**. No further combat work happens until then.

## New Battle Lab scenarios (the "feel" test)
- **LAB-011 Fast vs slow:** Ninjask against Slaking. Does Speed feel right without one side getting every turn?
- **LAB-012 Big vs small moves:** the same Pokémon, one side with a single strong move and the other with cheap ones. Is "wait for the big one" a real choice?
- **LAB-013 Long fight:** 3v3 with healing. Does energy prevent endless stalling?

Each runs under Classic and under the prototype, so they can be compared directly.

## Explicit exclusions
- Final balance: F019.
- The polished timeline UI: F018.
- Queueing, cancelling and intervening: F020.
- Saving more than 4 moves for boxed Pokémon: F015.
- Any campaign or release change.

## Test plan
- Upstream battle test suite green with the toggle off: no behaviour change by default.
- New tests: timeline ordering, energy arithmetic, and the out-of-energy path.
- Lab scenarios above, under both turn-order modes.
- Battle-log diffs between modes.

## Resource budgets and save compatibility
Dev-only prototype code and data. No save change: extended move lists exist only in battle RAM.

## Rollback
Revert. Everything sits behind a Lab-only toggle.

## Decisions
All taken: D023 (2026-11-26). Numbers are tuned in the Lab, and the owner judges the feel.
