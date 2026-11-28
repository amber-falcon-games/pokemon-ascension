# F015 — Move library: talent trees, learning and saving
Status: BUILT except the Lab integration (item 7); awaiting the combined owner playtest (D026)
Dependencies: F014 (8-move battle list)
Owner decision IDs: D015, D027 (2026-11-27)
Branch / commit: `feature/F015-move-library` (stacked on `feature/F014-move-menu`)

## Objective and scope
Every Pokémon's battle moves come from a **talent tree** for its evolution line. This replaces F014's temporary rule ("recent level-up moves").

1. **Tree model.** A line's tree is a set of **branches**, at most 8 (the battle list holds 8 moves). A branch is an ordered chain of moves, each unlocked at a level; a later move replaces the earlier one (e.g. Ember 7 → Flamethrower 34 → Fire Blast 50). Where a line's moves fork, a **branch point** offers a choice of 2–4 moves, and the player picks one when it unlocks.
2. **Generated, then editable (D027-1).**
   - `tools/ascension/talent_trees.py` generates `data/ascension/talent_trees.json` from upstream level-up learnsets. Same type and category, rising power, forms a chain. Moves learned within a few levels of each other in different chains become a branch point. Each line is capped at 8 branches.
   - The owner edits the JSON (any line, any move or level). The build compiles it into ROM tables, so edits never touch C.
   - A checker validates the file: unknown moves, more than 8 branches, levels out of order.
   - **Evolving keeps slots.** An evolution's tree starts with its pre-evolution's branches in the same order, and new branches are appended (at most 2 status branches in all). The checker enforces this for hand edits, because choices, TM jumps and joins are stored per slot. On evolving, the game carries each TM jump and choice over by move, since a slot's nodes can differ between stages.
3. **Global branches for taught moves (D027-2).** One generic chain per type and category covers every TM, tutor and egg move, e.g. Electric special: Thunder Shock → Thunderbolt → Thunder.
4. **Teaching a move (TM, tutor, egg):**
   - In the Pokémon's own tree, not yet reached: its branch jumps forward to that move.
   - In its own tree, already passed: the TM is refused ("already grown past it").
   - Not in its own tree: the Pokémon joins the global branch containing the move, at that move, and grows along it with level from there. If its 8 branches are full, the player replaces one.
   - Party menu messages:
     - "X's talent jumped ahead to Y!"
     - "X started a new talent path with Y!"
     - "X has already grown past Y."
     - "X can't follow any more new talent paths." (two joined chains at most)
     - With a full list: "X's moves are full. Pick a move for Y to replace.", then an 8-move picker. B cancels and keeps the TM.
