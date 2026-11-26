# Changelog

`scripts/release.sh` uses the first `##` section below as the release notes, so the next release's section stays at the top.

## 0.2.0 (unreleased) — Battle Lab foundation
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
