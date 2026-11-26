# F007b — Dev checkpoints: jump to set points in the story
Status: IN_PROGRESS (part 1: CP-START, CP-BALLS, CP-GYM1 done; CP-GYM2–8 next)
Dependencies: F007 (shares the debug-menu entry work); owner decision D019
Owner decision IDs: D019
Branch / commit: `feature/F007b-dev-checkpoints`

## Objective and scope
In dev builds, load a named point in the story from the debug menu (R + Start → Checkpoints…), instead of replaying the game or relying on emulator save states. Each checkpoint is data in the repository, so it survives new builds and is shared by everyone.

A checkpoint sets: location (warp), story flags and vars on the main path up to that point, badges, party (species, level, moves, held items, ability), bag items, Poké Balls, money and Pokédex state. Applying one replaces the current in-memory game state; saving afterwards is the player's choice, as usual.

### Checkpoints (owner's list)
| ID | Point in the story | Location | Badges |
|---|---|---|---|
| CP-START | Intro done (truck, Mom, clock set, rival's house visited), before choosing a starter | Littleroot Town north exit (walk north into the story) | 0 |
| CP-BALLS | Starter chosen, rival battle on Route 103 done, Pokédex and Poké Balls received: catching unlocked | Outside Birch's lab, Littleroot | 0 |
| CP-GYM1 | Before Roxanne | Rustboro City, outside the Gym | 0 |
| CP-GYM2 | Before Brawly | Dewford Town, outside the Gym | 1 |
| CP-GYM3 | Before Wattson | Mauville City, outside the Gym | 2 |
| CP-GYM4 | Before Flannery | Lavaridge Town, outside the Gym | 3 |
| CP-GYM5 | Before Norman | Petalburg City, outside the Gym | 4 |
| CP-GYM6 | Before Winona | Fortree City, outside the Gym | 5 |
| CP-GYM7 | Before Tate & Liza | Mossdeep City, outside the Gym | 6 |
| CP-GYM8 | Before Juan | Sootopolis City, outside the Gym | 7 |

The owner can toggle a gym as beaten with the existing Flags & Vars menu.

### Teams (proposal; values live in a data file the owner can edit)
Leader aces in this build: Roxanne Nosepass 15, Brawly Makuhita 19, Wattson Manectric 24, Flannery Torkoal 29, Norman Slaking 31, Winona Altaria 33, Tate & Liza Lunatone/Solrock 42, Juan Kingdra 46.
- **Team size:** grows 1 → 6 (CP-START has no Pokémon; CP-BALLS has the starter only; CP-GYM1 has 2; then one more per gym, up to 6).
- **Levels:** the team's ace is at the leader's ace level; others 1–3 below. This is "ready but not over-levelled", in line with the anti-grind vision.
- **Species:** only Pokémon catchable in Hoenn before that point, from one consistent run built around the Mudkip line by default (Torchic and Treecko variants can follow). At least one answer to each leader's type, with no free sweep.
- **Moves:** level-up moves at that level, plus TMs/HMs obtainable by then (e.g. Rock Tomb after Roxanne, Cut, Flash, Surf from Norman).
- **Items:** Potions/Super Potions and status heals matching the shops reachable by then; key items from the story (e.g. Acro/Mach Bike by CP-GYM4, Surf HM by CP-GYM6); money roughly what a normal run has (gym prize money plus trainer winnings).

## Explicit exclusions
No release-build access (debug menu only). No mid-route checkpoints beyond the list (easy to add later as data). No changes to campaign scripts.

## Acceptance criteria
- [ ] R + Start → Checkpoints… lists the 10 checkpoints; choosing one fades, warps and sets everything above.
- [ ] Automated test per checkpoint: applying it produces the documented badges, party levels, key items and the main-path flags.
- [ ] From each checkpoint, the next story step works normally: rescue Birch (CP-START), catch a Pokémon (CP-BALLS), enter and battle the gym (GYM1–8), with no blocked path or doubled event.
- [ ] Owner playtest: each checkpoint loads where expected and feels right for that point; teams adjusted to taste.
- [ ] Release build contains no checkpoint code (`nm` check).

## Technical approach and risks
- **Data:** `src/data/ascension/checkpoints.h` (or a text format converted at build time, like upstream's `.party` files). Teams reuse upstream's party description fields.
- **Flags and vars:** derived by tracing the main-path map scripts (`data/maps/*/scripts.inc`) up to each point.
- **Risk:** a missed story flag leaves the world inconsistent, such as an NPC still blocking a route or an event replaying. Mitigation: trace from the scripts (not memory), test the next story step from each checkpoint, and let the owner report anything odd. A missed flag is a data fix, not a code change.
- **Risk:** reachable items and Pokémon per point need care. Mitigation: build team lists from the game's own encounter tables and mart data.

## Manual playtest (owner)
`./scripts/play.sh` → New Game (SELECT on the title screen) → R + Start → Checkpoints… → pick one → check location, party, bag and badges → continue the story a little.

## Resource budgets and save compatibility
Debug-only data, a few KB of ROM in dev builds. No save layout change. Applying a checkpoint then saving writes an ordinary save.

## Rollback
Revert the PR.

## Evidence and acceptance
Part 1 (2026-11-26, Linux container): `make -j4` exit 0, dev ROM 26,746,968 B. Implemented as event scripts (`data/scripts/ascension_checkpoints.inc`) called from a new debug-menu submenu, with flags/vars traced from the map scripts. Checked headlessly with gbashot (scripted input, screenshots):
- CP-START: lands at Littleroot's north exit; walking north triggers the neighbour's "Um, hi! … I don't have any Pokémon" scene that leads to Birch's rescue.
- CP-BALLS: outside Birch's lab; start menu shows POKéDEX and POKéMON; party is Mudkip Lv7; bag has 5 Poké Balls.
- CP-GYM1: outside Rustboro Gym; party Mudkip Lv15, Taillow Lv13; walking north enters the gym. Story state set: Oldale rival gone, Wally's tutorial done (Petalburg city/gym vars), Petalburg Woods Aqua grunt beaten and Devon researcher gone, Great Ball, 3 Potions, Antidote, +₽2,400. Roxanne fight itself not yet driven headlessly.
Not yet: automated per-checkpoint tests (with GYM checkpoints), owner playtest.