5. **Battle list.** In battle, each branch contributes its current move, giving the F014 8-move list. The menu grouping (attacks, then support) is unchanged.
6. **Learning on level-up.** Reaching a move's level replaces the branch's move with a short message ("Charmander's Ember grew into Flamethrower!"). A branch point asks the player to choose.
   - This works in battle (experience) and with Rare Candy. A branch point says "X's talent can grow in more than one way!", then asks "Grow into A?" for each option in turn. NO moves on to the next option, wrapping round.
   - Upstream's "wants to learn / forget a move" flow is off for Pokémon with a tree. The daycare and evolution relearning refresh the moves silently.
   - Recorded battles (and the test runner's wild battles) cannot take input: a branch point keeps the saved choice there.
7. **Lab.** The slot editor shows the battle list from the tree. Scenario files can name branch choices.

## Save model
- **Stored per Pokémon:**
  - its branch choices: 2 bits × up to 8 branch points = 16 bits
  - the global branches it joined: per joined branch, a branch id (about 7 bits) plus the node it joined at (3 bits)
- **Worked out from data, not stored:** the move each branch is on (from line and level), and PP. Energy replaces PP in the overhaul.
- **Space:** upstream leaves 22 spare bits per Pokémon (`include/pokemon.h`), enough for the choices but not for joined branches.
  - Per D027-3, the space is reclaimed now that it is needed: the **contest condition stats, sheen and contest ribbons** come out of the Pokémon structure (a subset of F005c, about 70+ bits). That is enough for 4–6 joined branches per Pokémon.
  - Contests are disabled with it, as already listed for cutting in D011. The rest of F005c (Secret Bases, TV, Mail and so on) still waits.
- **Migration:** no saves have been released since 0.1.0 with a different layout. Dev saves start a new game (SAVE_COMPATIBILITY.md is updated, and `test/save.c` sizes change with the documented layout).

## Storage design (worked out 2026-11-27)
| Data | Bits | Where |
|---|---|---|
| Branch choices: 2 bits × 8 branches | 16 | Upstream spare bits (`talentChoicesA–D`, `MON_DATA_TALENT_CHOICES`). **Done**, no layout change. |
| TM "jump ahead" per branch: node index reached early, 3 bits × 8 | 24 | Contest data (below) |
| Joined global branches: chain id 6 + start position 6 + branch slot replaced 3, × up to 3 | 45 | Contest data (below) |

- **Contest data reused:** the condition stats (Cool, Beauty, Cute, Smart, Tough; 40 bits), sheen (8 bits) and the contest ribbon tiers (15 bits), 63 bits in all.
  - The fields stay where they are, so save size and layout are unchanged.
  - Contests and Pokéblock feeding must no longer write them. Those entry points are switched off in this step (Contest Hall entry, Pokéblock feeder), and the rest of the cut waits for F005c.
- **Global-chain growth without storing a level:**
  - The generator records each move's *typical level*: the median level at which species learn it naturally.
  - A Pokémon on a joined chain has the furthest chain move whose typical level it has reached, never earlier than the move it joined at.

## Explicit exclusions
- Hand-tuning every line. The owner does that over time, and the generator only provides a first pass.
- Move info and the in-battle Pokédex: F016. Energy balance: F019.

## Acceptance criteria
- [ ] The generator produces a tree for every evolution line, within the limits. The checker passes on it, and the owner can change a line in JSON and rebuild.
- [ ] Battles use tree-derived moves (up to 8). Level-up growth and branch choices work in the campaign and the Lab.
- [ ] TMs follow D027-2 in all three cases: jump, refuse, join a global branch. Tested.
- [ ] Saved layout documented and `test/save.c` updated. A party survives save and load with its choices and joined branches.
- [ ] Release build unchanged until the overhaul ships (behind `ASCENSION_MOVE_LIST`).

## Technical approach and risks
- Biggest risk: the quality of generated trees. Mitigations: simple rules, an owner-editable file, and a report listing lines that need attention.
- Upstream code reads `MON_DATA_MOVE1..4` widely (menus, contests, AI party data). The saved 4 move slots stay as a cache of the first 4 battle moves, so summary screens and upstream code keep working while the tree is the source of truth.

## Test plan
- Python: the generator is deterministic, the checker catches errors, and the JSON round-trips.
- C:
  - tree lookup by line and level
  - level-up replaces the move
  - branch point choice
  - all three TM cases
  - save and load of the new fields
  - battle list built from the tree
- Full upstream suite green. Lab checks of trees for starters.

## Evidence (2026-11-28)
- Core (fda7a5ad20): tree lookup, choices, the three TM cases, joined chains and the battle list.
- UI and growth: this commit. Tests in `test/ascension/move_list.c`, 18 in all:
  - teaching asks for a slot when the list is full and replaces only the picked move
  - the saved 4 moves follow the tree
  - growth reports from and to only at its level
  - a TM jump suppresses growth into nodes it passed
  - a branch point's pick is what the branch grows into
  - evolving carries a jump over
  - talent data survives boxing
- `test/ascension/talent_growth_battle.c`: in a real battle, levelling up shows "Pikachu's Thunder Shock grew into Electro Ball!" and updates the saved moves.
- Headless checks in the real game (Gym 8 and Poké Balls checkpoints; Rare Candy was added only in a local, uncommitted build):
  - Rock Tomb on Swampert (8 branches): the full-list message, the 8-move picker, then "Swampert started a new talent path with Rock Tomb!"
  - Rare Candy on Mudkip, level 7 to 12:
    - "Mudkip learned Rock Throw!" (a new branch)
    - "Mudkip's Water Gun grew into Water Pulse!" (a Marshtomp run)
    - At 12: "Mudkip's talent can grow in more than one way!", then "Grow into Protect?" (yes/no), then "Mudkip's Growl grew into Protect!"
- Not yet seen on screen: the in-battle branch-point prompt. It uses upstream's yes/no box, and the battle growth path is covered by the test above.
- Still to do:
  - **Lab (item 7):** Lab battles keep the scenario's hand-set moves.
  - **Save migration** for old saves with contest data (SAVE_COMPATIBILITY.md).
- Generator: evolution slot order (1,333 species, checker 0 errors). Python tool tests 16/16.

## Rollback
Turn `ASCENSION_MOVE_LIST` off: the battle list falls back to the 4 saved moves. Revert the branch for save-layout changes.
