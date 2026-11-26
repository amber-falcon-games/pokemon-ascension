# Project status

Last updated: 2026-11-26 17:25 UTC (Claude session: F011 implemented per D022, stacked on F010).
Current phase: 0.2.0 Battle Lab track (F007–F013), with F007b dev checkpoints.
Current feature: Battle Lab stack awaiting the owner's combined playtest: F008 (PR #11) → F009 (PR #12) → F010 (PR #13) → F011 scenario files (stacked PR, D022). Each merges in order after acceptance; the next PR is retargeted to main. Lab tests 20/20 + tool tests 4/4.
Done: F000–F005b, F006 (release v0.1.0 published 2026-11-26: https://github.com/amber-falcon-games/pokemon-ascension/releases/tag/v0.1.0, tag at c204b79399), F007 Battle Lab entry and F007b dev checkpoints (both owner playtested and accepted 2026-11-26).
Next eligible feature: F008, the Battle Lab team builder.
Last known-good playable build: v0.1.0 (c204b79399), owner smoke-tested and patch-verified in Rom Patcher JS.
Current engine version: pokeemerald-expansion `expansion/1.17.2` (commit 79dd4369) on `main`.
Automated tests: CI runs on each PR. `make check TESTS=test/ascension/checkpoints.c`: 10/10 PASS locally; every checkpoint was screenshot-checked with gbashot; release build has no checkpoint or Lab symbols.
Local Mac environment: verified by the owner (setup, play, 16 tests passed).
Pending owner decisions: none.
Owner actions: one combined Lab playtest (now simplest via `./scripts/play.sh --lab` from the F011 branch; steps on the dashboard).

## Session handoff
- Branches (D021): each feature is on its own `feature/F0xx-*` branch and they stack. `feature/F007b-dev-checkpoints` also carries the session-branch commits: gbashot, D021 and the Done-button convention.
- Checkpoints: `data/scripts/ascension_checkpoints.inc`: menu entry `Ascension_Checkpoint_<CP>` = `Ascension_CPState_<CP>` (story steps `Ascension_Story_GymNToGymN+1` in order + `Ascension_CPTeam_<CP>`) + warp. Menu rows in `src/debug.c` (`sDebugMenu_Actions_Checkpoints`); tests in `test/ascension/checkpoints.c`. Dev builds only (`#if !defined(RELEASE)` in `data/event_scripts.s`).
- Screenshots: `tools/ascension/shots/` (gbashot, built with libmgba-dev). Screenshots go only to the dashboard, never into the repo.
- Owner dashboard: https://claude.ai/artifact/TBcNe5AUVsVQBbduRuj2Wv (ArtifactData collections data/ascension/{features,actions,activity,state,shots}). Run `date -u` before writing timestamps. Check whether the owner pressed Done (`status: done`, `done_by: owner`). Set state to idle at the end of a turn.
- Exact next action: merge the Lab stack in order after the playtest. Next stacked work: F012 (Lab inspection and deterministic controls): write the spec first; upstream's in-battle debug menu already covers much of it.
