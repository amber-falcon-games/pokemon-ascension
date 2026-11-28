# Changelog

`scripts/release.sh` uses the first `##` section below as the release notes, so the next release's section stays at the top.

## Unreleased — combat overhaul (in progress)
**For players:** nothing yet. The overhaul is built in dev builds only (`ASCENSION_MOVE_LIST` and the stamina rules are off in release) until the owner's combined playtest (D026).

**For development (dev builds):**
- **Energy and dual bars** (F017, D023–D025): separate health and energy bars, energy costs, Exhausted, Boost, timeline turn order and bench regeneration, tried in the Battle Lab.
- **Up to 8 moves in battle** (F014): a grouped move menu (attacks, then support), four moves to a page.
- **Talent trees** (F015, D027): every Pokémon's battle moves come from a generated, editable tree (`data/ascension/talent_trees.json`).
  - Level-up growth: "Mudkip's Water Gun grew into Water Pulse!"
  - Branch points ask which move to grow into.
  - TMs jump a branch ahead, are refused once a Pokémon has grown past the move, or start a new talent path on a global chain. With a full list, the player picks the move it replaces.
  - Evolving keeps every branch, choice and TM jump.
- **Move info and battle Pokédex** (F016): L twice in the move menu shows the talent page. L in the action menu opens a panel on the opponent: types and moves seen, plus weaknesses, abilities and stats once caught.
- **Initiative timeline** (F018): end-of-turn effects tick once per beat, a recharge waits for the battler's own action, and a "Next:" strip shows the next four actors. Dual-bars singles draw energy inside the healthbox like HP.
- **Energy in the campaign** (F019): with `ASCENSION_COMBAT` (dev builds), energy replaces PP in campaign battles. It is kept between battles, recovers while walking and fully at Pokémon Centers, and Ethers and Elixirs restore it.
- **Queue a follow-up** (F020, D028): SELECT in the move menu queues the next action. It cancels itself if the target leaves, your Pokémon is Exhausted, takes a big hit or gets a status; B clears it.
- **Opponents use Boost** (F021): the AI boosts for a knock-out or to act before its target, and keeps an energy reserve.

## 0.2.0 — 2026-11-26 — Battle Lab foundation
**For players:** no gameplay changes. The 0.2.0 patch plays exactly like 0.1.0. Everything below is in dev builds only: release builds contain none of the Lab or checkpoint code, and its few small hooks into the battle engine compile to nothing (checked by the nightly CI build).

**For development (dev builds):**
- **Battle Lab** (F007–F012): a sandbox for real battles, opened by `./scripts/play.sh --lab`.
  - Both teams can be edited: up to 6 Pokémon, with species and form, level, moves, ability, item, nature, IVs, starting HP and status.
  - Rules: trainer 1v1, trainer 2v2 or wild; weather, terrain, opponent AI and RNG seed.
  - In battle, SELECT → Lab: Restart replays the same setup and Lab: Quit returns to the Lab with edits kept.
  - The last result and seed are shown, and any battle can be replayed exactly.
  - A turn-by-turn battle log appears in mGBA (Tools → View Logs).
  - Scenarios are JSON files in `lab/scenarios/` (LAB-009: 6v6 trainer battle; LAB-010: over-levelled wild Pokémon), injected into a separate Lab ROM with its own save (D022). Your campaign save is never touched.
- **Dev checkpoints** (F007b): R+Start → Checkpoints… jumps to 10 story points: start, Poké Balls, and outside each of the 8 gyms, each with a ready team, items, money and the story state of a normal run.
- Fixes in Lab battles: Pokémon always obey, a seed replays exactly regardless of input timing, and wild opponents keep the item they were given.

## 0.1.0 — 2026-11-26 — Foundation
- First release: RHH pokeemerald-expansion `expansion/1.17.2` (commit 79dd4369) with full history (D010), published as a `.bps` patch for your own Pokémon Emerald (USA/Europe) ROM.
- Game: link-cable, e-Reader and Mystery Event save data removed; redundant Pokédex flags removed; PC reduced to 10 boxes. This frees 12.4 KB of save space and working memory.
- Scripts for Apple Silicon Macs: `setup.sh`, `doctor.sh`, `build.sh`, `test.sh`, `play.sh`, `status.sh` and the guided `release.sh` (no sudo; pinned ARM toolchain in `.local/`).
- CI: pull requests only, selective (docs only, changed tests or full), tests split across 4 machines, incremental build cache, and nightly build and test badges.

## Before 0.1.0
- F000 scaffold review and F001 upstream audit; documentation bootstrap (vision, operating instructions, roadmap, battle design, Battle Lab specification, testing and workflow).
