# F017 — Feasibility spike: timeline, energy, more than 4 moves, AI time
Status: OWNER PLAYTEST (spike steps 1–5 built; go/no-go decision pending)
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

## Spike findings so far
- **Step 1, energy costs (done).** Moves spend energy from the HP bar instead of PP. LAB-012 log: Hydro Pump costs 31 of 175, Mud-Slap 5.
- **Step 2, Exhausted (done).** Triggers at 25% from any cause, and a fixed seed replays identically. In LAB-012 the Hydro Pump user tires itself out and loses to the cheap-move side, so the design tension shows up.
- **Step 3, timeline (done, `4313e8ab86` and the regen fix).**
  - **How it works.** Each battler has a clock time for its next action. Each engine round lets only the earliest battler act; the others sit it out through upstream's existing "cannot act this turn" action. After acting, the battler's clock moves on by 1000 × weight ÷ Speed. Weight is 100 for status moves, switches and items; damaging moves add power ÷ 3.
  - **LAB-011.** Ninjask acted 3 times to Slaking's 2. Heavy moves visibly push the next action back: in LAB-012, Hydro Pump costs 1,700 clock units and Mud-Slap 1,325.
  - **Engine verdict so far: feasible.** It needs one hook in action selection and no rewrite of the battle loop.
- **Finding: end-of-turn effects tick every round, and a round is now one action.** Upstream has 56 end-of-turn events: Speed Boost, Leftovers, poison, weather, screens and so on. In LAB-011, Ninjask's Speed Boost fired on Slaking's actions too and doubled its Speed in two rounds.
  - Energy regen is already fixed: it now follows clock time, at +2% per 1,000 units.
  - The rest need re-timing: per own action, or per 1,000 units for field effects. That's engineering for F018. It's a known, bounded list, not a blocker.
- **Known spike shortcuts.** If a battler sits out the round after Hyper Beam, its recharge turn passes anyway; the clock delay stands in for it. Exact ties act together, in classic order.

- **Step 4, Boost (done, `f19cd882c6`).**
  - In the move menu, R cycles EN → SPD → POW → ALL.
  - The PP box shows the boost and the energy cost (e.g. "POW 46" for Hydro Pump), and the player's energy bar previews what is left after paying.
  - Speed costs +50% and makes the next action come sooner (clock delay ×0.6). Power costs +50% for ×1.3 power. Both costs +100%.
  - Log: "Swampert pays 46 for Hydro Pump +POWER".
  - Owner feedback (2026-11-26) was that the preview was too subtle and the bar still said "HP". The preview now **blinks**: every 8 frames the bar alternates between your current energy and what you'll have after paying, so the part you'd spend flashes. In stamina battles the bar label reads **"EN"**, using a Lab-only tile edited from upstream's "HP" tile (`graphics/battle_interface/lab_energy_label.png`).
  - Not yet built: the AI never boosts, and the "screen on fire" effect (F018 polish).

