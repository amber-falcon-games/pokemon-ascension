# Project status

Last updated: 2026-11-26 13:26 UTC (Claude session: F007b part 2, gym checkpoints).
Current phase: 0.2.0 Battle Lab track (F007–F013), with F007b dev checkpoints.
Current feature: F007b dev checkpoints. Part 1 merged (PR #8, main a30d2a4). Part 2 (CP-GYM2–8 plus 10 automated checkpoint tests) is on `feature/F007b-gym-checkpoints`, PR open; it merges when CI is green (dev-only tooling). Owner playtest of all checkpoints pending.
Done: F000–F005b. F006: the owner ran the smoke test. Release 0.1.0 has **not** been published: there is no tag or GitHub release, and `release.sh` was probably left waiting at a prompt. F007 is merged (PR #7) and waits for the owner's Battle Lab playtest.
Next eligible feature: F008, the Battle Lab team builder.
Last known-good playable build: `main` @ 841465262e (dev). There is no tagged release yet.
Current engine version: pokeemerald-expansion `expansion/1.17.2` (commit 79dd4369) on `main`.
Automated tests: CI runs on each PR. `make check TESTS=test/ascension/checkpoints.c`: 10/10 PASS locally; every checkpoint was screenshot-checked with gbashot; release build has no checkpoint or Lab symbols.
Local Mac environment: verified by the owner (setup, play, 16 tests passed).
Pending owner decisions: none.
Owner actions: (1) finish `./scripts/release.sh` for 0.1.0; (2) playtest the Battle Lab (`./scripts/play.sh --lab`); (3) optionally, try the checkpoints on the PR #8 branch.

## Session handoff
- Branches (D021): each feature is on its own `feature/F0xx-*` branch and they stack. `feature/F007b-dev-checkpoints` also carries the session-branch commits: gbashot, D021 and the Done-button convention.
- Checkpoints: `data/scripts/ascension_checkpoints.inc`: menu entry `Ascension_Checkpoint_<CP>` = `Ascension_CPState_<CP>` (story steps `Ascension_Story_GymNToGymN+1` in order + `Ascension_CPTeam_<CP>`) + warp. Menu rows in `src/debug.c` (`sDebugMenu_Actions_Checkpoints`); tests in `test/ascension/checkpoints.c`. Dev builds only (`#if !defined(RELEASE)` in `data/event_scripts.s`).
- Screenshots: `tools/ascension/shots/` (gbashot, built with libmgba-dev). Screenshots go only to the dashboard, never into the repo.
- Owner dashboard: https://claude.ai/artifact/TBcNe5AUVsVQBbduRuj2Wv (ArtifactData collections data/ascension/{features,actions,activity,state,shots}). Run `date -u` before writing timestamps. Check whether the owner pressed Done (`status: done`, `done_by: owner`). Set state to idle at the end of a turn.
- Exact next action: when the F007b part 2 PR is green, merge it. Then F008 Battle Lab team builder (write the spec first).
