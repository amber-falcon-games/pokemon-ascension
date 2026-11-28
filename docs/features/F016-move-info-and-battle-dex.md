# F016 — Quick move info and in-battle Pokédex
Status: BUILT; awaiting the combined owner playtest (D026)
Dependencies: F015 (talent trees)
Owner decision IDs: D005 (optional move info, known-species Pokédex), D015, D026 (build the whole overhaul, then a combined playtest)
Branch / commit: `feature/F016-move-info` (stacked on `feature/F015-move-library`)

## Objective and scope
Give the player the detail they need to choose well, on demand, without crowding the Emerald battle screen (BATTLE_SYSTEM.md).

1. **Move info (L in the move menu).**
   - Upstream's info box stays as the first page: category, power, accuracy and description.
   - New: pressing L again shows a **talent page** for the highlighted move, then L closes the box. The page shows:
     - its branch and what it grows into next, e.g. "Grows into Hydro Pump at Lv 39"
     - at a branch point, the options, e.g. "Lv 39: Hydro Pump or Muddy Water"
     - on a joined global path, the next move on that path
     - "Fully grown" at the end of a branch
   - With energy rules, the cost box beside the menu already shows the energy cost (F017). No change there.
2. **In-battle Pokédex (L in the action menu: Battle / Bag / Pokémon / Run).**
   - L, not R: upstream uses R there to throw the last-used ball. L matches "L = info" in the move menu.
   - Opens a panel about the opposing Pokémon in the description box's place.
     - L or A turns the page, and the last page closes it.
     - In doubles, LEFT/RIGHT switches opponent.
     - B closes it.
     - It never uses a turn.
   - What it shows follows the Pokédex (proposed defaults; the owner may change them at the playtest):
     - **Always:** name, level and types (the field Pokédex shows types once a Pokémon is seen, and meeting it in battle marks it seen), plus "Moves seen": the moves this party member has used in this battle, four to a page.
     - **Caught** (Pokédex "owned" flag): also the types it is weak to (x4 marked), its possible abilities and its base stats.
     - **Not caught:** "Catch one to learn more."
   - Wild and trainer battles behave the same. Lab battles count every species as caught. It is off in Safari, link and recorded battles, and in the Battle Arena and Kanto-tutorial window layouts.

## Explicit exclusions
- Changing the field Pokédex screens.
- Info on the player's own Pokémon, which is the summary screen's job.
- Damage forecasts and hidden stats such as IVs/EVs: BATTLE_SYSTEM.md says not to flood the screen.
- Timeline preview: F018.

## Acceptance criteria
- [ ] L in the move menu cycles info → talent page → closed. The talent page is correct for plain growth, branch points, jumps, joined paths and fully grown branches.
- [ ] R in the action menu opens the opponent panel. What it shows follows the known/seen/caught rules. Doubles can switch target. B/R close it and the battle continues unchanged.
- [ ] Moves an opponent used appear in its panel for the rest of the battle.
- [ ] Upstream behaviour is unchanged with `ASCENSION_MOVE_LIST` off (release): there is no talent page and no R panel.
- [ ] The full suite is green, and a Lab scenario shows both features.

## Technical approach and risks
- **Talent page:** a new `Ascension_DescribeTalentMove(mon, move, buf)` in `ascension_moves.c` finds the move's source in the battle list (branch or join) and the next node. It is printed in `B_WIN_MOVE_DESCRIPTION` through upstream's description path (`gBattleStruct->descriptionSubmenu` gains a second page).
- **Pokédex panel:**
  - Drawn in the action-menu windows: the message window for name, types and status, and the action window for the details. Restored on close.
  - "Seen before this battle" is recorded at battle start, because upstream marks the opponent seen as the battle begins.
  - Moves used are recorded per opposing battler slot from `gLastMoves`/the move-end hook, a bitmask over its battle list.
- **Risks:**
  - Window space on the GBA screen: the panel must stay readable in FONT_NARROW.
  - R in the action menu must not clash with upstream bindings. Upstream leaves R free there; Boost (R) is in the move menu only.

## Automated tests and regression
- C:
  - the talent description for growth, branch point, jump, join and fully grown
  - the known-level rules (never seen / seen / caught)
  - a used move is recorded for the right opponent
- Battle tests: opening and closing the panel does not use a turn, and the AI and RNG are unaffected.
- The full upstream suite. Headless screenshots in the Lab.

## Manual playtest
Combined with the overhaul playtest (D026). `./scripts/play.sh --scenario LAB-014`, then:
- In the move menu, press L twice to see the talent page.
- In the action menu, press R to see Gardevoir's panel (Lab: everything shown).
- In the campaign, check a species never caught: only types, or "???" if never seen.

## Resource budgets and save compatibility
- No save change: it uses existing Pokédex flags.
- About 30 bytes of battle EWRAM (seen-before flags, used-move masks). A little ROM.

## Rollback
Turn `ASCENSION_MOVE_LIST` off, or revert the branch.

## Evidence and acceptance
- Built on `feature/F016-move-info`.
- Tests:
  - `test/ascension/move_list.c`: talent page text (next growth, fully grown after a jump, TM path, not in the list)
  - `test/ascension/battle_dex.c`: uncaught vs caught pages; moves seen recorded per opposing party member, once each, only for opponents, reset per battle
- Headless in the Lab ROM (LAB-014):
  - L, then L on Rock Slide: the info page, then "Talent branch 3. Fully grown.", then L closes.
  - Action menu L, page by page:
    - "Gardevoir Lv. 50 / Psychic / Fairy / Weak: Poison, Ghost, Steel"
    - "Abilities: Synchronize, Trace, Telepathy"
    - "Base stats HP 68 Atk 65 Def 65 / SpA 125 SpD 115 Spe 80"
    - "Moves seen: none yet"
    - The last L closes it and restores the screen.
  - After a turn: "Moves seen: Calm Mind, Moonblast".
- Implementation notes:
  - The panel is window `B_WIN_ASC_DEX`, which shares the description box's tiles (never shown together).
  - The bg0 tilemap under it is saved and restored, because tile 0 there is not blank.
  - Moves seen are kept separately from the AI's record, which already holds the AI's own full movesets.