## Measurements (step 5)
Raw logs come from headless runs (gbashot with the game's log).

| Question | Measured | Verdict |
|---|---|---|
| Can the upstream loop run one-action rounds? | Yes. One hook in action selection reuses upstream's "cannot act" action. Lab tests 22/22; LAB-011 and LAB-012 play to the end under the timeline. End-of-turn effects need re-timing (above) | **Go**, with F018 re-timing work |
| AI decision time (hardware timer 3, 4,389 ticks = 1 frame) | 1v1 Swampert mirror: 0.71–0.99 frames per decision under both Classic and Stamina. Metagross Lv55: 1.06 frames. **6v6 LAB-009 under Classic: 4.5–8.1 frames**, because upstream's smart AI analyses switches across the whole party. The timeline adds no measurable AI cost | **Go for 1v1** (under 2 frames). For full parties the 2-frame target is already missed upstream, at about 0.13 s, not a visible stall. F018 should cache or spread the switch analysis across frames |
| More than 4 moves in battle RAM | Not prototyped in the spike. Code audit: `MAX_MON_MOVES` is used 272 times in 26 battle files (446 in all of src). A separate battle-only `MAX_BATTLE_MOVES = 8`, with saved Pokémon keeping 4, adds about 1.3 KB of battle RAM (AI damage, score and accuracy tables, battler move lists). AI scoring is linear in moves: about 2× per decision, so about 1.5 frames for 1v1 | **Go (estimate)**. A bounded refactor plus a new scrolling move menu, for F015/F018 |
| Battle length, Classic vs Stamina (same scripted input: always the first move) | LAB-011: Classic lost after 1 turn (Slaking one-shots Ninjask) vs Stamina lost after 5 rounds, with Ninjask acting 3 times. LAB-012: Classic **lost** after 10 turns (Hydro Pump ran out of PP) vs Stamina **won** after 7 rounds (the cheap-move side tired itself out). The LAB-009 (6v6) and LAB-013 (3v3) headless runs did not finish: scripted input can't reliably get through faint/switch menus | Partial. **Owner playtest** compares feel and length |
| ROM and EWRAM (spike code, dev builds only) | +2,188 B ROM and +53 B EWRAM for the stamina, timeline and Boost code (symbol sizes), plus about 0.3 KB for the AI timer. Release build: 0 (checked) | **Go** |

## Go/no-go recommendation (owner decides)
**Recommendation: go, and stay on the GBA engine.** Energy, Exhausted, the timeline and Boost all run on upstream's battle loop with small, Lab-only hooks. The costs found are bounded engineering for F018:
- re-time the end-of-turn effects
- cache the AI's switch analysis across frames
- a battle-only 8-move list plus a scrolling menu

None of them argues for another platform. The open question is **feel**, and that's the owner's playtest.

## Owner playtest (Battle Lab)
1. `git fetch && git checkout feature/F017-energy-spike && ./scripts/play.sh --lab`
2. Play **LAB-011** (Ninjask vs Slaking), **LAB-012** (Hydro Pump vs cheap moves) and **LAB-013** (tire out the boss). All three default to Stamina.
3. In the move menu, press **R** to cycle the Boost: EN → SPD → POW → ALL. Watch the cost in the PP box and the preview on your energy bar.
4. To compare, set **Rules → Combat: Classic** and play the same scenario again.
5. Optional: mGBA **Tools → View Logs** shows the clock ("LAB clock: … ACTS"), energy payments and AI timings.
6. Tell me: does Speed feel right? Is "wait for the big move" a real choice? Does tiring the boss work? Are battles dynamic rather than grindy? And go or no-go?

## Owner playtest results (2026-11-27)
The owner played LAB-011, LAB-012 and LAB-013.
- **Speed works.** Ninjask got 2–3 moves before Slaking acted once.
- **Battles are too short.** Damage is unchanged, so a trainer battle still ends quickly. Trainer battles should last longer. This needs a balance decision (below).
- **Bug, fixed in `a8f5792353`.** Highlighting a move after losing energy appeared to add energy back. Cause: the bar was never redrawn after a move's cost was paid or after regen, so it showed a stale value. It now redraws immediately.
- **Feedback for F018/F019 (visual clarity):**
  - Turn order isn't visible. For example, Metagross is faster, acted before Blissey, and one-shot it. A turn-order strip should show who acts next.
  - Regen isn't visible.
  - The cost number lacks context.
  - There's no Exhausted indicator, and the slowdown isn't noticeable.
  - Boost needs a stronger visual signal.

## Iteration 2 (owner direction, 2026-11-27)
Owner decisions: keep refining the prototype (no go/no-go yet). Make battles longer with option c (more energy, less damage, cheaper moves).

**Built on the branch:**
- **Opening round.** At the start of a battle every Pokémon picks a move. The round runs in normal priority and Speed order, so the player can e.g. Protect against a faster foe. A Pokémon replacing a fainted one may act straight away. A Pokémon switched in by choice still joins half a step after "now".
- **Exhausted is now Speed ×0.25** (was ×0.75). The energy bar's label turns from "EN" to a yellow **"EX"** badge. While a status icon is shown, upstream draws it over the label, so the badge never hides a status condition.
- **Option c as Lab Rules rows.** Energy ×1/×1.5/×2/×3 (both parties), Damage 100/75/50/33% (every damaging hit) and Cost 100/75/50/25%.
  - These are packed into the old combat byte, so the scenario format is unchanged; the tool accepts `"energy"`, `"damage"` and `"cost"` keys.
  - LAB-011/012/013 now default to ×2 energy, 75% damage and 50% cost.
- **Feedback effects:**
  - Regen flashes the energy bar green.
  - A boosted move flashes the attacker: Speed cyan, Power red, Both gold.
  - The move-menu label reads "Cost 31", then "Spd+ 46", "Pow+ 46" or "All+ 62" as R cycles.
- **Headless results:**
  - LAB-013: Blissey now survives the opening Hammer Arm (580/660).
  - LAB-012: lasted 28+ rounds, against 7 before.
  - LAB-011: still short. Slaking's Giga Impact takes Ninjask from 272 to 62 energy, Ninjask becomes Exhausted, and at ×0.25 Speed it barely acts again.

**Design findings for the owner:**
1. **Free healing allows endless stalling.** In LAB-013 a Soft-Boiled loop ran for 168 rounds, because healing moves are free under D023. Healing probably needs a cost, or diminishing returns like Protect.
2. **The Exhausted death spiral.** Damage can push you into Exhausted, and at ×0.25 Speed you then rarely act, so one big hit decides the fight. Options:
   - keep it, as the intended "wear them down" tactic
   - make Exhausted only come from spending your own energy
   - use a softer slowdown (×0.5)
3. The fading "+N" number on regen needs a small text-sprite helper. Moved to F018 together with the turn-order strip.

## Iteration 3 (D024, 2026-11-27)
- **Messages.** The "used" message explains the action. For example:
  - "Ninjask used X-Scissor with boosted Power!"
  - "Ninjask is faster and acts again: X-Scissor!" (when it acted in the last round as well and the foe acted in neither)
  - "The exhausted Ninjask slowly used X-Scissor..."
- **Exhausted visuals.** A white badge with a red "EX" replaces the bar label. The Pokémon's sprite has a dull purple tint that slowly pulses ("panting") for as long as it is Exhausted.
- **Dual bars (Rules → Combat: Dual bars).**
  - Health and energy are separate. Damage takes health (and the Damage rule applies). Moves spend energy, which regenerates.
  - Energy capacity is 50% plus 50% × the health share.
  - Healing costs 20% of max energy.
  - A move that costs more energy than is left is locked; with nothing affordable, the Pokémon uses Struggle.
  - The Energy rule scales max energy only.
  - **Spike UI:** the player's EXP bar shows energy (it is still labelled EXP), and the health bar's label becomes EX when Exhausted. Opponents have no energy bar: their state shows through the EX badge, the tint and the battle log ("EN x/y"). A full two-bar healthbox for both sides is F018 work.
  - New scenarios: LAB-014 (Dual: tire the boss) and LAB-015 (Dual: fast vs slow), at ×1 energy, 50% damage and 100% cost.
  - Headless LAB-014: Blissey's Soft-Boiled costs 66 energy, so healing drains her (330 → 108 over three heals). Metagross's hits cap her energy ("capped by health 264 → 232").
  - Headless LAB-015: still a one-hit KO (Giga Impact on Ninjask). Tune Damage lower in the Lab.

## Iteration 4 (D025, 2026-11-27)
- **Dual bars is now the default** in LAB-011/012/013 (×1 energy, 50% damage, 100% cost). Stamina mode is still in Rules for comparison. LAB-014/015 were removed as duplicates.
- **End-of-round messages:**
  - "X is exhausted! It slows right down..."
  - "X got its second wind!"
  - "X's stamina was sapped by the blow!" (health capped its energy)
- **Regen:**
  - Benched Pokémon recover +8% energy per 1,000 clock units, up to their health cap.
  - Active regen in Dual is +3% per 1,000.
  - The whole-HUD green regen flash is removed.
- **Exhausted badge:** dark label background with a white "EX", blinking with the normal label every half second.
- **Headless runs (mashing the first move):** LAB-011 now lasts 7 turns (was 1–3). The energy cap triggered in all three scenarios, and the new messages show on screen.

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
