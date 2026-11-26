# F017 — Feasibility spike: timeline, energy, more than 4 moves, AI time
Status: SPEC — OWNER DECISIONS NEEDED (model to prototype, and four starting choices, below)
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

## The three models (owner: choose one to prototype, or two to compare)
All three keep the player in control: the timeline **pauses while you choose**, and nothing acts on its own (GAME_DESIGN.md). Numbers are starting points for the Lab, not final balance.

### A. Charge timeline (FFX-style)
- Every battler has a **time counter**. The battler with the lowest counter acts next.
- After acting, its counter grows by the **move's recovery** ÷ **Speed** factor: fast Pokémon and light moves come round sooner.
- **Energy:** a pool (e.g. 100). Each move costs energy, and every battler regains a little whenever anyone acts. At 0 the Pokémon can only Rest, which restores energy but costs time.
- *Feel:* very readable, and you can show the next 5–6 turns in a strip on screen. Speed matters a lot.
- *Engine cost:* medium. Upstream's turn loop collects an action from every battler each round. The spike makes the round 1 action long.

### B. Energy bar (ATB-style; recommended)
- Each battler's **energy bar fills over time** at a rate set by **Speed**. You may act once your bar covers the move's **cost**.
- Big moves cost more, so you wait longer to use them; small moves let you act often. After acting, the move's **recovery** briefly slows refilling.
- Energy *is* the time resource, so there is one number to balance, and stalling is self-limiting (no resource is free).
- *Feel:* tense and flexible: "hit now with a jab, or wait for the big one?" This fits D016 most directly: order comes from energy, recovery and Speed.
- *Engine cost:* the same as A (1-action rounds), plus a bar per battler in the UI.

### C. Rounds with energy (smallest change)
- Keep classic rounds: everyone picks, then actions resolve in order.
- Order is **Speed adjusted by the previous move's recovery**, so a heavy move makes you slower next round.
- **Energy** replaces PP as a pool, with a per-move cost and a per-round refill.
- *Feel:* familiar Pokémon rhythm with a resource twist. It is the least "timeline" of the three.
- *Engine cost:* low (it mostly changes turn order and PP), but it may not deliver the battle feel you're after.

**Recommendation: B**, with A as a fallback if the bars feel fiddly on a GBA screen. The spike can prototype **B and C side by side** for little extra cost (they share the energy code), so you can feel the difference in the Lab before choosing.

## Starting choices (owner: accept or change)
1. **Energy scale:** 100 per Pokémon. Move cost scales with power (e.g. Tackle 15, Flamethrower 35, Hyper Beam 70), and status moves cost 10–25.
2. **Speed effect:** fill rate or time gain is proportional to Speed, clamped to between 0.5× and 2× of the average, so slow Pokémon still get turns.
3. **Out of energy:** you can only Rest, which skips your action and refills energy faster. There's no Struggle.
4. **Selecting pauses the timeline** for both sides; the AI decides instantly when its turn comes.

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

## Decisions needed from the owner
1. **Model:** A, B, C, or **B and C side by side** (recommended).
2. **Starting choices 1–4:** accept or adjust.
3. **Spike length:** about one working session for the prototype and measurements, then a report with the go/no-go evidence.
