# Changelog

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

