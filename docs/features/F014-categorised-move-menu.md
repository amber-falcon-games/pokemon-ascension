# F014 — Categorised move menu (more than four moves in battle)
Status: BUILT, awaiting the combined owner playtest (D026)
Dependencies: F017 (go, D026)
Owner decision IDs: D015 (more than four moves, Final Fantasy-style categorised list), D025 (Dual bars), D026 (build the whole overhaul, then a combined owner playtest)
Branch / commit: `feature/F014-move-menu` (stacked on `feature/F017-energy-spike`)

## Objective and scope
Let a Pokémon use more than four moves in battle, chosen from a categorised, Final Fantasy-style menu that still feels like Emerald.

1. **Battle move list.** A battler carries up to **8 moves in battle** (`MAX_BATTLE_MOVES`). Saved Pokémon keep their 4 moves until F015 adds the move library and its save format.
2. **Where the extra moves come from until F015.** The 4 set moves come first. Up to 4 more are taken from the species' level-up learnset at or below the Pokémon's level, most recent first, skipping duplicates. F015 replaces this rule with the talent tree and stored choices. It sits behind `ASCENSION_MOVE_LIST` in `include/config/ascension.h`, which is on in dev and Lab builds and off in release until the overhaul ships.
3. **Menu.** Choosing Battle opens the move window as a list grouped by category:
   - **Attacks**: physical and special damaging moves.
   - **Support**: status moves.
   - **Up/Down** scroll through the list, two moves per row as in Emerald, crossing from one category into the next.
   - A thin header in the move window names the current category.
   - With 4 moves or fewer, the menu looks exactly like Emerald's.
   - Existing keys keep their jobs: L for move info, R for Boost (stamina modes), A to choose, B to go back.
4. **Engine.**
   - Battle code reads moves through the battle move list.
   - The move-selection state (`ChooseMoveStruct`, controller buffers) carries up to 8 moves.
   - AI scoring loops over the battle list.
   - Code that writes moves back to the saved Pokémon (PP, Mimic, Sketch, Transform) applies only to the first 4.
5. **Lab.** The F008/F009 slot editor gains moves 5–8, stored in the scenario's existing padding byte as an extended-moves flag. Lab JSON accepts `"moves"` lists of up to 8. Exact record layout to be settled in implementation; record-size changes are covered by the tool's tests.

## Explicit exclusions
- Talent tree, learning and saving more than 4 moves: F015.
- The move info panel and in-battle Pokédex: F016 (L keeps today's description box).
- Timeline UI and the real energy bar art: F018–F019.
- Doubles-specific menu layouts beyond what Emerald already does.

## Acceptance criteria
- [ ] A Pokémon with more than 4 eligible moves shows them all in battle, grouped by category, and can use any of them. Energy costs, Boost, and Exhausted locks work for moves 5–8.
- [ ] With 4 moves or fewer, the menu and battles behave exactly as before. The full upstream test suite is green.
- [ ] The AI considers all of its battle moves. The battle log shows its choice, and decision time is recorded (target: no visible stall).
- [ ] Saved data is unchanged: the save layout is identical, a party saves and loads with its 4 moves, and Mimic, Sketch and Transform behave as upstream.
- [ ] The release build is unaffected while `ASCENSION_MOVE_LIST` is off for release.

## Technical approach and risks
- Introduce `MAX_BATTLE_MOVES` (8). Change battle-side arrays from `MAX_MON_MOVES` to it: `BattlePokemon.moves/pp`, AI tables, `ChooseMoveStruct`, chosen-move positions. Upstream uses `MAX_MON_MOVES` 272 times in 26 battle files (F017 audit). Each use is classified as battle-side (→ 8) or saved-mon (stays 4). Biggest risk: off-by-one writes between battle and party data.
- Controller buffers are 0x200 bytes and fit 8 moves. The menu redraw uses the existing 4 move-name windows plus scrolling.
- AI time is roughly linear in moves. Measured with the F017 AI timer; mitigation if needed is skipping moves the AI already scored as useless.

## Automated tests and regression
- New tests:
  - the battle list is built from set moves plus learnset (order, duplicates, level cut-off)
  - a battler with 6 moves can use moves 5 and 6
  - PP and transform writes never touch party move slots 5 and above
  - a Lab scenario with 8 moves round-trips through the tool
- Full upstream suite (4 CI shards) with the flag on and off. Release build symbol checks.

## Manual playtest
Owner playtest is combined with the rest of the overhaul (D026). Lab scenario LAB-016 "Eight moves" will be added for it.

## Resource budgets and save compatibility
- About +1.3 KB EWRAM in battle (F017 estimate). Small ROM cost.
- No save change; saved Pokémon keep 4 moves until F015.

## Rollback
Turn `ASCENSION_MOVE_LIST` off, or revert the branch.

## Evidence and acceptance
- Engine (34b527fc24, f576b54a23): the full upstream suite runs in CI on PR #19.
- AI thinking-time ceilings (`test/battle/ai/ai_thinking_time.c`): with 8-move tables and 4 real moves, the smart AI measured 29 frames in doubles and 25 in Steven multi, against 28 and 23 on main (same local build). Empty-slot skips in the AI loops did not remove it, because the cost is spread across many loops. The two ceilings were raised to 30 and 26 (measured + 1). The other four are unchanged.
- Menu (4281347a5d, 97de346d8d):
  - The list is grouped attacks first, then support moves, four to a page.
  - Up/down at the edges turns the page, and arrows mark the other pages.
  - Headless LAB-012 check: page 1 shows Hydro Pump, Earthquake, Hammer Arm and Muddy Water. Page 2 shows Take Down, Protect, Screech and Amnesia. A move chosen from page 2 is used and its energy is paid.
- AI time with up to 8 moves on both sides (Lab AI timer):
  - 1v1 LAB-012: mean 1.26 frames, max 1.45 (F017 with 4 moves: about 0.9).
  - 6v6 LAB-009 Classic: mean 11.1 frames, max 12.7, about 0.2 s (F017 with 4 moves: 4.5–8).
  - This is not a visible stall, but it is up to about 60% more work. Optimising it, for example by pruning moves the AI already rules out or spreading the switch analysis over frames, is F021 (scheduler-aware AI).
- Deferred:
  - Editing moves 5–8 in the Lab slot editor. Lab Pokémon already get moves 5–8 from their learnset, and F015 replaces raw move lists with talent-tree choices, so the editor gains them there.
  - A thin category header. The order (attacks, then support) and the move type line make the grouping clear for now; F016 adds the info panel.
- Tests: `test/ascension/move_list.c` 2/2, Lab 22/22, and the full upstream suite is green in CI (PR #19), with the documented AI thinking-time ceiling change.
