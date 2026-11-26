# Changelog

## Unreleased — F012 Battle Lab inspection and diagnostics (2026-11-26)
- Dev builds: every Lab battle has a known seed (Rules shows the last one; A on Seed no. replays it); a turn-by-turn battle log goes to mGBA's log window. Fix: Lab Pokémon always obey (the Lab has no badges).

## Unreleased — F011 Battle Lab scenario files (2026-11-26)
- Dev builds: Battle Lab scenarios are JSON files in `lab/scenarios/`; `./scripts/play.sh --lab` (or `--scenario ID`) builds a separate Lab ROM that opens straight into the Lab with them. Presets LAB-009 (6v6 trainer) and LAB-010 (over-levelled wild). No save memory used (D022).

## Unreleased — F010 Battle Lab battle loop (2026-11-26)
- Dev builds: the in-battle debug menu (SELECT) gains Lab: Restart (replay the same setup at once) and Lab: Quit (back to the Lab with edits kept); the Lab title shows how the last battle ended.

## Unreleased — F009 Battle Lab details and rules (2026-11-26)
- Dev builds: each Lab Pokémon can have its moves, ability, held item, nature, IVs, starting HP % and status set; a Rules screen sets the format (trainer 1v1, trainer 2v2, wild), weather, terrain, opponent AI and a fixed RNG seed.

## Unreleased — F008 Battle Lab team builders (2026-11-26)
- Dev builds: the Battle Lab's Player team and Opponent team screens set up to 6 Pokémon per side (any species or form, level 1–100, in battle on/off). Changed slots are marked `!`; LAB-001 is the starting point and START resets it.

## Unreleased — F007b dev checkpoints (2026-11-26)
- Dev builds: R+Start → Checkpoints… jumps to 10 story points (Start, Poké Balls, and outside each of the 8 gyms) with a ready team, items, money and the story state of a normal run. Release builds contain none of it.

## Unreleased — F007 Battle Lab entry (2026-11-26)
- Dev builds: L + SELECT on the title screen opens the Battle Lab; LAB-001 (Swampert vs Blaziken, Lv50) runs a real battle and returns to the Lab; Exit soft-resets without saving. Release builds contain no Lab code.

## Unreleased — F004a, F002–F005, F005b (2026-11-26)
- CI: PR-only, selective (docs-only / changed tests / full), tests split across 4 machines, incremental build cache, nightly build and test badges.
- Scripts: `setup.sh`, `doctor.sh`, `build.sh`, `test.sh`, `play.sh`, `status.sh` for Apple Silicon Macs (no sudo; pinned ARM toolchain in `.local/`).
- Game: link-cable, e-Reader and Mystery Event save data removed; redundant Pokédex flags removed; PC reduced to 10 boxes. Frees 12.4 KB of save space and working memory. Save layout changed (start a new game on dev builds).

## Unreleased — F001 upstream import (2026-11-26)
- Merged RHH pokeemerald-expansion `expansion/1.17.2` (commit 79dd4369) with full history (decision D010, option A).
- Kept project README, PR template and docs; combined .gitignore; removed upstream Pages, label and Discord-link GitHub files; listed project docs in docs/SUMMARY.md.
- No gameplay changes: ROM is upstream 1.17.2.

## Unreleased — F000 review and F001 audit (2026-11-25)
- F000 scaffold review recorded with amendments (feature-spec naming, decision format, verification environments).
- F001 upstream audit of pokeemerald-expansion 1.17.2: build verified in a Linux container; memory, save, move-slot and scheduler constraints measured; integration options and recommendation recorded (decision D010 pending).
- No engine imported; no gameplay changes.

## Unreleased — documentation bootstrap
- Seed project vision, operating instructions, dependency roadmap, battle design, Battle Lab specification, testing and workflow.
- No engine integration, gameplay changes, executable scripts or verified ROM build yet.

