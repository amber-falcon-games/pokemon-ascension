# Project status

Last updated: 2026-11-27 08:10 UTC (Claude session: F017 iteration 2 built from owner playtest feedback; awaiting playtest 2).
Current phase: Combat track (after the 0.2.0 Battle Lab foundation). D003 gate passed: the Lab is accepted.
Current feature: F017 stamina-timeline spike (D023), branch `feature/F017-energy-spike`, draft PR #18. Steps 1–5 built; status OWNER PLAYTEST.
Done: F000–F013 and F007b. Releases: v0.1.0 (c204b79399) and v0.2.0 (3faa92d298, Battle Lab foundation; player build unchanged from 0.1.0).
Next eligible feature: F017 energy and turn-order spike.
Last known-good playable build: v0.2.0 (3faa92d298), owner smoke-tested and published.
Current engine version: pokeemerald-expansion `expansion/1.17.2` (commit 79dd4369) on `main`.
Automated tests: CI runs on each PR. `make check TESTS=test/ascension/checkpoints.c`: 10/10 PASS locally; every checkpoint was screenshot-checked with gbashot; release build has no checkpoint or Lab symbols.
Local Mac environment: verified by the owner (setup, play, 16 tests passed).
Pending owner decisions: (1) should healing moves cost energy (free healing allows endless stalling)? (2) the Exhausted death spiral: keep it, only from self-spending, or ×0.5 Speed? Go/no-go comes after more refinement (owner, 2026-11-27: "keep refining the prototype").
Owner actions: playtest F017 iteration 2 (try the new Rules rows Energy/Damage/Cost), answer the two questions above.

## Session handoff
- Branches (D021): each feature is on its own `feature/F0xx-*` branch and they stack. `feature/F007b-dev-checkpoints` also carries the session-branch commits: gbashot, D021 and the Done-button convention.
- Checkpoints: `data/scripts/ascension_checkpoints.inc`: menu entry `Ascension_Checkpoint_<CP>` = `Ascension_CPState_<CP>` (story steps `Ascension_Story_GymNToGymN+1` in order + `Ascension_CPTeam_<CP>`) + warp. Menu rows in `src/debug.c` (`sDebugMenu_Actions_Checkpoints`); tests in `test/ascension/checkpoints.c`. Dev builds only (`#if !defined(RELEASE)` in `data/event_scripts.s`).
- Screenshots: `tools/ascension/shots/` (gbashot, built with libmgba-dev). Screenshots go only to the dashboard, never into the repo.
- Owner dashboard: https://claude.ai/artifact/TBcNe5AUVsVQBbduRuj2Wv (ArtifactData collections data/ascension/{features,actions,activity,state,shots}). Run `date -u` before writing timestamps. Check whether the owner pressed Done (`status: done`, `done_by: owner`). Set state to idle at the end of a turn.
- F017 tooling: battle log lines "LAB clock", "LAB energy", "LAB ai" (timer 3, 4389 ticks = 1 frame). Headless comparison ROMs/scripts were scratch only.
- Known F017 findings: end-of-turn effects tick per action (re-time in F018); AI never boosts; recharge passes while sitting out; LAB-011 balance (Slaking one-shots Ninjask).
- Exact next action: wait for the owner's F017 playtest feedback; tune numbers or take go/no-go. Do not start F014+ combat work before the decision.
